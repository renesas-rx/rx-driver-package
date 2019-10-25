/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
* other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
* EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
* SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
* SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
* this software. By using this software, you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2016 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : r_wdt_rx.c
* Description  : Functions for using WDT on RX devices.
***********************************************************************************************************************/
/**********************************************************************************************************************
* History : DD.MM.YYYY Version Description
*           28.04.2016 1.00    First Release.
*           01.10.2017 1.10    A comment of MCU register setting was added.
*           20.10.2017 1.20    Changed global variable name, added comments and brackets 
*                              according to Renesas GSCE Coding Standards 5.00.
*           01.02.2019 1.40    Added support for RX72T, RX65N-64pin.
*                              Bug fix: R_WDT_Open(), R_IWDT_Open() invalidated if either module is in auto-start mode.
*           20.05.2019 2.00    Added support for GNUC and ICCRX.
*           15.08.2019 2.20    Fixed warnings in IAR.
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include "platform.h"
/* Defines for WDT support */
#include "r_wdt_rx_config.h"
#include "r_wdt_rx_if.h"

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/
#if ((BSP_CFG_OFS0_REG_VALUE & OFS0_WDT_DISABLED) == OFS0_WDT_DISABLED) /* Register start mode */
/* State variable for this package. */
static bool gs_already_opened = false;

/* Internal functions. */
static wdt_err_t wdt_init_register_start_mode(wdt_config_t *p_cfg);
#if (1 == WDT_CFG_PARAM_CHECKING_ENABLE)
static bool wdt_parameter_check(wdt_config_t *p_cfg);

#endif /* WDT_CFG_PARAM_CHECKING_ENABLE */
#endif /* BSP_CFG_OFS0_REG_VALUE */

static inline bool acquire_hw_lock(void);
static inline void release_hw_lock(void);

/***********************************************************************************************************************
* Function Name: R_WDT_Open
* Description  : This function configures the WDT counter options by initializing
*                the associated registers. This API is affected in Register start mode only.
* Arguments    : p_cfg -
*                    Pointer to configuration structure of type wdt_ config_t.
* Return Value : WDT_SUCCESS -
*                    WDT is initialized successful.
*                WDT_ERR_OPEN_IGNORED -
*                    The calling to this function is ignored because it is already called.
*                WDT_ERR_INVALID_ARG -
*                    An element of the pCfg structure contains an invalid value.
*                WDT_ERR_NULL_PTR -
*                    pCfg pointer is NULL.
*                WDT_ERR_BUSY -
*                    WDT resource cannot be taken. Try to call this function again.
***********************************************************************************************************************/
#if ((BSP_CFG_OFS0_REG_VALUE & OFS0_WDT_DISABLED) == OFS0_WDT_DISABLED) /*Register start mode*/
wdt_err_t R_WDT_Open (void * const p_cfg)
{
    wdt_err_t   err;
    bool        ret;

    err = WDT_SUCCESS;

    /* CHECK ARGUMENTS */
#if (1 == WDT_CFG_PARAM_CHECKING_ENABLE)
    if (NULL == p_cfg)
    {
        return WDT_ERR_NULL_PTR;
    }

    /* WDT parameter checking */
    ret = wdt_parameter_check((wdt_config_t *)p_cfg);
    if (false == ret)
    {
        return WDT_ERR_INVALID_ARG;
    }
#endif

    ret = acquire_hw_lock();
    if (false == ret)
    {
        /* Lock not obtained, return error */
        return WDT_ERR_BUSY;
    }

    /* Lock obtained, we can change state */
    if (true == gs_already_opened)
    {
        /* Open function is already called */
        release_hw_lock();
        return WDT_ERR_OPEN_IGNORED;
    }

    /* Make setting to WDT register for initialization */
    err = wdt_init_register_start_mode((wdt_config_t *)p_cfg);

    /* Update status */
    gs_already_opened = true;

    release_hw_lock();
    return err;
}
/******************************************************************************
End of function R_WDT_Open
******************************************************************************/

