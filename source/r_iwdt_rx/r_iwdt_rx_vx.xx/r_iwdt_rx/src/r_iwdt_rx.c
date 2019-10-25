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
* Copyright (C) 2013-2019 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : r_iwdt_rx.c
* Description  : Functions for using IWDT on RX devices.
***********************************************************************************************************************/
/**********************************************************************************************************************
* History : DD.MM.YYYY Version Description
*           07.06.2013 1.00    First Release
*           06.09.2013 1.10    Add NMI interrupt enable, update name of enumeration
*           19.03.2014 1.20    Removed unnecessary #includes. Changed parameter check for p_cfg->timeout.
*           19.12.2016 1.30    Added support for RX24U, RX24T(512KB)
*                              FIT_NO_PTR check added to NULL check.
*           21.07.2017 1.80    Added support for RX65N-2M, RX130-512KB
*           28.09.2018 1.90    Added support for RX66T
*                              Updated follow GSCE coding rule 5.0
*           01.02.2019 2.00    Added support for RX72T, RX65N-64pin.
*                              Bug fix: R_WDT_Open(), R_IWDT_Open() invalidated if either module is in auto-start mode.
*           20.05.2019 3.00    Added support for GNUC and ICCRX.
*           15.08.2019 3.20    Fixed warnings in IAR.
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include "platform.h"
/* Defines for IWDT support */
#include "r_iwdt_rx_config.h"
#include "r_iwdt_rx_if.h"

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/
#if ((BSP_CFG_OFS0_REG_VALUE & OFS0_IWDT_DISABLED) == OFS0_IWDT_DISABLED) /* Register start mode */
/* State variable for this package. */
static bool already_opened = false;

/* Internal functions. */
static iwdt_err_t iwdt_init_register_start_mode(iwdt_config_t *p_cfg);
#if (1 == IWDT_CFG_PARAM_CHECKING_ENABLE)
static bool iwdt_parameter_check(iwdt_config_t *p_cfg);
#endif /* IWDT_CFG_PARAM_CHECKING_ENABLE */
#endif /* BSP_CFG_OFS0_REG_VALUE */

static inline bool acquire_hw_lock(void);
static inline void release_hw_lock(void);

/***********************************************************************************************************************
* Function Name: R_IWDT_Open
* Description  : This function configures the IWDT counter options by initializing
*                the associated registers. This API is affected in Register start mode only.
* Arguments    : p_cfg -
*                    Pointer to configuration structure of type iwdt_ config_t.
* Return Value : IWDT_SUCCESS -
*                    IWDT is initialized successful.
*                IWDT_ERR_OPEN_IGNORED -
*                    The calling to this function is ignored because it is already called.
*                IWDT_ERR_INVALID_ARG -
*                    An element of the pCfg structure contains an invalid value.
*                IWDT_ERR_NULL_PTR -
*                    pCfg pointer is NULL.
*                IWDT_ERR_BUSY -
*                    IWDT resource cannot be taken. Try to call this function again.
***********************************************************************************************************************/
#if ((BSP_CFG_OFS0_REG_VALUE & OFS0_IWDT_DISABLED) == OFS0_IWDT_DISABLED) /* Register start mode */
iwdt_err_t R_IWDT_Open (void * const p_cfg)
{
    iwdt_err_t   err;
    bool         ret;

    err = IWDT_SUCCESS;

    /* CHECK ARGUMENTS */
#if (1 == IWDT_CFG_PARAM_CHECKING_ENABLE)
    /* Check argument p_cfg */
    if ((NULL == p_cfg) || (FIT_NO_PTR == p_cfg))
    {
        return IWDT_ERR_NULL_PTR;
    }

    /* Casting void* type to iwdt_config_t* type is valid */
    ret = iwdt_parameter_check((iwdt_config_t *)p_cfg);
    if (false == ret)
    {
        return IWDT_ERR_INVALID_ARG;
    }
#endif

    ret = acquire_hw_lock();
    if (false == ret)
    {
        /* Lock not obtained, return error */
        return IWDT_ERR_BUSY;
    }

    /* Lock obtained, we can change state */
    if (true == already_opened)
    {
        /* Open function is already called */
        release_hw_lock();
        return IWDT_ERR_OPEN_IGNORED;
    }

    /* Casting void* type to iwdt_config_t* type is valid */
    err = iwdt_init_register_start_mode((iwdt_config_t *)p_cfg);

    /* Update status */
    already_opened = true;

    release_hw_lock();
    return err;
}
/******************************************************************************
End of function R_IWDT_Open
******************************************************************************/

