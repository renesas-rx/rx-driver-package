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
File        : LCDConf.c
Purpose     : Display controller configuration (single layer)
----------------------------------------------------------------------

MultiBuffering:
  emWin requires 3 buffers to be able to work with efficiently.
  The available On-chip RAM unfortunately restricts use of
  3 buffers for MultiBuffering to 1- and 4bpp mode.
  In 8bpp the line offset is 512, because LNOFF needs to be a
  multiple of 0x40 (see GRnFLM3.LNOFF). In that case
  0x200 * 0x110 * 3 = 0x66000 bytes are required. But
  unfortunately the size of the memory area is only 0x60000.
  In that case DoubleBuffering is used. In that mode the CPU has to
  wait for the next VSYNC interrupt in case of a new frame buffer.

SDRAM unavailable:
  According to R20UT3887EG SDRAM could not be used
  with OnTFT (see 'Bus Switch Select Configuration').
  When setting SW4 Pin4 to 'On', SDRAM is unavailable.

32bpp mode:
  Has not been tested because of a lack of RAM.

---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"
#include "GUIDRV_Lin.h"

#ifdef __RX
  #include "platform.h"
#endif
#ifdef __ICCRX__
  #include "iorx65n.h"
#endif

/*********************************************************************
*
*       Layer configuration (to be modified)
*
**********************************************************************
*/
//
// Physical display size
//
#define XSIZE_PHYS 480
#define YSIZE_PHYS 272

//
// Color depth
//
#define BITS_PER_PIXEL 8  // Allowed values: 1, 4, 8, 16, 32

//
// Frame buffer
//
#define FRAMEBUFFER_START 0x800000  // Begin of Expansion RAM

/*********************************************************************
*
*       Determine driver, color conversion and size of frame buffer
*
**********************************************************************
*/
//
// Color format selection
//
#define FORMAT_RGB_565   0
#define FORMAT_RGB_888   1
#define FORMAT_ARGB_1555 2
#define FORMAT_ARGB_4444 3
#define FORMAT_ARGB_8888 4
#define FORMAT_CLUT_8    5
#define FORMAT_CLUT_4    6
#define FORMAT_CLUT_1    7

//
// Color conversion, display driver and multiple buffers
//
#if   (BITS_PER_PIXEL == 32)
  #define COLOR_CONVERSION GUICC_M8888I
  #define DISPLAY_DRIVER   GUIDRV_LIN_32
  #define COLOR_FORMAT     FORMAT_RGB_888
  #define NUM_BUFFERS      1
#elif (BITS_PER_PIXEL == 16)
  #define COLOR_CONVERSION GUICC_M565
  #define DISPLAY_DRIVER   GUIDRV_LIN_16
  #define COLOR_FORMAT     FORMAT_RGB_565
  #define NUM_BUFFERS      1
#elif (BITS_PER_PIXEL == 8)
  #define COLOR_CONVERSION GUICC_8666
  #define DISPLAY_DRIVER   GUIDRV_LIN_8
  #define COLOR_FORMAT     FORMAT_CLUT_8
  #define NUM_BUFFERS      2
#elif (BITS_PER_PIXEL == 4)
  #define COLOR_CONVERSION GUICC_16
  #define DISPLAY_DRIVER   GUIDRV_LIN_4
  #define COLOR_FORMAT     FORMAT_CLUT_4
  #define NUM_BUFFERS      3
#elif (BITS_PER_PIXEL == 1)
  #define COLOR_CONVERSION GUICC_1
  #define DISPLAY_DRIVER   GUIDRV_LIN_1
  #define COLOR_FORMAT     FORMAT_CLUT_1
  #define NUM_BUFFERS      3
#endif

//
// Buffer size and stride
//
#define BYTES_PER_LINE   ((BITS_PER_PIXEL * XSIZE_PHYS) / 8)
#define LINE_OFFSET      (((BYTES_PER_LINE + 63) / 64) * 64)
#define VXSIZE_PHYS      ((LINE_OFFSET * 8) / BITS_PER_PIXEL)
#define BYTES_PER_BUFFER (LINE_OFFSET * YSIZE_PHYS)

//
// Timing
//
#define SYNC_H       4
#define SYNC_V       1
#define BGSYNC_HP    2
#define BGSYNC_VP    2
#define BGHSIZE_HP  (BGSYNC_HP + SYNC_H)
#define BGVSIZE_VP  (BGSYNC_VP + SYNC_V)
#define GRC_VS      (BGVSIZE_VP - BGSYNC_VP)
#define GRC_HS      (BGHSIZE_HP - BGSYNC_HP)

