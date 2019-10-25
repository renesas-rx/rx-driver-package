/*******************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only 
* intended for use with Renesas products. No other uses are authorized. This 
* software is owned by Renesas Electronics Corporation and is protected under
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
* LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE 
* AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS 
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE 
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR
* ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software
* and to discontinue the availability of this software. By using this software,
* you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2014-2019 Renesas Electronics Corporation. All rights reserved.    
*******************************************************************************/
/******************************************************************************
* File Name    : r_ssi_api_rx.c
* Version      : 1.24
* Device       : RX64M, RX71M, RX113, RX231, RX230, RX23W
* Tool-Chain   : RX Family C Compiler
*                GCC for Renesas RX
*                IAR C/C++ Compiler for Renesas RX
* Description  : SSI driver module file for RX MCUs.
*******************************************************************************
* History : DD.MM.YYYY  Version  Description
*         : 01.08.2014   1.00     First release.
*         : 03.12.2014   1.10     Change flag access operations.
*                                 And add flag access functions
*                                 for TDE, RDF, TUIRQ, TOIRQ, RUIRQ, ROIRQ.
*         : 11.12.2014   1.11     RX71M support added.
*         : 28.04.2015   1.20     RX113, RX231, RX230 support added.
*         : 07.04.2017   1.21     Version number updated.
*         : 01.02.2019   1.22     Changed Minor version to 1.22.
*         : 20.05.2019   1.23     Added support for the GCC and IAR compiler.
*         : 20.06.2019   1.24     RX23W support added.
*
******************************************************************************/

/******************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/
/* Configuration options for the SSI API. This is also included in 
   r_ssi_api_rx.h and would normally not need to be included separately.
   It is included separately here so that the decision can be made to use
   the r_bsp package or not. */
#include "r_ssi_api_rx_config.h"

/* Function prototypes and device specific info needed for SSI API */
#include "r_ssi_api_rx_if.h"

/* Information needed for SSI API. */
#include "r_ssi_api_rx_private.h"

/******************************************************************************
Macro definitions
******************************************************************************/
#define SSI_FIFONUM2    (2u) /* Operator just used for functions in this file. */
#define SSI_CALCNUM2    (2u) /* Operator just used for functions in this file. */
                        
/* time-out count
   How to decide "time-out count".
     Functions R_SSI_Stop() & R_SSI_Mute() wait 2 PCM data in TX FIFO & 
       1 data in shift register to be sent. So totally at least "1/Fs [uSec] x 3" 
       period of time is needed.
     And when Sampling frequency is 8kHz, the time to finish sending all data 
       is the maximum. it is 
       1/(8kHz) x 3 = 375[uSec]. 
     So waiting time must be larger than 375[uSec].
     In case, CPU clock is 120MHz, loop times is maximum.
       1/(120MHz) x 45000 = 375[uSec]
     Adding about 10% margin(5000), total times for loop is 
       1/(120MHz) x (45000 + 5000) = 416[uSec]
     Then, here timeout loop times is configured "50000".
*/ 
#define SSI_TIMEOUT_416USEC         (50000u) /* time-out count */

/* Macro definitions to access status flag bits */
#define SSI_BIT_RDF     (0x00000001)    /* RDF bit location to clear*/
#define SSI_BIT_TDE     (0x00010000)    /* TDE bit location to clear*/
#define SSI_KEEP_RDF    (SSI_BIT_RDF)   /* RDF bit location to mask */
#define SSI_KEEP_TDE    (SSI_BIT_TDE)   /* TDE bit location to mask */
#define SSI_BIT_TUIRQ   (0x20000000)    /* TUIRQ bit location to clear*/
#define SSI_BIT_TOIRQ   (0x10000000)    /* TOIRQ bit location to clear*/
#define SSI_BIT_RUIRQ   (0x08000000)    /* RUIRQ bit location to clear*/
#define SSI_BIT_ROIRQ   (0x04000000)    /* ROIRQ bit location to clear*/
#define SSI_KEEP_TO_RU_RO  (SSI_BIT_TOIRQ | SSI_BIT_RUIRQ | SSI_BIT_ROIRQ) /* TO/RU/ROIRQ bits to mask */
#define SSI_KEEP_TU_RU_RO  (SSI_BIT_TUIRQ | SSI_BIT_RUIRQ | SSI_BIT_ROIRQ) /* TU/RU/ROIRQ bits to mask */
#define SSI_KEEP_TU_TO_RO  (SSI_BIT_TUIRQ | SSI_BIT_TOIRQ | SSI_BIT_ROIRQ) /* TU/TO/ROIRQ bits to mask */
#define SSI_KEEP_TU_TO_RU  (SSI_BIT_TUIRQ | SSI_BIT_TOIRQ | SSI_BIT_RUIRQ) /* TU/TO/RUIRQ bits to mask */

/******************************************************************************
Typedef definitions
******************************************************************************/
/* MSTP parameter  */
typedef enum {
    SSI_MSTP_CLR = 0,
    SSI_MSTP_SET = 1,
} ssi_mstp_t;

/******************************************************************************
Exported global variables 
******************************************************************************/

/******************************************************************************
Private global variables and functions
******************************************************************************/
/* Tables contain values for local functions */
static const uint32_t r_ssi_get_iomode [] =  /* iomode table*/
{
    SSI_CH0_IO_MODE,
#ifdef SSI1
    SSI_CH1_IO_MODE, /* activates when using MCU with SSI1 */
#endif
};

static const uint32_t r_ssi_get_ssi_address [] = /* SSI0/1 io register address table */
{
    (uint32_t)&SSI0,
#ifdef SSI1 
    (uint32_t)&SSI1, /* activates when using MCU with SSI1 */
#endif
};

/******************************************************************************
* Function Name: r_ssi_down_count
* Description  : is used for time-out operation, decrements "down_count" value
* Arguments    : down_count: a value to be decremented
* Return Value : "down_count" value after decement
******************************************************************************/
R_BSP_PRAGMA_STATIC_INLINE(r_ssi_down_count)
uint16_t r_ssi_down_count ( uint32_t down_count )
{
    if ( down_count > 0u )
    {
        down_count--;
    }
    else
    {
        /* Do nothing. */
    }
    return down_count;
} /* End of Function r_ssi_down_count */

/******************************************************************************
* Function Name: r_ssi_clear_flag_tde
* Description  : clears TDE bit of io register
* Arguments    : p_ssi_reg: SSI0 or SSI1 io register address
* Return Value : none
******************************************************************************/
R_BSP_PRAGMA_STATIC_INLINE(r_ssi_clear_flag_tde)
void r_ssi_clear_flag_tde ( volatile struct st_ssi R_SSI_EVENACCESS * p_ssi_reg )
{   
    uint32_t ssifsr = p_ssi_reg->SSIFSR.LONG;  /* get SSIFSR */

    ssifsr &= (~SSI_BIT_TDE);                 /* clear TDE to 0 */
    ssifsr |= SSI_KEEP_RDF;                   /* keep RDF value */
    p_ssi_reg->SSIFSR.LONG = ssifsr;           /* update SSIFSR */
} /* End of Function r_ssi_clear_flag_tde */

/******************************************************************************
* Function Name: r_ssi_clear_flag_rdf
* Description  : clears TDE bit of io register
* Arguments    : p_ssi_reg: SSI0 or SSI1 io register address
* Return Value : none
******************************************************************************/
R_BSP_PRAGMA_STATIC_INLINE(r_ssi_clear_flag_rdf)
void r_ssi_clear_flag_rdf ( volatile struct st_ssi R_SSI_EVENACCESS * p_ssi_reg )
{   
    uint32_t ssifsr = p_ssi_reg->SSIFSR.LONG;  /* get SSIFSR */

    ssifsr &= (~SSI_BIT_RDF);                 /* clear RDF to 0 */
    ssifsr |= SSI_KEEP_TDE;                   /* keep TDE value */
    p_ssi_reg->SSIFSR.LONG = ssifsr;           /* update SSIFSR */
} /* End of Function r_ssi_clear_flag_rdf */

/******************************************************************************
* Function Name: r_ssi_clear_flag_tuirq
* Description  : clears TDE bit of io register
* Arguments    : p_ssi_reg: SSI0 or SSI1 io register address
* Return Value : none
******************************************************************************/
R_BSP_PRAGMA_STATIC_INLINE(r_ssi_clear_flag_tuirq)
void r_ssi_clear_flag_tuirq ( volatile struct st_ssi R_SSI_EVENACCESS * p_ssi_reg )
{   
    uint32_t ssisr = p_ssi_reg->SSISR.LONG;  /* get SSISR */

    ssisr &= (~SSI_BIT_TUIRQ);              /* clear TUIRQ to 0 */
    ssisr |= SSI_KEEP_TO_RU_RO;             /* keep TO/RU/ROIRQ value */
    p_ssi_reg->SSISR.LONG = ssisr;           /* update SSISR */
} /* End of Function r_ssi_clear_flag_tuirq */