/***********************************************************************************************************************
* Function Name: iwdt_init_register_start_mode
* Description  : Make setting to IWDT register for initialization.
* Arguments    : p_cfg -
*                    Pointer to configuration structure of type iwdt_config_t.
* Return Value : IWDT_SUCCESS -
*                    IWDT is initialized successful.
***********************************************************************************************************************/
static iwdt_err_t iwdt_init_register_start_mode(iwdt_config_t *p_cfg)
{
    /* Set Time-out period, Clock Division Ratio, Window Start/End Position */
    IWDT.IWDTCR.WORD = ((((uint16_t)p_cfg->timeout) | ((uint16_t)p_cfg->iwdtclk_div)) | \
        (((uint16_t)p_cfg->window_start) | ((uint16_t)p_cfg->window_end)));

    /* Set reset output or NMI output */
    IWDT.IWDTRCR.BYTE = (uint8_t)p_cfg->timeout_control;

    if (IWDT_TIMEOUT_NMI == p_cfg->timeout_control)
    {
        /* Enable NMI interrupt */
        ICU.NMIER.BIT.IWDTEN = 0x1;
    }

    /* Set enable or disable count stop in low-power mode */
    IWDT.IWDTCSTPR.BYTE = (uint8_t)p_cfg->count_stop_enable;

    return IWDT_SUCCESS;
}
/******************************************************************************
End of function iwdt_init_register_start_mode
******************************************************************************/

/***********************************************************************************************************************
* Function Name: iwdt_parameter_check
* Description  : Check validity of input parameter of R_IWDT_Open().
* Arguments    : p_cfg -
*                    Pointer to configuration structure of type iwdt_config_t.
* Return Value : true -
*                    Input is valid.
*                false -
*                    Input is invalid.
***********************************************************************************************************************/
#if (1 == IWDT_CFG_PARAM_CHECKING_ENABLE)
static bool iwdt_parameter_check(iwdt_config_t *p_cfg)
{
    bool ret = true;

    if (p_cfg->timeout >= IWDT_NUM_TIMEOUTS)
    {
        ret = false;
    }

    if (((IWDT_CLOCK_DIV_1   != p_cfg->iwdtclk_div)  && \
         (IWDT_CLOCK_DIV_16  != p_cfg->iwdtclk_div)) && \
       (((IWDT_CLOCK_DIV_32  != p_cfg->iwdtclk_div)  && \
         (IWDT_CLOCK_DIV_64  != p_cfg->iwdtclk_div)) && \
        ((IWDT_CLOCK_DIV_128 != p_cfg->iwdtclk_div)  && \
         (IWDT_CLOCK_DIV_256 != p_cfg->iwdtclk_div))))
    {
        ret = false;
    }

    if (((IWDT_WINDOW_END_75 != p_cfg->window_end)  && \
         (IWDT_WINDOW_END_50 != p_cfg->window_end)) && \
        ((IWDT_WINDOW_END_25 != p_cfg->window_end)  && \
         (IWDT_WINDOW_END_0  != p_cfg->window_end)))
    {
        ret = false;
    }

    if (((IWDT_WINDOW_START_25  != p_cfg->window_start)  && \
         (IWDT_WINDOW_START_50  != p_cfg->window_start)) && \
        ((IWDT_WINDOW_START_75  != p_cfg->window_start)  && \
         (IWDT_WINDOW_START_100 != p_cfg->window_start)))
    {
        ret = false;
    }

    if ((IWDT_TIMEOUT_RESET != p_cfg->timeout_control) && \
        (IWDT_TIMEOUT_NMI   != p_cfg->timeout_control))
    {
        ret = false;
    }

    if ((IWDT_COUNT_STOP_DISABLE != p_cfg->count_stop_enable) && \
        (IWDT_COUNT_STOP_ENABLE  != p_cfg->count_stop_enable))
    {
        ret = false;
    }

    return ret;
}
/******************************************************************************
End of function iwdt_parameter_check
******************************************************************************/
#endif /* IWDT_CFG_PARAM_CHECKING_ENABLE */
#endif /* BSP_CFG_OFS0_REG_VALUE */