//
// Brightness & COntrast
//
#define _BRIGHTNESS 0x200
#define _CONTRAST   0x80

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static volatile int _PendingBuffer = -1;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _VSYNC_ISR
*/
static void _VSYNC_ISR(void * p) {
#if (NUM_BUFFERS == 3)
  U32 Addr;
#endif

  GLCDC.STCLR.BIT.VPOSCLR = 1;                  // Clears the STMON.VPOS flag
#if (NUM_BUFFERS == 3)
  //
  // Makes the pending buffer visible
  //
  if (_PendingBuffer >= 0) {
    Addr = FRAMEBUFFER_START
         + BYTES_PER_BUFFER * _PendingBuffer;   // Calculate address of buffer to be used  as visible frame buffer
    GLCDC.GR2FLM2              = Addr;          // Specify the start address of the frame buffer
    GLCDC.GR2VEN.BIT.VEN       = 1;             // Graphic 2 Register Value Reflection Enable
    GUI_MULTIBUF_ConfirmEx(0, _PendingBuffer);  // Tell emWin that buffer is used
    _PendingBuffer = -1;                        // Clear pending buffer flag
  }
#elif (NUM_BUFFERS == 2)
  //
  // Sets a flag to be able to notice the interrupt
  //
  _PendingBuffer = 0;
#endif
}

/*********************************************************************
*
*       _SwitchBuffersOnVSYNC
*/
#if (NUM_BUFFERS == 2)

static void _SwitchBuffersOnVSYNC(int Index) {
  U32 Addr;

  for (_PendingBuffer = 1; _PendingBuffer; );
  Addr = FRAMEBUFFER_START
       + BYTES_PER_BUFFER * Index;   // Calculate address of buffer to be used  as visible frame buffer
  GLCDC.GR2FLM2              = Addr; // Specify the start address of the frame buffer
  GLCDC.GR2VEN.BIT.VEN       = 1;    // Graphic 2 Register Value Reflection Enable
  GUI_MULTIBUF_ConfirmEx(0, Index);  // Tell emWin that buffer is used

}
#endif