/******************************************************************************
* Function Name: r_ssi_clear_flag_toirq
* Description  : clears TDE bit of io register
* Arguments    : p_ssi_reg: SSI0 or SSI1 io register address
* Return Value : none
******************************************************************************/
R_BSP_PRAGMA_STATIC_INLINE(r_ssi_clear_flag_toirq)
void r_ssi_clear_flag_toirq ( volatile struct st_ssi R_SSI_EVENACCESS * p_ssi_reg )
{   
    uint32_t ssisr = p_ssi_reg->SSISR.LONG;  /* get SSISR */

    ssisr &= (~SSI_BIT_TOIRQ);              /* clear TOIRQ to 0 */
    ssisr |= SSI_KEEP_TU_RU_RO;             /* keep TU/RU/ROIRQ value */
    p_ssi_reg->SSISR.LONG = ssisr;           /* update SSISR */
} /* End of Function r_ssi_clear_flag_toirq */

/******************************************************************************
* Function Name: r_ssi_clear_flag_ruirq
* Description  : clears TDE bit of io register
* Arguments    : p_ssi_reg: SSI0 or SSI1 io register address
* Return Value : none
******************************************************************************/
R_BSP_PRAGMA_STATIC_INLINE(r_ssi_clear_flag_ruirq)
void r_ssi_clear_flag_ruirq ( volatile struct st_ssi R_SSI_EVENACCESS * p_ssi_reg )
{   
    uint32_t ssisr = p_ssi_reg->SSISR.LONG;  /* get SSISR */

    ssisr &= (~SSI_BIT_RUIRQ);              /* clear RUIRQ to 0 */
    ssisr |= SSI_KEEP_TU_TO_RO;             /* keep TU/TO/ROIRQ value */
    p_ssi_reg->SSISR.LONG = ssisr;           /* update SSISR */
} /* End of Function r_ssi_clear_flag_ruirq */

/******************************************************************************
* Function Name: r_ssi_clear_flag_roirq
* Description  : clears TDE bit of io register
* Arguments    : p_ssi_reg: SSI0 or SSI1 io register address
* Return Value : none
******************************************************************************/
R_BSP_PRAGMA_STATIC_INLINE(r_ssi_clear_flag_roirq)
void r_ssi_clear_flag_roirq ( volatile struct st_ssi R_SSI_EVENACCESS * p_ssi_reg )
{   
    uint32_t ssisr = p_ssi_reg->SSISR.LONG;  /* get SSISR */

    ssisr &= (~SSI_BIT_ROIRQ);              /* clear ROIRQ to 0 */
    ssisr |= SSI_KEEP_TU_TO_RU;             /* keep TU/TO/RUIRQ value */
    p_ssi_reg->SSISR.LONG = ssisr;           /* update SSISR */
} /* End of Function r_ssi_clear_flag_roirq */

/******************************************************************************
* Function Name: r_ssi_module_stop
* Description  : 
* Arguments    : p_ssi_reg: SSI0 or SSI1 io register address
*                ssi_mstp : activate or deactivate module stop
* Return Value : none
******************************************************************************/
R_BSP_PRAGMA_STATIC_INLINE(r_ssi_module_stop)
void r_ssi_module_stop (const ssi_ch_t Channel, const ssi_mstp_t ssi_mstp)
{
    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_LPC_CGC_SWR);

    switch (Channel)
    {
        case SSI_CH0:
            MSTP(SSI0) = ssi_mstp;
        break;

#ifdef SSI1 /* activates when using MCU with SSI1 */
        case SSI_CH1:
            MSTP(SSI1) = ssi_mstp;
        break;
#endif
        default:
            ; /* no operation */
        break;
    }

    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_LPC_CGC_SWR);
} /* End of Function  */

/******************************************************************************
* Function Name: r_ssi_check_param
* Description  : 
* Arguments    : const ssi_ch_t Channel     : [I] Channel Number of SSI
* Return Value : return value defined in enum ssi_ret_t
******************************************************************************/
R_BSP_PRAGMA_STATIC_INLINE(r_ssi_check_param)
ssi_ret_t r_ssi_check_param (const ssi_ch_t Channel)
{
    ssi_ret_t ret = SSI_SUCCESS;

    switch (Channel)
    {
        case SSI_CH0:
            if (true == R_BSP_HardwareLock(BSP_LOCK_SSI0))
            {
                (void)R_BSP_HardwareUnlock(BSP_LOCK_SSI0);
                ret = SSI_ERR_CHANNEL;
            }
        break;

#ifdef SSI1 /* activates when using MCU with SSI1 */
        case SSI_CH1:
            if (true == R_BSP_HardwareLock(BSP_LOCK_SSI1))
            {
                (void)R_BSP_HardwareUnlock(BSP_LOCK_SSI1);
                ret = SSI_ERR_CHANNEL;
            }
        break;

#endif
        default:
            ret = SSI_ERR_PARAM;
        break;
    }

    return ret;
} /* End of Function r_ssi_check_param */

/******************************************************************************
* Function Name: r_ssi_check_param_buf
* Description  : 
* Arguments    : const ssi_ch_t Channel     : [I] Channel Number of SSI
*              : const void * pBuf          : [I/O] Pointer to Output Buffer
*              : const uint8_t Samples      : [I] Number of samples to write 
* Return Value : return value defined in enum ssi_ret_t
******************************************************************************/
R_BSP_PRAGMA_STATIC_INLINE(r_ssi_check_param_buf)
ssi_ret_t r_ssi_check_param_buf (const ssi_ch_t Channel,
                                 const  void * pBuf,
                                 const  uint8_t Samples)
{
    ssi_ret_t ret = SSI_SUCCESS;

    switch (Channel)
    {
        case SSI_CH0:
            if (((uint32_t *)FIT_NO_PTR == pBuf) || (0 == Samples))
            {
                ret = SSI_ERR_PARAM;
                break;
            }

            if (true == R_BSP_HardwareLock(BSP_LOCK_SSI0))
            {
                (void)R_BSP_HardwareUnlock(BSP_LOCK_SSI0);
                ret = SSI_ERR_CHANNEL;
            }
        break;

#ifdef SSI1 /* activates when using MCU with SSI1 */
        case SSI_CH1:
            if (((uint32_t *)FIT_NO_PTR == pBuf) || (0 == Samples))
            {
                ret = SSI_ERR_PARAM;
                break;
            }

            if (true == R_BSP_HardwareLock(BSP_LOCK_SSI1))
            {
                (void)R_BSP_HardwareUnlock(BSP_LOCK_SSI1);
                ret = SSI_ERR_CHANNEL;
            }
        break;

#endif
        default:
            ret = SSI_ERR_PARAM;
        break;
    }

    return ret;
} /* End of Function r_ssi_check_param_buf */

/******************************************************************************
* Function Name: r_ssi_check_param_mute
* Description  : 
* Arguments    : const ssi_ch_t Channel      : [I] Channel Number of SSI
*              : const ssi_mute_t OnOff      : [I] Mute ON/OFF Switch
* Return Value : return value defined in enum ssi_ret_t
******************************************************************************/
R_BSP_PRAGMA_STATIC_INLINE(r_ssi_check_param_mute)
ssi_ret_t r_ssi_check_param_mute (const ssi_ch_t Channel,
                                  const  ssi_mute_t OnOff)
{
    ssi_ret_t ret = SSI_SUCCESS;

    switch (Channel)
    {
        case SSI_CH0:

        	/* The following code occurs a compilation warning but does not affect actual operation. */
        	if ((SSI_MUTE_ON > OnOff) || (SSI_MUTE_OFF < OnOff))
            {
                ret = SSI_ERR_PARAM;
                break;
            }

            if (true == R_BSP_HardwareLock(BSP_LOCK_SSI0))
            {
                (void)R_BSP_HardwareUnlock(BSP_LOCK_SSI0);
                ret = SSI_ERR_CHANNEL;
            }
        break;

#ifdef SSI1 /* activates when using MCU with SSI1 */
        case SSI_CH1:

        	/* The following code occurs a compilation warning but does not affect actual operation. */
        	if ((SSI_MUTE_ON > OnOff) || (SSI_MUTE_OFF < OnOff))
            {
                ret = SSI_ERR_PARAM;
                break;
            }

            if (true == R_BSP_HardwareLock(BSP_LOCK_SSI1))
            {
                (void)R_BSP_HardwareUnlock(BSP_LOCK_SSI1);
                ret = SSI_ERR_CHANNEL;
            }
        break;

#endif
        default:
            ret = SSI_ERR_PARAM;
        break;
    }

    return ret;
} /* End of Function r_ssi_check_param_mute */