/***********************************************************************************************************************
* Function Name: wdt_init_register_start_mode
* Description  : Make setting to WDT register for initialization.
* Arguments    : p_cfg -
*                    Pointer to configuration structure of type wdt_config_t.
* Return Value : WDT_SUCCESS -
*                    WDT is initialized successful.
***********************************************************************************************************************/
static wdt_err_t wdt_init_register_start_mode(wdt_config_t *p_cfg)
{
    /* Set Time-out period, Clock Division Ratio, Window Start/End Position */
    WDT.WDTCR.WORD = (((uint16_t)(p_cfg->timeout) | (uint16_t)(p_cfg->wdtcks_div)) | \
                     ((uint16_t)(p_cfg->window_start) | (uint16_t)(p_cfg->window_end)));

    /* Set reset output or NMI output */
    WDT.WDTRCR.BYTE = (uint8_t)p_cfg->timeout_control;

    if (WDT_TIMEOUT_NMI == p_cfg->timeout_control)
    {
        /* Enable NMI interrupt
         NMIER - Non-Maskable Interrupt Enable Register
         b0  WDTEN - WDT Underflow/Refresh Error Enable - WDT Underflow/Refresh Error interrupt is enabled */
        ICU.NMIER.BIT.WDTEN = 0x1;
    }

    return WDT_SUCCESS;
}
/******************************************************************************
End of function wdt_init_register_start_mode
******************************************************************************/

/***********************************************************************************************************************
* Function Name: wdt_parameter_check
* Description  : Check validity of input parameter of R_WDT_Open().
* Arguments    : p_cfg -
*                    Pointer to configuration structure of type wdt_config_t.
* Return Value : true -
*                    Input is valid.
*                false -
*                    Input is invalid.
***********************************************************************************************************************/
#if (1 == WDT_CFG_PARAM_CHECKING_ENABLE)
static bool wdt_parameter_check(wdt_config_t *p_cfg)
{
    bool ret = true;

    if (p_cfg->timeout >= WDT_NUM_TIMEOUTS)
    {
        ret = false;
    }

    if (((WDT_CLOCK_DIV_4    != p_cfg->wdtcks_div)  && \
         (WDT_CLOCK_DIV_64   != p_cfg->wdtcks_div)) && \
       (((WDT_CLOCK_DIV_128  != p_cfg->wdtcks_div)  && \
         (WDT_CLOCK_DIV_512  != p_cfg->wdtcks_div)) && \
        ((WDT_CLOCK_DIV_2048 != p_cfg->wdtcks_div)  && \
         (WDT_CLOCK_DIV_8192 != p_cfg->wdtcks_div))))
    {
        ret = false;
    }

    if (((WDT_WINDOW_END_75 != p_cfg->window_end)  && \
         (WDT_WINDOW_END_50 != p_cfg->window_end)) && \
        ((WDT_WINDOW_END_25 != p_cfg->window_end)  && \
         (WDT_WINDOW_END_0  != p_cfg->window_end)))
    {
        ret = false;
    }

    if (((WDT_WINDOW_START_25  != p_cfg->window_start)  && \
         (WDT_WINDOW_START_50  != p_cfg->window_start)) && \
        ((WDT_WINDOW_START_75  != p_cfg->window_start)  && \
         (WDT_WINDOW_START_100 != p_cfg->window_start)))
    {
        ret = false;
    }

    if ((WDT_TIMEOUT_RESET != p_cfg->timeout_control) && \
        (WDT_TIMEOUT_NMI   != p_cfg->timeout_control))
    {
        ret = false;
    }

    return ret;
}
/******************************************************************************
End of function wdt_parameter_check
******************************************************************************/
#endif /* WDT_CFG_PARAM_CHECKING_ENABLE */
#endif /* BSP_CFG_OFS0_REG_VALUE */