/*********************************************************************
*
*       _InitController
*
* Purpose:
*   Should initialize the display controller
*/
static void _InitController(void) {
  //
  // Release stop state of GLCDC
  //
  SYSTEM.PRCR.WORD = 0xA50Bu;      // Protect off
  SYSTEM.MSTPCRC.BIT.MSTPC29 = 0;  // GLCDC, Release from the module-stop state
  SYSTEM.PRCR.WORD = 0xA500u;      // Protect on
  //
  // Function select of multiplex pins (Display B)
  //
  MPC.PWPR.BIT.B0WI   = 0;     // Enable function select access
  MPC.PWPR.BIT.PFSWE  = 1;
  MPC.PE5PFS.BIT.PSEL = 0x25;  // Pin8  - R3
  MPC.PE4PFS.BIT.PSEL = 0x25;  // Pin9  - R4
  MPC.PE3PFS.BIT.PSEL = 0x25;  // Pin10 - R5 (R0)
  MPC.PE2PFS.BIT.PSEL = 0x25;  // Pin11 - R6 (R1)
  MPC.PE1PFS.BIT.PSEL = 0x25;  // Pin12 - R7 (R2)
  MPC.PA3PFS.BIT.PSEL = 0x25;  // Pin15 - G2
  MPC.PA2PFS.BIT.PSEL = 0x25;  // Pin16 - G3
  MPC.PA1PFS.BIT.PSEL = 0x25;  // Pin17 - G4
  MPC.PA0PFS.BIT.PSEL = 0x25;  // Pin18 - G5
  MPC.PE7PFS.BIT.PSEL = 0x25;  // Pin19 - G6 (G0)
  MPC.PE6PFS.BIT.PSEL = 0x25;  // Pin20 - G7 (G1)
  MPC.PB0PFS.BIT.PSEL = 0x25;  // Pin24 - B3
  MPC.PA7PFS.BIT.PSEL = 0x25;  // Pin25 - B4
  MPC.PA6PFS.BIT.PSEL = 0x25;  // Pin26 - B5 (B0)
  MPC.PA5PFS.BIT.PSEL = 0x25;  // Pin27 - B6 (B1)
  MPC.PA4PFS.BIT.PSEL = 0x25;  // Pin28 - B7 (B2)
  MPC.PB5PFS.BIT.PSEL = 0x25;  // Pin30 - CLK
  MPC.PB2PFS.BIT.PSEL = 0x25;  // Pin32 - HSYNC (TCON2)
  MPC.PB4PFS.BIT.PSEL = 0x25;  // Pin33 - VSYNC (TCON0)
  MPC.PB1PFS.BIT.PSEL = 0x25;  // Pin34 - DEN   (TCON3)
  MPC.PWPR.BIT.B0WI   = 0;     // Disable function select access
  MPC.PWPR.BIT.PFSWE  = 0;
  //
  // Select IO function (Port Mode Register)
  //
  SYSTEM.PRCR.WORD = 0xA50Bu;  // Protect off
  PORTB.PMR.BYTE   = 0x37;     // Pins 0, 1, 2, 4 and 5 of PORTB
  PORTA.PMR.BYTE   = 0xFF;     // Pins 0 - 7 of PORTA
  PORTE.PMR.BYTE   = 0xFE;     // Pins 1 - 7 of PORTE
  SYSTEM.PRCR.WORD = 0xA500u;  // Protect on
  //
  // Set DISP signal on P97 (Pin31)
  //
  PORT6.PDR.BIT.B3  = 1;  // Port direction: output
  PORT6.PODR.BIT.B3 = 1;  // DISP on
  //
  // Switch backlight on, no PWM
  //
  PORT6.PDR.BIT.B6  = 1;
  PORT6.PODR.BIT.B6 = 1;
  //
  // Set the BGEN.SWRST bit to 1 to release the GLCDC from a software reset
  //
  GLCDC.BGEN.BIT.SWRST = 1;
  //
  // Set the frequency of the LCD_CLK and PXCLK to suit the format and set the PANELCLK.CLKEN bit to 1
  //
  GLCDC.PANELCLK.BIT.CLKSEL = 1;   // Select PLL clock
  GLCDC.PANELCLK.BIT.DCDR   = 24;  // 240 / 24 = 10 MHz
  GLCDC.PANELCLK.BIT.PIXSEL = 0;   // No frequency division
  GLCDC.PANELCLK.BIT.CLKEN  = 1;   // Enable LCD_CLK output
  //
  // Definition of Background Screen
  //
  GLCDC.BGPERI.BIT.FH  = XSIZE_PHYS + 45;  // Horizontal cycle (whole control screen)
  GLCDC.BGPERI.BIT.FV  = YSIZE_PHYS + 14;  // Vertical cycle (whole control screen)
  GLCDC.BGSYNC.BIT.HP  = BGSYNC_HP;        // Horizontal Synchronization Signal Assertion Position
  GLCDC.BGSYNC.BIT.VP  = BGSYNC_VP;        // Vertical Synchronization Assertion Position
  GLCDC.BGHSIZE.BIT.HP = BGHSIZE_HP;       // Horizontal Active Pixel Start Position (min. 6 pixels)
  GLCDC.BGHSIZE.BIT.HW = XSIZE_PHYS;       // Horizontal Active Pixel Width
  GLCDC.BGVSIZE.BIT.VP = BGVSIZE_VP;       // Vertical Active Display Start Position (min. 3 lines)
  GLCDC.BGVSIZE.BIT.VW = YSIZE_PHYS;       // Vertical Active Display Width
  //
  // Graphic 2 configuration
  //
  GLCDC.GR2FLMRD.BIT.RENB    = 1;                     // Enable reading of the frame buffer
  GLCDC.GR2FLM2              = FRAMEBUFFER_START;     // Specify the start address of the frame buffer
  GLCDC.GR2FLM3.BIT.LNOFF    = LINE_OFFSET;           // Offset value from the end address of the line to the start address of the next line
  GLCDC.GR2FLM5.BIT.DATANUM  = LINE_OFFSET / 64 - 1;  // Single Line Data Transfer Count
  GLCDC.GR2FLM5.BIT.LNNUM    = YSIZE_PHYS - 1;        // Single Frame Line Count
  GLCDC.GR2FLM6.BIT.FORMAT   = COLOR_FORMAT;          // Frame Buffer Color Format RGB565
  GLCDC.GR2AB1.BIT.DISPSEL   = 2;                     // Display Screen Control (current graphics)
  GLCDC.GR2AB1.BIT.ARCDISPON = 0;                     // Rectangular Alpha Blending Area Frame Display Control
  GLCDC.GR2AB1.BIT.GRCDISPON = 0;                     // Graphics Area Frame Display Control
  GLCDC.GR2AB1.BIT.ARCON     = 0;                     // Alpha Blending Control (Per-pixel alpha blending)
  GLCDC.GR2AB2.BIT.GRCVS     = GRC_VS;                // Graphics Area Vertical Start Position
  GLCDC.GR2AB2.BIT.GRCVW     = YSIZE_PHYS;            // Graphics Area Vertical Width
  GLCDC.GR2AB3.BIT.GRCHS     = GRC_HS;                // Graphics Area Horizontal Start Position
  GLCDC.GR2AB3.BIT.GRCHW     = XSIZE_PHYS;            // Graphics Area Horizontal Width
  GLCDC.GR2AB4.BIT.ARCVS     = GRC_VS;                // Rectangular Alpha Blending Area Vertical Start Position
  GLCDC.GR2AB4.BIT.ARCVW     = YSIZE_PHYS;            // Rectangular Alpha Blending Area Vertical Width
  GLCDC.GR2AB5.BIT.ARCHS     = GRC_HS;                // Rectangular Alpha Blending Area Horizontal Start Position
  GLCDC.GR2AB5.BIT.ARCHW     = XSIZE_PHYS;            // Rectangular Alpha Blending Area Horizontal Width
  GLCDC.GR2VEN.BIT.VEN       = 1;                     // Graphic 2 Register Value Reflection Enable
  //
  // Timing configuration
  //
  GLCDC.TCONTIM.BIT.OFFSET  = 3;           // Horizontal Synchronization Signal Reference Timing Offset
  GLCDC.TCONSTHB2.BIT.HSSEL = 1;           // Signal Reference Timing Select
  GLCDC.TCONSTVB1.BIT.VS    = 1;           // STVB Signal Assertion Timing
  GLCDC.TCONSTVB1.BIT.VW    = YSIZE_PHYS;  // STVB Signal Pulse Width
  GLCDC.TCONSTHB1.BIT.HW    = XSIZE_PHYS;  // STHB Signal Pulse Width
  GLCDC.TCONSTVA2.BIT.SEL   = 0;           // TCON0 Output Signal Select STVA (VSYNC)
  GLCDC.TCONSTHA2.BIT.SEL   = 2;           // TCON2 Output Signal Select STHA (HSYNC)
  GLCDC.TCONSTHB2.BIT.SEL   = 7;           // TCON3 Output Signal Select DE (DEN)
  //
  // Output interface
  //
  GLCDC.OUTSET.BIT.PHASE    = 0;  // Serial RGB Data Output Delay Control (0 cycles)
  GLCDC.OUTSET.BIT.DIRSEL   = 0;  // Serial RGB Scan Direction Select (forward)
  GLCDC.OUTSET.BIT.FRQSEL   = 0;  // Pixel Clock Division Control (no division)
  GLCDC.OUTSET.BIT.FORMAT   = 2;  // Output Data Format Select (RGB565)
  GLCDC.OUTSET.BIT.SWAPON   = 0;  // Pixel Order Control (B-G-R)
  GLCDC.OUTSET.BIT.ENDIANON = 0;  // Bit Endian Control (Little endian)
  //
  // Brightness Adjustment
  //
  GLCDC.BRIGHT2.BIT.BRTB = _BRIGHTNESS;  // B
  GLCDC.BRIGHT1.BIT.BRTG = _BRIGHTNESS;  // G
  GLCDC.BRIGHT2.BIT.BRTR = _BRIGHTNESS;  // R
  //
  // Contrast Adjustment Value
  //
  GLCDC.CONTRAST.BIT.CONTB = _CONTRAST;  // B
  GLCDC.CONTRAST.BIT.CONTG = _CONTRAST;  // G
  GLCDC.CONTRAST.BIT.CONTR = _CONTRAST;  // R
  //
  // Enable VPOS ISR
  //
  GLCDC.GR2CLUTINT.BIT.LINE               = YSIZE_PHYS;  // Detecting Scanline Setting
  GLCDC.DTCTEN.BIT.VPOSDTC                = 1;           // Enable detection of specified line notification in graphic 2
  GLCDC.INTEN.BIT.VPOSINTEN               = 1;           // Enable VPOS interrupt request
  ICU.IPR[VECT_ICU_GROUPAL1].BIT.IPR      = 1;           // Interrupt Priority Level
  ICU.IER[VECT_ICU_GROUPAL1 / 8].BIT.IEN1 = 1;           // Interrupt Request Enable
  GLCDC.STCLR.BIT.VPOSCLR                 = 1;           // Clears the STMON.VPOS flag
  ICU.GENAL1.BIT.EN8                      = 1;           // VPOS (line detection)
  //
  // Set function to be called on VSYNC
  //
  R_BSP_InterruptWrite(BSP_INT_SRC_AL1_GLCDC_VPOS, _VSYNC_ISR);
  //
  // Register Reflection
  //
  GLCDC.OUTVEN.BIT.VEN = 0x00000001;  // Output Control Block Register Value Reflection
  GLCDC.BGEN.LONG      = 0x00010101;  // Enable background generation block
}