/******************************************************************************
* Function Name: R_SSI_Open
* Description  : Opens SSI module & should be called once before use SSI
                   peripheral.

* Arguments    : const ssi_ch_t Channel      : [I] Channel Number of SSI
* Return Value : Extcution Result
******************************************************************************/
ssi_ret_t R_SSI_Open ( const ssi_ch_t Channel )
{
    ssi_ret_t ret = SSI_SUCCESS;

    /* parameter check operation */
    switch (Channel)
    {
        case SSI_CH0:
            if (false == R_BSP_HardwareLock(BSP_LOCK_SSI0))
            {
                ret = SSI_ERR_CHANNEL;
            }
        break;

#ifdef SSI1 /* activates when using MCU with SSI1 */
        case SSI_CH1:
            if (false == R_BSP_HardwareLock(BSP_LOCK_SSI1))
            {
                ret = SSI_ERR_CHANNEL;
            }
        break;

#endif
        default:
            ret = SSI_ERR_PARAM;
        break;
    }

    if ( SSI_SUCCESS == ret )
    {
        uint32_t ssifcr;
        uint32_t ssicr;
        volatile struct st_ssi R_SSI_EVENACCESS *p_ssi_reg;

        /* logical operation to make value to set ssi register  */
        switch (Channel)
        {
            case SSI_CH0:
                ssifcr = ((SSI_CH0_AUCKE << SSI_AUCKE_SHIFT) |
                            (SSI_CH0_TTRG << SSI_TTRG_SHIFT) |
                            (SSI_CH0_RTRG << SSI_RTRG_SHIFT));
                ssicr = ((SSI_CH0_CKDV << SSI_CKDV_SHIFT) |   
                           (SSI_CH0_DEL << SSI_DEL_SHIFT) |
                           (SSI_CH0_PDTA << SSI_PDTA_SHIFT) |
                           (SSI_CH0_SDTA << SSI_SDTA_SHIFT) |
                           (SSI_CH0_SPDP << SSI_SPDP_SHIFT) |
                           (SSI_CH0_SWSP << SSI_SWSP_SHIFT) |
                           (SSI_CH0_SCKP << SSI_SCKP_SHIFT) |
                           (SSI_CH0_SWSD << SSI_SWSD_SHIFT) |
                           (SSI_CH0_SCKD << SSI_SCKD_SHIFT) |
                           (SSI_CH0_SWL << SSI_SWL_SHIFT) |
                           (SSI_CH0_DWL << SSI_DWL_SHIFT));
                p_ssi_reg = &(SSI0);
            break;

#ifdef SSI1 /* activates when using MCU with SSI1 */
            case SSI_CH1:
                ssifcr = ((SSI_CH1_AUCKE << SSI_AUCKE_SHIFT) |
                           (SSI_CH1_TTRG << SSI_TTRG_SHIFT) |
                           (SSI_CH1_RTRG << SSI_RTRG_SHIFT));
                ssicr = ((SSI_CH1_CKDV << SSI_CKDV_SHIFT) |
                          (SSI_CH1_DEL << SSI_DEL_SHIFT) |
                          (SSI_CH1_PDTA << SSI_PDTA_SHIFT) |
                          (SSI_CH1_SDTA << SSI_SDTA_SHIFT) |
                          (SSI_CH1_SPDP << SSI_SPDP_SHIFT) |
                          (SSI_CH1_SWSP << SSI_SWSP_SHIFT) |
                          (SSI_CH1_SCKP << SSI_SCKP_SHIFT) |
                          (SSI_CH1_SWSD << SSI_SWSD_SHIFT) |
                          (SSI_CH1_SCKD << SSI_SCKD_SHIFT) |
                          (SSI_CH1_SWL << SSI_SWL_SHIFT) |
                          (SSI_CH1_DWL << SSI_DWL_SHIFT));
                p_ssi_reg = &(SSI1);  
            break;
                
#endif
            default:
                /* no operation */
            break;
        }
        
        /* Open operation mainbody */
        r_ssi_module_stop (Channel, SSI_MSTP_CLR);

        /* open operation */
        if ( SSI_SSISR_TSWNO_ON == p_ssi_reg->SSISR.BIT.TSWNO )
        {
            uint32_t timeout;

            /* reset SSI peripheral */
            timeout = SSI_TIMEOUT_416USEC; /* initializes timeout counter. */
            p_ssi_reg->SSIFCR.BIT.SSIRST = 1; /* sets SSIRST bit to "1" */

            /* WAIT_LOOP */
            while( 1 != p_ssi_reg->SSIFCR.BIT.SSIRST ) /* waits till SSIRST to be "1" */
            {
                /* Check loop count to detect abnormal condition. */
                timeout = r_ssi_down_count ( timeout );
                if ( 0 == timeout )
                {
                    ret = SSI_ERR_EXCEPT; /* returns abnormal condition. */
                    break;
                }
            }

            timeout = SSI_TIMEOUT_416USEC; /* initializes timeout counter. */
            p_ssi_reg->SSIFCR.BIT.SSIRST = 0; /* clears SSIRST bit to "0" */

            /* WAIT_LOOP */
            while( 0 != p_ssi_reg->SSIFCR.BIT.SSIRST ) /* waits till SSIRST to be "0" */
            {
                /* Check loop count to detect abnormal condition. */
                timeout = r_ssi_down_count ( timeout );
                if ( 0 == timeout )
                {
                    ret = SSI_ERR_EXCEPT; /* returns abnormal condition. */
                    break;
                }
            }
        }
            
        if ( SSI_SUCCESS == ret)
        {
            /* Set SSIFCR, SSICR register */
            p_ssi_reg->SSIFCR.LONG = 0u;
            p_ssi_reg->SSICR.LONG = 0u;
            p_ssi_reg->SSITDMR.LONG = 0u;
            
            /* Set SSIFCR, SSICR register */
            p_ssi_reg->SSIFCR.LONG |= ssifcr;
            p_ssi_reg->SSICR.LONG |= ssicr;
        }
    }

    return ret;
}

/******************************************************************************
End of function  R_SSI_Open
******************************************************************************/

