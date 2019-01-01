/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2017  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

***** emWin - Graphical user interface for embedded applications *****
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.

----------------------------------------------------------------------
File        : PIDConf.c
Purpose     : Touch panel configuration
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"

#include "platform.h"
#include "r_sci_iic_rx_if.h"
#include "r_cmt_rx_if.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define SLAVE_ADDRESS        0x38

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
/*********************************************************************
*
*       TOUCH_DATA
*
*  Holds information about coordinates and ID.
*/
typedef struct {
  U8 xHigh;       // Bit 6..7 - EventFlag, Bit 0..3 xHigh
  U8 xLow;
  U8 yHigh;       // Bit 4..7 - TouchID, Bit 0..3 yHigh
  U8 yLow;
  U8 ID;
} TOUCH_DATA;

/*********************************************************************
*
*       REPORT_DATA
*
*  Holds information about different touch points, mode, gesture and
*  number of points.
*/
typedef struct {
  U8         DeviceMode;
  U8         GestureID;
  U8         NumPoints;
} REPORT_DATA;

/*********************************************************************
*
*       POINT_DATA
*
*  Used by this module to calculate the different MT flags and events.
*/
typedef struct {
  U16 xPos;
  U16 yPos;
  U8  Id;
  U8  Flags;
} POINT_DATA;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static int _LayerIndex;

static sci_iic_info_t _Info;

static U8 _aBuffer[63] = {0};
static U8 _Busy;

/*********************************************************************
*
*       _cb_SCI_IIC_ch6
*/
static void _cb_SCI_IIC_ch6(void) {
  sci_iic_mcu_status_t      iic_status;
  volatile sci_iic_return_t ret;

  ret = R_SCI_IIC_GetStatus(&_Info, &iic_status);
  if ((ret == SCI_IIC_SUCCESS) && (iic_status.BIT.NACK == 1)) {
    static GUI_PID_STATE StatePID;
    static int           IsTouched;
    REPORT_DATA          Report;
    TOUCH_DATA           TouchPoint;
    U8                 * pBuffer;

    StatePID.Layer    = _LayerIndex;          // Set layer who should handle touch
    pBuffer           =  _aBuffer;
    Report.DeviceMode = *pBuffer++;           // Get device mode, 000b - Work Mode, 001b - Factory Mode
    Report. GestureID = *pBuffer++;           // GestureID:  0x10 Move UP
                                              //             0x14 Move Left
                                              //             0x18 Move Down
                                              //             0x1C Move Right
                                              //             0x48 Zoom In
                                              //             0x49 Zoom Out
                                              //             0x00 No Gesture
    Report.NumPoints  = *pBuffer++;           // Number of points
    TouchPoint.xHigh  = (*pBuffer++) & 0x0F;  // Get the upper 4 bits of the x position
    TouchPoint.xLow   = *pBuffer++;           // and the lower 8 bits
    TouchPoint.yHigh  = (*pBuffer++) & 0x0F;  // Get the upper 4 bits of the y position
    TouchPoint.yLow   = *pBuffer++;           // and the lower 8 bits
    //
    // We just handle one touch point, so we stop here.
    //
    // Check if we have a touch detected
    //
    if (Report.NumPoints) {
      IsTouched        = 1;  // Remember that we have a touch, needed for generating up events
      StatePID.Pressed = 1;  // State is pressed
      //
      // Shift bits for x- and y- coordinate to the correct position
      //
      StatePID.x       = ((TouchPoint.xHigh & 0x0F) << 8 | TouchPoint.xLow);
      StatePID.y       = ((TouchPoint.yHigh & 0x0F) << 8 | TouchPoint.yLow);
      //
      // Pass touch data to emWin
      //
      GUI_TOUCH_StoreStateEx(&StatePID);
    } else {
      if (IsTouched) {         // If we had a touch,
        IsTouched        = 0;  // now we don't.
        StatePID.Pressed = 0;  // So, state is not pressed.
        //
        // Tell emWin
        //
        GUI_TOUCH_StoreStateEx(&StatePID);
      }
    }
    _Busy = 0;
  }
}

/*********************************************************************
*
*       _Exec
*/
static void _Exec(void) {
  //
  // Read all touch points to clear the buffer on TC side
  //
  static uint8_t slave_addr_eeprom[] = { SLAVE_ADDRESS };  // Slave address
  static uint8_t access_addr1[]      = { 0x00 };           // 1st data field
  volatile sci_iic_return_t ret;

  if (_Busy) {
    return;
  }
  //
  // Sets IIC Information (Ch6)
  //
  _Info.p_slv_adr    = slave_addr_eeprom;
  _Info.p_data1st    = access_addr1;
  _Info.p_data2nd    = _aBuffer;
  _Info.dev_sts      = SCI_IIC_NO_INIT;
  _Info.cnt1st       = sizeof(access_addr1);
  _Info.cnt2nd       = sizeof(_aBuffer);
  _Info.callbackfunc = &_cb_SCI_IIC_ch6;
  //
  // Start Master Receive
  //
  ret = R_SCI_IIC_MasterReceive(&_Info);
  if (ret == SCI_IIC_SUCCESS) {
    _Busy = 1;
  }
}

/*********************************************************************
*
*       _cbTimer
*/
static void _cbTimer(void * pData) {
  _Exec();
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       PID_X_SetLayerIndex
*/
void PID_X_SetLayerIndex(int LayerIndex) {
  _LayerIndex = LayerIndex;
}

/*********************************************************************
*
*       PID_X_Init
*/
void PID_X_Init(void) {
  U32 Channel;

  //
  // Sets IIC Information (Ch6)
  //
  _Info.ch_no        = 6;
  //
  // SCI open
  //
  if (R_SCI_IIC_Open(&_Info) != SCI_IIC_SUCCESS) {
    return;  // Error
  }
  //
  // Create timer for executing touch
  //
  R_CMT_CreatePeriodic(50, _cbTimer, &Channel);
}

/*********************************************************************
*
*       Dummies, not used
*/
void GUI_TOUCH_X_ActivateX(void) {}
void GUI_TOUCH_X_ActivateY(void) {}
int  GUI_TOUCH_X_MeasureX(void) { return 0;}
int  GUI_TOUCH_X_MeasureY(void) { return 0;}

/*************************** End of file ****************************/