//int aaanl;
//LCD_COLOR cll[256];
//U8 pos[256];

/*********************************************************************
*
*       _SetLUTEntry
*
* Purpose:
*   Should set the desired LUT entry
*/
static void _SetLUTEntry(LCD_COLOR Color, U8 Pos) {
  GLCDC.GR2CLUT0[Pos].BIT.R = (Color & 0xFF0000) >> 16;
  GLCDC.GR2CLUT0[Pos].BIT.G = (Color & 0x00FF00) >> 8;
  GLCDC.GR2CLUT0[Pos].BIT.B = (Color & 0x0000FF);
//  cll[aaanl] = Color;
//  pos[aaanl] = Pos;
//  aaanl++;
}

/*********************************************************************
*
*       _DisplayOnOff
*/
static void _DisplayOnOff(int OnOff) {
  if (OnOff) {
    GLCDC.BGEN.LONG   = 0x00010101;  // Enable background generation block
    PORT6.PODR.BIT.B6 = 1;
  } else {
    GLCDC.BGEN.LONG   = 0x00010100;  // Disable background generation block
    PORT6.PODR.BIT.B6 = 0;
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_X_Config
*
* Purpose:
*   Called during the initialization process in order to set up the
*   display driver configuration.
*
*/
void LCD_X_Config(void) {
  #if (NUM_BUFFERS > 1)
  GUI_MULTIBUF_ConfigEx(0, NUM_BUFFERS);
  #endif
  //
  // Set display driver and color conversion for 1st layer
  //
  GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER, COLOR_CONVERSION, 0, 0);
  //
  // Display driver configuration
  //
  LCD_SetSizeEx (0, XSIZE_PHYS, YSIZE_PHYS);
  LCD_SetVSizeEx(0, VXSIZE_PHYS, YSIZE_PHYS);
  LCD_SetVRAMAddrEx(0, (void *)FRAMEBUFFER_START);
}

/*********************************************************************
*
*       LCD_X_DisplayDriver
*
* Purpose:
*   This function is called by the display driver for several purposes.
*   To support the according task the routine needs to be adapted to
*   the display controller. Please note that the commands marked with
*   'optional' are not cogently required and should only be adapted if
*   the display controller supports these features.
*
* Parameter:
*   LayerIndex - Index of layer to be configured
*   Cmd        - Please refer to the details in the switch statement below
*   pData      - Pointer to a LCD_X_DATA structure
*
* Return Value:
*   < -1 - Error
*     -1 - Command not handled
*      0 - Ok
*/
int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData) {

	static int	flicker = 0;	/* Prevention of flicker 3 */
  int r;

  GUI_USE_PARA(LayerIndex);
  switch (Cmd) {
  //
  // Required
  //
  case LCD_X_INITCONTROLLER: {
    //
    // Called during the initialization process in order to set up the
    // display controller and put it into operation. If the display
    // controller is not initialized by any external routine this needs
    // to be adapted by the customer...
    //
    _InitController();
    return 0;
  }
  case LCD_X_SETLUTENTRY: {
    //
    // Required for setting a lookup table entry which is passed in the 'Pos' and 'Color' element of p
    //
    LCD_X_SETLUTENTRY_INFO * p;

    p = (LCD_X_SETLUTENTRY_INFO *)pData;
    _SetLUTEntry(p->Color, p->Pos);
    return 0;
  }
  case LCD_X_SHOWBUFFER: {
    //
    // Required if multiple buffers are used. The 'Index' element of p contains the buffer index.
    //
    LCD_X_SHOWBUFFER_INFO * p;

    p = (LCD_X_SHOWBUFFER_INFO *)pData;
#if (NUM_BUFFERS == 2)
    _SwitchBuffersOnVSYNC(p->Index);
#else
    _PendingBuffer = p->Index;
#endif
    break;
  }
  case LCD_X_ON: {
    //
    // Required if the display controller should support switching on and off
    //
		/* Prevention of flicker 4 */
//	_DisplayOnOff(1);
	if(flicker != 0)
	{
		_DisplayOnOff(1);
	}
	flicker = 1;
    break;
  }
  case LCD_X_OFF: {
    //
    // Required if the display controller should support switching on and off
    //
    _DisplayOnOff(0);
    break;
  }
  default:
    r = -1;
  }
  return r;
}


/*************************** End of file ****************************/