/******************************************************************************
* Function Name: R_SSI_Start
* Description  : Starts transmit or receive operation using SSI peripheral.
* Arguments    : const ssi_ch_t Channel      : [I] Channel Number of SSI
* Return Value : Extcution Result
******************************************************************************/
ssi_ret_t R_SSI_Start ( const ssi_ch_t Channel )
{
    ssi_ret_t ret = SSI_SUCCESS;

    /* parameter & hardware lock checking operation */
    ret = r_ssi_check_param (Channel);

    /* start operation mainbody */
    if (SSI_SUCCESS == ret)  /* parameters are valid. */
    {
        volatile struct st_ssi R_SSI_EVENACCESS *p_ssi_reg;

        /* getting ssi register address */
        p_ssi_reg = (volatile struct st_ssi R_SSI_EVENACCESS *)r_ssi_get_ssi_address [Channel];

        const uint32_t ssisr_iirq = p_ssi_reg->SSISR.BIT.IIRQ;
        uint32_t dummy;         /* valiable for dummy reading to write SSIFSR, SSISR. */

        /* SSI Start operation */
        switch (r_ssi_get_iomode [Channel])
        {
            case SSI_IO_MODE_TX: /* case SSI transmit mode */
                if ((SSI_SSISR_IIRQ_ON == ssisr_iirq) &&
                   (SSI_SSICR_TEN_DISABLE == p_ssi_reg->SSICR.BIT.TEN))
                {
                    uint32_t count;

                    /* Tx FIFO Reset */
                    p_ssi_reg->SSIFCR.BIT.TFRST = SSI_SSIFCR_TFRST_ENABLE;
                    p_ssi_reg->SSIFCR.BIT.TFRST = SSI_SSIFCR_TFRST_DISABLE;
                    
                    /* Tx FIFO Initialize */

                    /* WAIT_LOOP */
                    for (count = 0u; count < SSI_SSIFSR_TDCMAX; count++)
                    {
                        p_ssi_reg->SSIFTDR = 0u;
                    }
                    
                    /* SSIFSR, SSISR Initialize */
                    dummy = p_ssi_reg->SSIFSR.LONG;       /* dummy reading is needed to write SSIFSR. */
                    p_ssi_reg->SSIFSR.LONG = 0u;
                    dummy = p_ssi_reg->SSISR.LONG;        /* dummy reading is needed to write SSISR. */
                    p_ssi_reg->SSISR.LONG = 0u;
                    dummy; /* This code occurs a compilation warning but does not affect actual operation. */
                    
                    /* Setting to start SSI Tx */
                    p_ssi_reg->SSIFCR.BIT.TIE = SSI_SSIFCR_TIE_ENABLE;
                    p_ssi_reg->SSICR.LONG |= ((SSI_SSICR_TUIEN_ENABLE << SSI_TUIEN_SHIFT) |
                                              (SSI_SSICR_TOIEN_ENABLE << SSI_TOIEN_SHIFT) |
                                              (SSI_SSICR_TEN_ENABLE << SSI_TEN_SHIFT));
                }

            break;

            case SSI_IO_MODE_RX: /* case SSI receive mode */
                if ((SSI_SSISR_IIRQ_ON == ssisr_iirq) &&
                (SSI_SSICR_REN_DISABLE == p_ssi_reg->SSICR.BIT.REN))
                {
                    /* Rx FIFO Reset */
                    p_ssi_reg->SSIFCR.BIT.RFRST = SSI_SSIFCR_RFRST_ENABLE;
                    p_ssi_reg->SSIFCR.BIT.RFRST = SSI_SSIFCR_RFRST_DISABLE;
                    
                    /* SSIFSR, SSISR Initialize */
                    dummy = p_ssi_reg->SSIFSR.LONG;       /* dummy reading is needed to write SSIFSR. */
                    p_ssi_reg->SSIFSR.LONG = 0u;
                    dummy = p_ssi_reg->SSISR.LONG;        /* dummy read is needed to write SSISR. */
                    p_ssi_reg->SSISR.LONG = 0u;
                    dummy; /* This code occurs a compilation warning but does not affect actual operation. */
                    
                    /* Setting to start SSI Rx */
                    p_ssi_reg->SSIFCR.BIT.RIE = SSI_SSIFCR_RIE_ENABLE;
                    p_ssi_reg->SSICR.LONG |= ((SSI_SSICR_RUIEN_ENABLE << SSI_RUIEN_SHIFT) |
                                              (SSI_SSICR_ROIEN_ENABLE << SSI_ROIEN_SHIFT) |
                                              (SSI_SSICR_REN_ENABLE));
                }
            break;

            case SSI_IO_MODE_TR: /* case SSI transmit & receive mode */
                if ((SSI_SSISR_IIRQ_ON == ssisr_iirq) &&
                   (SSI_IO_MODE_NONE == (p_ssi_reg->SSICR.LONG & SSI_IO_MODE_TR)))
                {
                     uint32_t count;
                    
                    /* Tx FIFO, Rx FIFO Reset */
                    p_ssi_reg->SSIFCR.LONG |= ((SSI_SSIFCR_TFRST_ENABLE << SSI_TFRST_SHIFT) |
                                               (SSI_SSIFCR_RFRST_ENABLE));
                    p_ssi_reg->SSIFCR.LONG &= ~((SSI_SSIFCR_TFRST_ENABLE << SSI_TFRST_SHIFT) |
                                                (SSI_SSIFCR_RFRST_ENABLE));
                    
                    /* Tx FIFO Initialize */

                    /* WAIT_LOOP */
                    for (count = 0u; count < SSI_SSIFSR_TDCMAX; count++)
                    {
                        p_ssi_reg->SSIFTDR = 0u;
                    }
                    
                    /* SSIFSR, SSISR clear to "0". */
                    dummy = p_ssi_reg->SSIFSR.LONG;       /* dummy reading is needed to write SSIFSR. */
                    p_ssi_reg->SSIFSR.LONG = 0u;
                    dummy = p_ssi_reg->SSISR.LONG;        /* dummy reading is needed to write SSISR. */
                    p_ssi_reg->SSISR.LONG = 0u;
                    dummy; /* This code occurs a compilation warning but does not affect actual operation. */
                    
                    /* Setting to start SSI Tx, Rx */
                    p_ssi_reg->SSIFCR.LONG |= ((SSI_SSIFCR_TIE_ENABLE << SSI_TIE_SHIFT) |
                                               (SSI_SSIFCR_RIE_ENABLE << SSI_RIE_SHIFT));
                    p_ssi_reg->SSICR.LONG |= ((SSI_SSICR_TUIEN_ENABLE << SSI_TUIEN_SHIFT) |
                                              (SSI_SSICR_TOIEN_ENABLE << SSI_TOIEN_SHIFT) |
                                              (SSI_SSICR_RUIEN_ENABLE << SSI_RUIEN_SHIFT) |
                                              (SSI_SSICR_ROIEN_ENABLE << SSI_ROIEN_SHIFT) |
                                              (SSI_SSICR_TEN_ENABLE << SSI_TEN_SHIFT) |
                                              (SSI_SSICR_REN_ENABLE));
                }
            break;

            default:
                ret = SSI_ERR_CHANNEL;
            break;
        }
    }

    return ret;
}
/******************************************************************************
End of function  R_SSI_Start
******************************************************************************/

/******************************************************************************
* Function Name: R_SSI_Stop
* Description  : Stops SSI transmit or receive operation.
* Arguments    : const ssi_ch_t Channel      : [I] Channel Number of SSI
* Return Value : Extcution Result
******************************************************************************/
ssi_ret_t R_SSI_Stop(const ssi_ch_t Channel)
{
    ssi_ret_t ret = SSI_SUCCESS;

    /* parameter & hardware lock checking operation */
    ret = r_ssi_check_param (Channel);

    /* stop operation mainbody */
    if (SSI_SUCCESS == ret)  /* parameters are valid. */
    {
        uint32_t timeout;
        volatile struct st_ssi R_SSI_EVENACCESS *p_ssi_reg;

        /* getting ssi register address */
        p_ssi_reg = (volatile struct st_ssi R_SSI_EVENACCESS *)r_ssi_get_ssi_address [Channel];

        const uint32_t ssisr_iirq = p_ssi_reg->SSISR.BIT.IIRQ;

        /* SSI Stop operation */
        switch (r_ssi_get_iomode [Channel])
        {
            case SSI_IO_MODE_TX: /* case SSI transmit mode */
                if ((SSI_SSISR_IIRQ_OFF == ssisr_iirq) &&
                    (SSI_SSICR_TEN_ENABLE == p_ssi_reg->SSICR.BIT.TEN))
                {
                    /* procedure to disable SSI transmit operation. */
                    p_ssi_reg->SSICR.LONG &= ~((SSI_SSICR_TUIEN_ENABLE << SSI_TUIEN_SHIFT) |
                                               (SSI_SSICR_TOIEN_ENABLE << SSI_TOIEN_SHIFT));
                    p_ssi_reg->SSIFCR.BIT.TIE = SSI_SSIFCR_TIE_DISABLE;
                    
                    /* wait until Tx FIFO to be more than two empties. */
                    /* initialize timeout counter. */
                    timeout = SSI_TIMEOUT_416USEC; 

                    /* WAIT_LOOP */
                    while (p_ssi_reg->SSIFSR.BIT.TDC >= SSI_SSIFSR_TDC7)
                    {
                        timeout = r_ssi_down_count ( timeout );
                        if ( 0u == timeout )
                        { 
                            /* Return from abnormal condition. */
                            ret = SSI_ERR_EXCEPT;
                            break;
                        }
                    }
                    
                    if (ret != SSI_ERR_EXCEPT)
                    {
                        /* Write "0" to Tx FIFO. It requires to write 8 bytes to Tx FIFO. */
                        p_ssi_reg->SSIFTDR = 0u;
                        p_ssi_reg->SSIFTDR = 0u;

                        /* wait until Tx FIFO becomes the underflow. */
                        /* initialize timeout counter. */
                        timeout = SSI_TIMEOUT_416USEC;

                        /* WAIT_LOOP */
                        while (SSI_SSISR_TUIRQ_OFF == p_ssi_reg->SSISR.BIT.TUIRQ)
                        {
                            /* Check loop count to detect abnormal condition. */
                            timeout = r_ssi_down_count ( timeout );
                            if ( 0u == timeout )
                            { 
                                /* Return from abnormal condition. */
                                ret = SSI_ERR_EXCEPT;
                                break;
                            }
                        }
                    }

                    /* disable SSI transmit operation  */
                    p_ssi_reg->SSICR.BIT.TEN = SSI_SSICR_TEN_DISABLE;
                    r_ssi_clear_flag_tuirq( p_ssi_reg );

                    /* check TSWNO to judge it abnormal condition or not. */
                    if ( SSI_SSISR_TSWNO_ON == p_ssi_reg->SSISR.BIT.TSWNO )
                    {
                        ret = SSI_ERR_EXCEPT;
                    }
                }
                else
                {
                    /* Exception Error */
                    p_ssi_reg->SSICR.BIT.TEN = SSI_SSICR_TEN_DISABLE;
                    ret = SSI_ERR_EXCEPT;
                }
            break;

            case SSI_IO_MODE_RX: /* case SSI receive mode */

                /* disable SSI receive operation. */
                p_ssi_reg->SSICR.LONG &= ~((SSI_SSICR_RUIEN_ENABLE << SSI_RUIEN_SHIFT) |
                                           (SSI_SSICR_ROIEN_ENABLE << SSI_ROIEN_SHIFT) |
                                           (SSI_SSICR_REN_ENABLE));

                /* disable SSI receive operation  */
                p_ssi_reg->SSIFCR.BIT.RIE = SSI_SSIFCR_RIE_DISABLE;
            break;

            case SSI_IO_MODE_TR: /* case SSI transmit & receive mode */
                if ((SSI_SSISR_IIRQ_OFF == ssisr_iirq) &&
                    (SSI_IO_MODE_TR == (p_ssi_reg->SSICR.LONG & SSI_IO_MODE_TR)))
                {
                    /* Procedure to disable SSI transmit operation. 
                       Receive operation is disabled here.
                    */
                    p_ssi_reg->SSICR.LONG &= ~((SSI_SSICR_TUIEN_ENABLE << SSI_TUIEN_SHIFT) |
                                               (SSI_SSICR_TOIEN_ENABLE << SSI_TOIEN_SHIFT) |
                                               (SSI_SSICR_RUIEN_ENABLE << SSI_RUIEN_SHIFT) |
                                               (SSI_SSICR_ROIEN_ENABLE << SSI_ROIEN_SHIFT) |
                                               (SSI_SSICR_REN_ENABLE));
                    p_ssi_reg->SSIFCR.LONG &= ~((SSI_SSIFCR_TIE_ENABLE << SSI_TIE_SHIFT) |
                                                (SSI_SSIFCR_RIE_ENABLE << SSI_RIE_SHIFT));
                    
                    /* wait until Tx FIFO to be more than two empties. */
                    /* initialize timeout counter. */
                    timeout = SSI_TIMEOUT_416USEC; 

                    /* WAIT_LOOP */
                    while (p_ssi_reg->SSIFSR.BIT.TDC >= SSI_SSIFSR_TDC7)
                    {
                        /* Check loop count to detect abnormal condition. */
                        timeout = r_ssi_down_count ( timeout );
                        if ( 0u == timeout )
                        {
                            /* Return from abnormal condition. */
                            ret = SSI_ERR_EXCEPT;
                            break;
                        }
                    }
                    
                    if (ret != SSI_ERR_EXCEPT)
                    {
                        /* write data "0" in Tx FIFO */
                        p_ssi_reg->SSIFTDR = 0u;
                        p_ssi_reg->SSIFTDR = 0u;

                        /* wait until Tx FIFO becomes the underflow */
                        /* initialize timeout counter. */
                        timeout = SSI_TIMEOUT_416USEC; 

                        /* WAIT_LOOP */
                        while (SSI_SSISR_TUIRQ_OFF == p_ssi_reg->SSISR.BIT.TUIRQ)
                        {
                            /* Check loop count to detect abnormal condition. */
                            timeout = r_ssi_down_count ( timeout );
                            if ( 0 == timeout )
                            { 
                                /* Return from abnormal condition. */
                                ret = SSI_ERR_EXCEPT;
                                break;
                            }
                        }
                    }

                    /* disable SSI transmit operation  */
                    p_ssi_reg->SSICR.BIT.TEN = SSI_SSICR_TEN_DISABLE;
                    r_ssi_clear_flag_tuirq( p_ssi_reg );

                    /* check TSWNO to judge it abnormal condition or not. */
                    if ( SSI_SSISR_TSWNO_ON == p_ssi_reg->SSISR.BIT.TSWNO )
                    {
                        ret = SSI_ERR_EXCEPT;
                    }
                }
                else
                {
                    /* Exception Error */
                    p_ssi_reg->SSICR.LONG &= ~((SSI_SSICR_TEN_ENABLE << SSI_TEN_SHIFT) |
                                               (SSI_SSICR_REN_ENABLE)); /* execution order doesn't affect result */
                    ret = SSI_ERR_EXCEPT;
                }
            break;

            default:
                ret = SSI_ERR_CHANNEL;
            break;
        }

        /* Wait operation for IIRQ to be set. */
        if ( ret == SSI_SUCCESS )
        {
            /* Wait until Idle mode Interrrupt status Flag to be set. */
            /* initialize timeout counter. */
            timeout = SSI_TIMEOUT_416USEC;

            /* WAIT_LOOP */
            while ( SSI_SSISR_IIRQ_OFF == p_ssi_reg->SSISR.BIT.IIRQ )
            {
                /* Check loop count to detect abnormal condition. */
                timeout = r_ssi_down_count ( timeout );
                if ( 0u == timeout )
                { 
                    /* Return from abnormal condition. */
                    ret = SSI_ERR_EXCEPT;
                    break;
                }
            }
        }
    }

   return ret;
}
/******************************************************************************
End of function  R_SSI_Stop
******************************************************************************/