/***********************************************************************************************************************
* Function Name: R_WDT_Control
* Description  : This function performs getting WDT status (underflow error status,
                  refresh error status and WDT counter value) and refreshing the down-counter of WDT.
                 It is used in both Auto start mode and Register start mode.
* Arguments    : cmd -
*                    Command to refresh WDT counter or get WDT status
*                p_status -
*                    Stores the counter status.
* Return Value : WDT_SUCCESS -
*                    Setting to appropriate register for performing a refreshing successfully.
*                WDT_ERR_INVALID_ARG -
*                    Invalid argument.
*                WDT_ERR_NULL_PTR -
*                    p_status is NULL.
*                WDT_ERR_NOT_OPENED -
*                    Open function is not called yet.
*                WDT_ERR_BUSY -
*                    WDT resource cannot be taken. Try to call this function again.
***********************************************************************************************************************/
wdt_err_t R_WDT_Control(wdt_cmd_t const cmd, uint16_t * p_status)
{
    bool ret = false;

    /* CHECK ARGUMENTS */
#if (1 == WDT_CFG_PARAM_CHECKING_ENABLE)
    if ((WDT_CMD_REFRESH_COUNTING != cmd) && (WDT_CMD_GET_STATUS != cmd))
    {
        return WDT_ERR_INVALID_ARG;
    }

    if ((NULL == p_status) && (WDT_CMD_GET_STATUS == cmd))
    {
        return WDT_ERR_NULL_PTR;
    }
#endif

    ret = acquire_hw_lock();
    if (false == ret)
    {
        /* Lock not obtained, return error */
        return WDT_ERR_BUSY;
    }

    /* Lock obtained */
#if ((BSP_CFG_OFS0_REG_VALUE & OFS0_WDT_DISABLED) == OFS0_WDT_DISABLED) /* Register start mode */
    if (false == gs_already_opened)
    {
        /* Open function has not called yet */
        release_hw_lock();
        return WDT_ERR_NOT_OPENED;
    }
#endif

    switch (cmd)
    {
        case WDT_CMD_REFRESH_COUNTING:
            {
            /* Make settings to WDTRR register to refresh the counter
            WDTRR - WDT Refresh Register
            Refresh the down-counter of WDT */
            WDT.WDTRR = 0x00u;

            /* Set WDTRR register value to 0xFFu to refresh counting */
            WDT.WDTRR = 0xFFu;
            }
        break;

        case WDT_CMD_GET_STATUS:
            {
            /* Get WDT status from WDTSR register */
            *p_status = WDT.WDTSR.WORD;
            }
        break;

        default:
            {
                R_BSP_NOP();
            }
        break;
    }

    release_hw_lock();
    return WDT_SUCCESS;
}
/******************************************************************************
End of function R_WDT_Control
******************************************************************************/

/***********************************************************************************************************************
* Function Name: R_WDT_GetVersion
* Description  : Returns the current version of this module. The version number is encoded where the top 2 bytes are the
*                major version number and the bottom 2 bytes are the minor version number.
* Arguments    : none
* Return Value : Version of this module.
***********************************************************************************************************************/
uint32_t  R_WDT_GetVersion (void)
{
    uint32_t const version = (WDT_RX_VERSION_MAJOR << 16) | WDT_RX_VERSION_MINOR;

    return version;
}
/******************************************************************************
End of function R_WDT_GetVersion
******************************************************************************/

/*****************************************************************************
* Function Name: acquire_hw_lock
* Description  : get the hardware lock BSP_LOCK_WDT.
* Arguments    : None.
* Return Value : true -
*                  the lock is acquired successfully
*                false -
*                   fail to get the lock
******************************************************************************/
static inline bool acquire_hw_lock(void)
{
    return R_BSP_HardwareLock(BSP_LOCK_WDT);
}
/******************************************************************************
End of function acquire_hw_lock
******************************************************************************/

/*****************************************************************************
* Function Name: release_hw_lock
* Description  : release hardware lock BSP_LOCK_WDT.
* Arguments    : None.
* Return Value : None.
******************************************************************************/
static inline void release_hw_lock(void)
{
    R_BSP_HardwareUnlock(BSP_LOCK_WDT);
}
/******************************************************************************
End of function release_hw_lock
******************************************************************************/