/***********************************************************************************************************************
* Function Name: R_IWDT_Control
* Description  : This function performs getting IWDT status (underflow error status,
                  refresh error status and IWDT counter value) and refreshing the down-counter of IWDT.
                 It is used in both Auto start mode and Register start mode.
* Arguments    : cmd -
*                    Command to refresh IWDT counter or get IWDT status
*                p_status -
*                    Stores the counter status.
* Return Value : IWDT_SUCCESS -
*                    Setting to appropriate register for performing a refreshing successfully.
*                IWDT_ERR_INVALID_ARG -
*                    Invalid argument.
*                IWDT_ERR_NULL_PTR -
*                    p_status is NULL.
*                IWDT_ERR_NOT_OPENED -
*                    Open function is not called yet.
*                IWDT_ERR_BUSY -
*                    IWDT resource cannot be taken. Try to call this function again.
***********************************************************************************************************************/
iwdt_err_t R_IWDT_Control(iwdt_cmd_t const cmd, uint16_t * p_status)
{
    bool ret = false;

    /* CHECK ARGUMENTS */
#if (1 == IWDT_CFG_PARAM_CHECKING_ENABLE)
    if ((IWDT_CMD_REFRESH_COUNTING != cmd) && (IWDT_CMD_GET_STATUS != cmd))
    {
        return IWDT_ERR_INVALID_ARG;
    }

    /* Check argument p_status */
    if (((NULL == p_status) || (FIT_NO_PTR == p_status)) && (IWDT_CMD_GET_STATUS == cmd))
    {
        return IWDT_ERR_NULL_PTR;
    }
#endif

    ret = acquire_hw_lock();
    if (false == ret)
    {
        /* Lock not obtained, return error */
        return IWDT_ERR_BUSY;
    }

    /* Lock obtained */
#if ((BSP_CFG_OFS0_REG_VALUE & OFS0_IWDT_DISABLED) == OFS0_IWDT_DISABLED) /* Register start mode */
    if (false == already_opened)
    {
        /* Open function has not called yet */
        release_hw_lock();
        return IWDT_ERR_NOT_OPENED;
    }
#endif

    switch (cmd)
    {
        case IWDT_CMD_REFRESH_COUNTING:
        {
            /* Make settings to IWDTRR register to refresh the counter */
            IWDT.IWDTRR = 0x00u;

            /* Write FFh to register IWDTRR to refresh the counter */
            IWDT.IWDTRR = 0xFFu;
            break;
        }
        case IWDT_CMD_GET_STATUS:
        {
            /* Get value 16 bits IWDTSR register */
            *p_status = IWDT.IWDTSR.WORD;
            break;
        }
        default:
        {
            break;
        }
    }

    release_hw_lock();
    return IWDT_SUCCESS;
}
/******************************************************************************
End of function R_IWDT_Control
******************************************************************************/

/***********************************************************************************************************************
* Function Name: R_IWDT_GetVersion
* Description  : Returns the current version of this module. The version number is encoded where the top 2 bytes are the
*                major version number and the bottom 2 bytes are the minor version number.
* Arguments    : none
* Return Value : Version of this module.
***********************************************************************************************************************/
uint32_t  R_IWDT_GetVersion (void)
{
    uint32_t const version = (IWDT_RX_VERSION_MAJOR << 16) | IWDT_RX_VERSION_MINOR;

    return version;
}
/******************************************************************************
End of function R_IWDT_GetVersion
******************************************************************************/

/*****************************************************************************
* Function Name: acquire_hw_lock
* Description  : get the hardware lock BSP_LOCK_IWDT.
* Arguments    : None.
* Return Value : true -
*                  the lock is acquired successfully
*                false -
*                   fail to get the lock
******************************************************************************/
static inline bool acquire_hw_lock(void)
{
    return R_BSP_HardwareLock(BSP_LOCK_IWDT);
}
/******************************************************************************
End of function acquire_hw_lock
******************************************************************************/

/*****************************************************************************
* Function Name: release_hw_lock
* Description  : release hardware lock BSP_LOCK_IWDT.
* Arguments    : None.
* Return Value : None.
******************************************************************************/
static inline void release_hw_lock(void)
{
    R_BSP_HardwareUnlock(BSP_LOCK_IWDT);
}
/******************************************************************************
End of function release_hw_lock
******************************************************************************/