/******************************************************************************
* Function Name: R_SSI_Close
* Description  : Closes SSI module & should be called when stop using SSI module.
* Arguments    : const ssi_ch_t Channel      : [I] Channel Number of SSI
* Return Value : Extcution Result
* Note         : Need to wait more than a periof of 3 AUDIO_CLK before
*                R_SSI_Open() to be called after R_SSI_Close().
******************************************************************************/
ssi_ret_t R_SSI_Close(const ssi_ch_t Channel)
{
    ssi_ret_t ret = SSI_SUCCESS;
    uint32_t dummy;         /* valiable for dummy reading to write SSIFSR, SSISR. */

    /* parameter & hardware lock checking operation */
    ret = r_ssi_check_param (Channel);

    if (SSI_SUCCESS == ret)  /* parameters are valid. */
    {
        volatile struct st_ssi R_SSI_EVENACCESS *p_ssi_reg;

        /* getting ssi register address */
        p_ssi_reg = (volatile struct st_ssi R_SSI_EVENACCESS *)r_ssi_get_ssi_address [Channel];

        /* Close operation mainbody */
        { 
            /* case: hardware is locked. */
            const uint32_t ssisr_iirq = p_ssi_reg->SSISR.BIT.IIRQ;

            if ((SSI_SSISR_IIRQ_OFF == ssisr_iirq) &&
               ((p_ssi_reg->SSICR.LONG & SSI_IO_MODE_TR) != SSI_IO_MODE_NONE))
            {
                ret = SSI_ERR_CHANNEL;
            }
            else
            {
                /*  'Set reset value to registers. 
                    'This operation sets AUCKE disabled.
                    'Clear operation for FIFOs is not executed,
                       because FIFOs are filled with "0" after R_SSI_Stop()
                       operation at this moment. So that FIFO underflow & 
                       overflow will not be arisen.
                */
                /* turn registers to default value */
                p_ssi_reg->SSIFCR.LONG = 0u;
                p_ssi_reg->SSICR.LONG = 0u;
                p_ssi_reg->SSITDMR.LONG = 0u;
                dummy = p_ssi_reg->SSIFSR.LONG;
                p_ssi_reg->SSIFSR.LONG = 0u;
                dummy = p_ssi_reg->SSISR.LONG;
                p_ssi_reg->SSISR.LONG = SSI_SSISR_RESET_VALUE;
                dummy; /* This code occurs a compilation warning but does not affect actual operation. */

                /* Stop feeding clock to SSI peripheral to deactivate it. */
                r_ssi_module_stop (Channel, SSI_MSTP_SET);

                switch (Channel)
                {

                    /* release HW lock to close SSI driver. */
                    case SSI_CH0:
                        if (false == R_BSP_HardwareUnlock(BSP_LOCK_SSI0)) /* Unlock to close SSI driver */
                        {
                            /* case Unlock failure */
                            ret = SSI_ERR_CHANNEL;
                        }
                    break;

#ifdef SSI1 /* activates when using MCU with SSI1 */
                    case SSI_CH1:
                        if (false == R_BSP_HardwareUnlock(BSP_LOCK_SSI1)) /* Unlock to close SSI driver */
                        {
                            /* case Unlock failure */
                            ret = SSI_ERR_CHANNEL;
                        }
                    break;

#endif
                    default:
                        /* no operation */
                    break;
                }
            }
        }
    }

    return ret;
}
/******************************************************************************
End of function  R_SSI_Close
******************************************************************************/

/******************************************************************************
* Function Name: R_SSI_Write
* Description  : Writes PCM data to SSI Tx FIFO from pointed buffer memory.
* Arguments    : const ssi_ch_t Channel     : [I] Channel Number of SSI
*              : const void * pBuf          : [I/O] Pointer to Output Buffer
*              : const uint8_t Samples      : [I] Number of samples to write 
* Return Value : Extcution Result(<0) or Number of Samples written to Tx FIFO(>=0)
******************************************************************************/
int8_t R_SSI_Write( const ssi_ch_t Channel,
                    const  void * pBuf,
                    const  uint8_t Samples)
{
    int8_t ret = SSI_SUCCESS;

    /* parameter check operation */
    ret = r_ssi_check_param_buf (Channel, pBuf, Samples);

    /* write operation mainbody */
    if (SSI_SUCCESS == ret)  /* parameters are valid. */
    {
        volatile struct st_ssi R_SSI_EVENACCESS *p_ssi_reg;

        /* getting ssi register address */
        p_ssi_reg = (volatile struct st_ssi R_SSI_EVENACCESS *)r_ssi_get_ssi_address [Channel];

        /* SSI Write */
        if (SSI_IO_MODE_RX == r_ssi_get_iomode [Channel])
        {
            ret = SSI_ERR_CHANNEL;
        }
        else
        {
            uint32_t fifo_free;     /* holds the number of free memory in Tx FIFO */
            uint32_t request;       /* holds the number of times to write-access by request.*/
            uint32_t limit;         /* holds the number of times to write-access to Tx FIFO. */
            uint32_t count = 0u;   /* The number of times Tx FIFO was accessed. */
                
            /* Process to write PCM data to Tx FIFO */
            /* Calculate the number of free memory in Tx FIFO. */
            fifo_free = SSI_SSIFSR_TDCMAX - p_ssi_reg->SSIFSR.BIT.TDC;
            if (0u == fifo_free)
            {
                /* return because Tx FIFO is full. */
                ret = 0u;
            }
            else
            {
                /* Calculate time to write-access just corresponding to the "Samples". */
                if (SSI_SSICR_DWL08 == p_ssi_reg->SSICR.BIT.DWL)
                {
                    /* Casting "Samples" to uint32_t is valid because "Samples" is uint8_t
                         and division by SSI_CALCNUM2 is within the uint32_t. */
                    request = ((uint32_t)Samples) / SSI_CALCNUM2;
                }
                else if (SSI_SSICR_DWL16 == (uint32_t)p_ssi_reg->SSICR.BIT.DWL)
                {
                    /* Casting "Samples" to uint32_t is valid because "Samples" is uint8_t. */
                    request = (uint32_t)Samples;
                }
                else
                {
                    /* Casting "Samples" to uint32_t is valid because "Samples" is uint8_t
                         and multiple by SSI_CALCNUM2 is within the uint32_t. */
                    request = ((uint32_t)Samples) * SSI_CALCNUM2;
                }
                    
                /* Limit operation for times to write-accecss to Tx FIFO. */
                if (fifo_free <= request)
                {
                    /* case, the number of free memory of Tx FIFO is larger than request. */
                    limit = fifo_free;
                }
                else
                {
                    /* case, the number of free memory of Tx FIFO is smaller than request. */
                    limit = request;
                }
                    
                /*  Write PCM data to Tx FIFO.
                    It is necessary to write 8 bytes as one time.
                */

                /* WAIT_LOOP */
                while ((limit - count) >= SSI_FIFONUM2)
                {
                 /* Casting pBuf to (const uint32_t *) is valid because it just points
                       address on 32 bit address space. */
                    p_ssi_reg->SSIFTDR = *((const uint32_t *)pBuf);
                    pBuf = ((const uint32_t *)pBuf) + 1u;
                    p_ssi_reg->SSIFTDR = *((const uint32_t *)pBuf);
                    pBuf = ((const uint32_t *)pBuf) + 1u;
                    count += SSI_FIFONUM2;
                }
                    
                /* Clear Tx FIFO empty flag */
                r_ssi_clear_flag_tde( p_ssi_reg );

                /* Convert the number of FIFO to be plactically written
                     in Tx FIFO into the number of Samples */
                if (SSI_SSICR_DWL08 == p_ssi_reg->SSICR.BIT.DWL)
                {
                    /* Casting "(count * SSI_CALCNUM2)" to uint32_t is valid because "count" shows
                         times to access to Tx FIFO and the size of Tx FIFO is 8. */
                    ret = (int8_t)(count * SSI_CALCNUM2);
                }
                else if (SSI_SSICR_DWL16 == p_ssi_reg->SSICR.BIT.DWL)
                {
                    /* Casting "count" to uint32_t is valid because "count" shows
                         times to access to Tx FIFO and the size of Tx FIFO is 8. */
                    ret = (int8_t)count;
                }
                else
                {
                    /* Casting "(count / SSI_CALCNUM2)" to uint32_t is valid because "count" shows
                         times to access to Tx FIFO and the size of Tx FIFO is 8. */
                    ret = (int8_t)(count / SSI_CALCNUM2);
                }
            }
        }
    }

    return ret;
}
/******************************************************************************
End of function  R_SSI_Write
******************************************************************************/

/******************************************************************************
* Function Name: R_SSI_Read
* Description  : Reads PCM data from SSI Rx FIFO to pointed buffer memory.
* Arguments    : const ssi_ch_t Channel     : [I] Channel Number of SSI
*              : void * pBuf                : [I/O] Pointer to Input Buffer
*              : const uint8_t Samples      : [I] Number of samples to read 
* Return Value : Extcution Result(<0) or Number of Samples read from Rx FIFO(>=0)
******************************************************************************/
int8_t R_SSI_Read( const ssi_ch_t Channel,
                    void * pBuf,
                    const uint8_t Samples)
{
    int8_t ret = SSI_SUCCESS;

    /* parameter check operation */
    ret = r_ssi_check_param_buf (Channel, pBuf, Samples);

    /* read operation mainbody */
    if (SSI_SUCCESS == ret)  /* parameters are valid. */
    {
        volatile struct st_ssi R_SSI_EVENACCESS *p_ssi_reg;

        /* getting ssi register address */
        p_ssi_reg = (volatile struct st_ssi R_SSI_EVENACCESS *)r_ssi_get_ssi_address [Channel];

        if (SSI_IO_MODE_TX == r_ssi_get_iomode [Channel])
        {
            ret = SSI_ERR_CHANNEL;
        }
        else
        {
            uint32_t fifo_filled;   /* holds the number of filled memory in Rx FIFO */
            uint32_t request;       /* holds the number of times to read-access by request.*/
            uint32_t limit;         /* holds the number of times to read-access to Rx FIFO. */
            uint32_t count = 0u;   /* The number of times Tx FIFO was accessed. */
                
            /* Process to read PCM data from Rx FIFO */
            /* Get the number of filled memory in Rx FIFO. */
            fifo_filled = p_ssi_reg->SSIFSR.BIT.RDC;
            if (0u == fifo_filled)
            {
                /* return because Rx FIFO is empty. */
                ret = 0u;
            }
            else
            {
                /* Calculate time to read-access just corresponding to the "Samples". */
                if (SSI_SSICR_DWL08 == p_ssi_reg->SSICR.BIT.DWL)
                {
                    /* Casting "Samples" to uint32_t is valid because "Samples" is uint8_t
                         and division by SSI_CALCNUM2 is within the uint32_t. */
                    request = ((uint32_t)Samples) / SSI_CALCNUM2;
                }
                else if (SSI_SSICR_DWL16 == (uint32_t)p_ssi_reg->SSICR.BIT.DWL)
                {
                    /* Casting "Samples" to uint32_t is valid because "Samples" is uint8_t. */
                    request = (uint32_t)Samples;
                }
                else
                {
                    /* Casting "Samples" to uint32_t is valid because "Samples" is uint8_t
                         and multiple by SSI_CALCNUM2 is within the uint32_t. */
                    request = ((uint32_t)Samples) * SSI_CALCNUM2;
                }
                
                /* Limit operation for times to read-accecss to Rx FIFO. */
                if (fifo_filled <= request)
                {
                    /* case, the number of filled memory of FIFO is smaller than request. */
                    limit = fifo_filled;
                }
                else
                {
                    /* case, the number of filled memory of FIFO is larger than request. */
                    limit = request;
                }
                
                /*  Read PCM data from Rx FIFO.
                    It is necessary to read 8 bytes as one time.
                */

                /* WAIT_LOOP */
                while ((limit - count) >= SSI_FIFONUM2)
                {
                 /* Casting pBuf to (const uint32_t *) is valid because it just points
                       address on 32 bit address space. */
                    *((uint32_t *)pBuf) = p_ssi_reg->SSIFRDR;
                    pBuf = ((uint32_t *)pBuf) + 1u;
                    *((uint32_t *)pBuf) = p_ssi_reg->SSIFRDR;
                    pBuf = ((uint32_t *)pBuf) + 1u;
                    count += SSI_FIFONUM2;
                }
                
                /* Clear Rx FIFO full flag */
                r_ssi_clear_flag_rdf( p_ssi_reg );
                
                /* Convert the number of FIFO to be plactically read 
                   from Rx FIFO into the number of Samples */
                if (SSI_SSICR_DWL08 == p_ssi_reg->SSICR.BIT.DWL)
                {
                    /* Casting "(count * SSI_CALCNUM2)" to uint8_t is valid because "count" shows
                        times to access to Tx FIFO and the size of Tx FIFO is 8. */
                    ret = (int8_t)(count * SSI_CALCNUM2);
                }
                else if (SSI_SSICR_DWL16 == p_ssi_reg->SSICR.BIT.DWL)
                {
                    /* Casting "count" to uint8_t is valid because "count" shows
                         times to access to Tx FIFO and the size of Tx FIFO is 8. */
                    ret = (int8_t)count;
                }
                else
                {
                    /* Casting "(count / SSI_CALCNUM2)" to uint8_t is valid because "count" shows
                         times to access to Tx FIFO and the size of Tx FIFO is 8. */
                    ret = (int8_t)(count / SSI_CALCNUM2);
                }
            }
        }
    }

    return ret;
}
/******************************************************************************
End of function  R_SSI_Read
******************************************************************************/

/******************************************************************************
* Function Name: R_SSI_Mute
* Description  : Executes mute(assert/deassert) operation. 
* Arguments    : const ssi_ch_t Channel      : [I] Channel Number of SSI
*              : const ssi_mute_t OnOff      : [I] Mute ON/OFF Switch
* Return Value : Extcution Result
******************************************************************************/
ssi_ret_t R_SSI_Mute(const ssi_ch_t Channel, const ssi_mute_t OnOff)
{
    uint32_t timeout;
    ssi_ret_t ret = SSI_SUCCESS;

    /* parameter check operation */
    ret = r_ssi_check_param_mute (Channel, OnOff);

    /* mute operation mainbody */
    if (SSI_SUCCESS == ret)  /* parameters are valid. */
    {
        volatile struct st_ssi R_SSI_EVENACCESS *p_ssi_reg;

        /* getting ssi register address */
        p_ssi_reg = (volatile struct st_ssi R_SSI_EVENACCESS *)r_ssi_get_ssi_address [Channel];
        
        if (SSI_IO_MODE_TX != r_ssi_get_iomode [Channel])
        {
        /* Error because Mute operation is available only for SSI_IO_MODE_TX. */
            ret = SSI_ERR_CHANNEL;
        }
        else
        {
        /* Start Mute operation. */
            const uint32_t ssifcr_tie = p_ssi_reg->SSIFCR.BIT.TIE;
            const uint32_t ssicr_ten = p_ssi_reg->SSICR.BIT.TEN;
            
            if (SSI_MUTE_ON == OnOff)
            {
                /* Mute on operaton (Mute assertion) */
                if ((SSI_SSIFCR_TIE_ENABLE == ssifcr_tie) && (SSI_SSICR_TEN_ENABLE == ssicr_ten))
                {
                    /* Setting to mute on SSI Tx */
                    p_ssi_reg->SSICR.BIT.TUIEN = SSI_SSICR_TUIEN_DISABLE;
                    p_ssi_reg->SSIFCR.BIT.TIE = SSI_SSIFCR_TIE_DISABLE;
                        
                    /* wait until Tx FIFO to be more than two empties. */

                    /* WAIT_LOOP */
                    while (p_ssi_reg->SSIFSR.BIT.TDC >= SSI_SSIFSR_TDC7)
                    {
                        /* Exception Check */
                        if (SSI_SSISR_IIRQ_ON == p_ssi_reg->SSISR.BIT.IIRQ)
                        {
                            /* Exception Error */
                            ret = SSI_ERR_EXCEPT;
                            break;
                        }
                    }

                    if (ret != SSI_ERR_EXCEPT)
                    {
                        /* Write "0" to Tx FIFO to disable transmit operation.
                           Wait until transmit under flow error arisen.
                           After the error arisen, disable transmit 
                               by TEN = SSI_SSICR_TEN_DISABLE.
                        */
                        p_ssi_reg->SSIFTDR = 0u;
                        p_ssi_reg->SSIFTDR = 0u;

                        /* Set WS Continue Mode to keep WS output */
                        p_ssi_reg->SSITDMR.BIT.CONT = 1u;

                        /* Wait until transmit under flow error flag to be set. */
                        /* initialize timeout counter. */
                        timeout = SSI_TIMEOUT_416USEC;

                        /* WAIT_LOOP */
                        while (SSI_SSISR_TUIRQ_OFF == p_ssi_reg->SSISR.BIT.TUIRQ)
                        {
                            /* Check loop count to detect abnormal condition. */
                            timeout = r_ssi_down_count ( timeout );
                            if ( 0 == timeout )
                            { 
                                /* Return from abnormal condition. */
                                ret = SSI_ERR_EXCEPT;
                                break;
                            }
                        }

                        if (ret != SSI_ERR_EXCEPT)
                        {
                            /* disable SSI transmit operation */
                            p_ssi_reg->SSICR.BIT.TEN = SSI_SSICR_TEN_DISABLE;
                            r_ssi_clear_flag_tuirq( p_ssi_reg );

                            /* check TSWNO to judge it abnormal condition or not. */
                            if ( SSI_SSISR_TSWNO_ON == p_ssi_reg->SSISR.BIT.TSWNO )
                            {
                                ret = SSI_ERR_EXCEPT;
                            }

                            /* Wait operation for IIRQ to be set. */
                            if ( ret == SSI_SUCCESS )
                            {
                                uint32_t timeout;

                                /* Wait until Idle mode Interrrupt status Flag to be set. */
                                /* initialize timeout counter. */
                                timeout = SSI_TIMEOUT_416USEC;

                                /* WAIT_LOOP */
                                while ( SSI_SSISR_IIRQ_OFF == p_ssi_reg->SSISR.BIT.IIRQ )
                                {
                                    /* Check loop count to detect abnormal condition. */
                                    timeout = r_ssi_down_count ( timeout );
                                    if ( 0u == timeout )
                                    { 
                                        /* Return from abnormal condition. */
                                        ret = SSI_ERR_EXCEPT;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            } /* End of Mute on process */
            else
            {
                /* Mute off operaton (Mute deassertion) */
                if ((SSI_SSIFCR_TIE_DISABLE == ssifcr_tie) && (SSI_SSICR_TEN_DISABLE == ssicr_ten))
                {
                    /*  check if Tx FIFO is empty or not */
                    if ( 0u == p_ssi_reg->SSIFSR.BIT.TDC)
                    { 
                        /*  case Tx FIFO is empty:
                            Write "0" to Tx FIFO. It requires to write 8 bytes to Tx FIFO.
                        */
                        p_ssi_reg->SSIFTDR = 0u;
                        p_ssi_reg->SSIFTDR = 0u;

                        /*  Enable transmit operation for Mute off by TEN = SSI_SSICR_TEN_ENABLE,
                            When Mute was turned ON, transmit operation is desabled.
                        */
                        p_ssi_reg->SSICR.BIT.TEN = SSI_SSICR_TEN_ENABLE;

                        /* Wait until Idle mode Interrrupt status Flag to be set. */
                        /* initialize timeout counter. */
                        timeout = SSI_TIMEOUT_416USEC;

                        /* WAIT_LOOP */
                        while ( SSI_SSISR_IIRQ_ON == p_ssi_reg->SSISR.BIT.IIRQ )
                        {
                            /* Check loop count to detect abnormal condition. */
                            timeout = r_ssi_down_count ( timeout );
                            if ( 0 == timeout )
                            {
                                ret = SSI_ERR_EXCEPT;  /* Return from abnormal condition. */
                                break;
                            }
                        }

                        if (ret != SSI_ERR_EXCEPT)
                        {
                            /* Reset WS Continue Mode (it should depend on config.h !!) */
                            p_ssi_reg->SSITDMR.BIT.CONT = 0u;

                            /* Setting to mute off SSI Tx */
                            r_ssi_clear_flag_tde( p_ssi_reg );
                            r_ssi_clear_flag_tuirq( p_ssi_reg );
                            p_ssi_reg->SSIFCR.BIT.TIE = SSI_SSIFCR_TIE_ENABLE;
                            p_ssi_reg->SSICR.BIT.TUIEN = SSI_SSICR_TUIEN_ENABLE;
                        }
                    }
                    else
                    {
                        ret = SSI_ERR_EXCEPT;
                    }
                }
            }
        }
    }

    return ret;
}

/******************************************************************************
End of function  R_SSI_Mute
******************************************************************************/

/*******************************************************************************
* Function Name: R_SSI_GetVersion
* Description : Returns the version of this module. The version number is
* encoded where the top 2 bytes are the major version number and
* the bottom 2 bytes are the minor version number. For example,
* Rev 4.25 would be 0x00040019.
* Arguments : none
* Return Value : Version Number
*******************************************************************************/
uint32_t R_SSI_GetVersion( void )
{
    uint32_t version_number = 0u;

    /* Bring in major version number. */
    version_number = ((uint16_t)SSI_API_VERSION_MAJOR) << 16u;

    /* Bring in minor version number. */
    version_number |= (uint16_t)SSI_API_VERSION_MINOR;

    return version_number;
}

/******************************************************************************
End of function  R_SSI_GetVersion
******************************************************************************/

/******************************************************************************
* Function Name: R_SSI_ClearFlagTxUnderFlow
* Description  : Clears Tx Under Flow flag. 
* Arguments    : Channel: shows SSI0 or SSI1 
* Return Value : Extcution Result
******************************************************************************/
ssi_ret_t R_SSI_ClearFlagTxUnderFlow ( const ssi_ch_t Channel )
{
    ssi_ret_t ret = SSI_SUCCESS;

    /* parameter & hardware lock checking operation */
    ret = r_ssi_check_param (Channel);

    /* flag clear operation mainbody */
    if ( SSI_SUCCESS == ret )
    {
        volatile struct st_ssi R_SSI_EVENACCESS *p_ssi_reg;

        /* getting ssi register address */
        p_ssi_reg = (volatile struct st_ssi R_SSI_EVENACCESS *)r_ssi_get_ssi_address [Channel];

        /* flag clear operation */
        r_ssi_clear_flag_tuirq(p_ssi_reg);
    }

    return ret;
}

/******************************************************************************
End of function R_SSI_ClearFlagTxUnderFlow
******************************************************************************/

/******************************************************************************
* Function Name: R_SSI_ClearFlagTxOverFlow
* Description  : Clears Tx Over Flow flag. 
* Arguments    : Channel: shows SSI0 or SSI1 
* Return Value : Extcution Result
******************************************************************************/
ssi_ret_t R_SSI_ClearFlagTxOverFlow ( const ssi_ch_t Channel )
{
    ssi_ret_t ret = SSI_SUCCESS;

    /* parameter & hardware lock checking operation */
    ret = r_ssi_check_param (Channel);

    /* flag clear operation mainbody */
    if ( SSI_SUCCESS == ret )
    {
        volatile struct st_ssi R_SSI_EVENACCESS *p_ssi_reg;

        /* getting ssi register address */
        p_ssi_reg = (volatile struct st_ssi R_SSI_EVENACCESS *)r_ssi_get_ssi_address [Channel];

        /* flag clear operation */
        r_ssi_clear_flag_toirq(p_ssi_reg);
    }

    return ret;
}

/******************************************************************************
End of function R_SSI_ClearFlagTxOverFlow
******************************************************************************/

/******************************************************************************
* Function Name: R_SSI_ClearFlagRxUnderFlow
* Description  : Clears Rx Under Flow flag. 
* Arguments    : Channel: shows SSI0 or SSI1 
* Return Value : Extcution Result
******************************************************************************/
ssi_ret_t R_SSI_ClearFlagRxUnderFlow ( const ssi_ch_t Channel )
{
    ssi_ret_t ret = SSI_SUCCESS;

    /* parameter & hardware lock checking operation */
    ret = r_ssi_check_param (Channel);

    /* flag clear operation mainbody */
    if ( SSI_SUCCESS == ret )
    {
        volatile struct st_ssi R_SSI_EVENACCESS *p_ssi_reg;

        /* getting ssi register address */
        p_ssi_reg = (volatile struct st_ssi R_SSI_EVENACCESS *)r_ssi_get_ssi_address [Channel];

        /* flag clear operation */
        r_ssi_clear_flag_ruirq(p_ssi_reg);
    }

    return ret;
}

/******************************************************************************
End of function R_SSI_ClearFlagRxUnderFlow
******************************************************************************/

/******************************************************************************
* Function Name: R_SSI_ClearFlagRxOverFlow
* Description  : Clears Rx Over Flow flag. 
* Arguments    : Channel: SSI_CH0 or SSI_CH1 
* Return Value : Extcution Result
******************************************************************************/
ssi_ret_t R_SSI_ClearFlagRxOverFlow ( const ssi_ch_t Channel )
{
    ssi_ret_t ret = SSI_SUCCESS;

    /* parameter & hardware lock checking operation */
    ret = r_ssi_check_param (Channel);

    /* flag clear operation mainbody */
    if ( SSI_SUCCESS == ret )
    {
        volatile struct st_ssi R_SSI_EVENACCESS *p_ssi_reg;

        /* getting ssi register address */
        p_ssi_reg = (volatile struct st_ssi R_SSI_EVENACCESS *)r_ssi_get_ssi_address [Channel];

        /* flag clear operation */
        r_ssi_clear_flag_roirq(p_ssi_reg);
    }

    return ret;
}

/******************************************************************************
End of function R_SSI_ClearFlagRxOverFlow
******************************************************************************/

/******************************************************************************
* Function Name: R_SSI_GetFlagTxUnderFlow
* Description  : checks Tx Under Flow flag and return the flag is set or not. 
* Arguments    : Channel: target SSI channel number to check flag
* Return Value : shows the flag is "0" or "1" as follows;
*                 SSI_FLAG_CLR : flag is 0
*                 SSI_FLAG_SET : flag is 1
******************************************************************************/
ssi_ret_t R_SSI_GetFlagTxUnderFlow ( const ssi_ch_t Channel )
{
    ssi_ret_t ret = SSI_SUCCESS;

    /* parameter & hardware lock checking operation */
    ret = r_ssi_check_param (Channel);

    /* flag get operation mainbody */
    if ( SSI_SUCCESS == ret )
    {
        volatile struct st_ssi R_SSI_EVENACCESS *p_ssi_reg;

        /* getting ssi register address */
        p_ssi_reg = (volatile struct st_ssi R_SSI_EVENACCESS *)r_ssi_get_ssi_address [Channel];

        /* get enum member for target ssi channel. */
        if ( 0 == p_ssi_reg->SSISR.BIT.TUIRQ )
        {
            ret = SSI_FLAG_CLR; 
        }
        else
        {
            ret = SSI_FLAG_SET;
        }
    }

    return ret;
}

/******************************************************************************
End of function  R_SSI_GetFlagTxUnderFlow
******************************************************************************/

/******************************************************************************
* Function Name: R_SSI_GetFlagTxOverFlow
* Description  : checks Tx Over Flow flag and return the flag is set or not. 
* Arguments    : Channel: target SSI channel number to check flag
* Return Value : shows the flag is "0" or "1" as follows;
*                 SSI_FLAG_CLR : flag is 0
*                 SSI_FLAG_SET : flag is 1
******************************************************************************/
ssi_ret_t R_SSI_GetFlagTxOverFlow ( const ssi_ch_t Channel )
{
    ssi_ret_t ret = SSI_SUCCESS;

    /* parameter & hardware lock checking operation */
    ret = r_ssi_check_param (Channel);

    /* flag get operation mainbody */
    if ( SSI_SUCCESS == ret )
    {
        volatile struct st_ssi R_SSI_EVENACCESS *p_ssi_reg;

        /* getting ssi register address */
        p_ssi_reg = (volatile struct st_ssi R_SSI_EVENACCESS *)r_ssi_get_ssi_address [Channel];

        /* get enum member for target ssi channel. */
        if ( 0 == p_ssi_reg->SSISR.BIT.TOIRQ )
        {
            ret = SSI_FLAG_CLR; 
        }
        else
        {
            ret = SSI_FLAG_SET;
        }
    }

    return ret;
}

/******************************************************************************
End of function  R_SSI_GetFlagTxOverFlow
******************************************************************************/

/******************************************************************************
* Function Name: R_SSI_GetFlagRxUnderFlow
* Description  : checks Rx Under Flow flag and return the flag is set or not. 
* Arguments    : Channel: target SSI channel number to check flag
* Return Value : shows the flag is "0" or "1" as follows;
*                 SSI_FLAG_CLR : flag is 0
*                 SSI_FLAG_SET : flag is 1
******************************************************************************/
ssi_ret_t R_SSI_GetFlagRxUnderFlow ( const ssi_ch_t Channel )
{
    ssi_ret_t ret = SSI_SUCCESS;

    /* parameter & hardware lock checking operation */
    ret = r_ssi_check_param (Channel);

    /* flag get operation mainbody */
    if ( SSI_SUCCESS == ret )
    {
        volatile struct st_ssi R_SSI_EVENACCESS *p_ssi_reg;

        /* getting ssi register address */
        p_ssi_reg = (volatile struct st_ssi R_SSI_EVENACCESS *)r_ssi_get_ssi_address [Channel];

        /* get enum member for target ssi channel. */
        if ( 0 == p_ssi_reg->SSISR.BIT.RUIRQ )
        {
            ret = SSI_FLAG_CLR; 
        }
        else
        {
            ret = SSI_FLAG_SET;
        }
    }

    return ret;
}

/******************************************************************************
End of function  R_SSI_GetFlagRxUnderFlow
******************************************************************************/

/******************************************************************************
* Function Name: R_SSI_GetFlagRxOverFlow
* Description  : checks Rx Over Flow flag and return the flag is set or not. 
* Arguments    : Channel: target SSI channel number to check flag
* Return Value : shows the flag is "0" or "1" as follows;
*                 SSI_FLAG_CLR : flag is 0
*                 SSI_FLAG_SET : flag is 1
******************************************************************************/
ssi_ret_t R_SSI_GetFlagRxOverFlow ( const ssi_ch_t Channel )
{
    ssi_ret_t ret = SSI_SUCCESS;

    /* parameter & hardware lock checking operation */
    ret = r_ssi_check_param (Channel);

    /* flag get operation mainbody */
    if ( SSI_SUCCESS == ret )
    {
        volatile struct st_ssi R_SSI_EVENACCESS *p_ssi_reg;

        /* getting ssi register address */
        p_ssi_reg = (volatile struct st_ssi R_SSI_EVENACCESS *)r_ssi_get_ssi_address [Channel];

        /* get enum member for target ssi channel. */
        if ( 0 == p_ssi_reg->SSISR.BIT.ROIRQ )
        {
            ret = SSI_FLAG_CLR; 
        }
        else
        {
            ret = SSI_FLAG_SET;
        }
    }

    return ret;
}

/******************************************************************************
End of function R_SSI_GetFlagRxOverFlow
******************************************************************************/

