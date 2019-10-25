/**********************************************************************************************************************
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
* Copyright (C) 2017(2019) Renesas Electronics Corporation. All rights reserved.
**********************************************************************************************************************/
/**********************************************************************************************************************
* System Name  : SDHI Driver
* File Name    : r_sdhi_dev.c
* Version      : 2.04
* Device       : RX231
* Abstract     : API & Sub module
* Tool-Chain   : For RX231 Group e2_studio
* OS           : not use
* H/W Platform : RSK board for RX231
* Description  : Interface file for SDHI API for RX
* Limitation   : None
**********************************************************************************************************************/
/**********************************************************************************************************************
* History      : DD.MM.YYYY Version Description
*              : 31.07.2017 2.00    First Release
*              : 20.05.2019 2.04    Added support for GNUC and ICCRX.
*                                   Fixed coding style. 
**********************************************************************************************************************/

/**********************************************************************************************************************
Includes <System Includes> , "Project Includes"
**********************************************************************************************************************/
#include "platform.h"
#if defined(BSP_MCU_RX231)

#include "r_sdhi_rx_if.h"
#include ".\src\r_sdhi_rx_private.h"
#include ".\src\targets\rx231\r_sdhi_rx_target.h"

/**********************************************************************************************************************
Macro definitions
**********************************************************************************************************************/

/**********************************************************************************************************************
Typedef definitions
**********************************************************************************************************************/

/**********************************************************************************************************************
Private global variables and functions
**********************************************************************************************************************/

/**********************************************************************************************************************
* Outline      : SD Buffer Access Interrupt Handler
* Function Name: r_sdhi_dev_sbfai_isr
* Description  : Call a callback function.
* Arguments    : None
* Return Value : None
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
R_BSP_PRAGMA_STATIC_INTERRUPT(r_sdhi_dev_sbfai_isr, VECT(SDHI, SBFAI))
R_BSP_ATTRIB_STATIC_INTERRUPT void r_sdhi_dev_sbfai_isr(void)
{
    sdhi_sdhndl_t   * p_hndl = 0;
    uint32_t channel = 0;

    p_hndl = SDHI_GET_HNDL(channel);

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if (0 == p_hndl)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_DEV, __LINE__);
        return;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    if (p_hndl->int_sdbuff_callback)
    {
        (*p_hndl->int_sdbuff_callback)(0);
    }
} /* End of function r_sdhi_dev_sbfai_isr() */

/**********************************************************************************************************************
* Outline      : Card Detection Interrupt Handler
* Function Name: r_sdhi_dev_cdeti_isr
* Description  : Call a R_SDHI_IntHandler0() function.
* Arguments    : None
* Return Value : None
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
R_BSP_PRAGMA_STATIC_INTERRUPT(r_sdhi_dev_cdeti_isr, VECT(SDHI, CDETI))
R_BSP_ATTRIB_STATIC_INTERRUPT void r_sdhi_dev_cdeti_isr(void)
{
    void * vect = NULL;
    R_SDHI_IntHandler0(vect);
} /* End of function r_sdhi_dev_cdeti_isr() */

/**********************************************************************************************************************
* Outline      : Card Access Interrupt Handler
* Function Name: r_sdhi_dev_caci_isr
* Description  : Call a R_SDHI_IntHandler0() function.
* Arguments    : None
* Return Value : None
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
R_BSP_PRAGMA_STATIC_INTERRUPT(r_sdhi_dev_caci_isr, VECT(SDHI, CACI))
R_BSP_ATTRIB_STATIC_INTERRUPT void r_sdhi_dev_caci_isr(void)
{
    void * vect = NULL;
    R_SDHI_IntHandler0(vect);
} /* End of function r_sdhi_dev_caci_isr() */

/**********************************************************************************************************************
* Outline      : SDIO Access Interrupt Handler
* Function Name: r_sdhi_dev_caci_isr
* Description  : Call a R_SDHI_IntHandler0() function.
* Arguments    : None
* Return Value : None
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
R_BSP_PRAGMA_INTERRUPT(r_sdhi_dev_sdaci_isr, VECT(SDHI, SDACI))
R_BSP_ATTRIB_INTERRUPT void r_sdhi_dev_sdaci_isr (void)
{
    void * vect = NULL;
    R_SDHI_IntHandler0(vect);
} /* End of function r_sdhi_dev_sdaci_isr() */

/**********************************************************************************************************************
* Outline      : Initialize SDHI
* Function Name: r_sdhi_dev_init
* Description  : Initializes hardware to access SDHI.
* Arguments    : uint32_t           channel              ;   SDHI Channel No.
* Return Value : SDHI_SUCCESS                            ;   Successful operation
*----------------------------------------------------------------------------------------------------------------------
* Notes        :  None
**********************************************************************************************************************/
sdhi_status_t r_sdhi_dev_init(uint32_t channel)
{
    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_LPC_CGC_SWR);
    MSTP_SDHI = 0;              /* SDHI ON */
    if (0 == MSTP_SDHI)
    {
        R_BSP_NOP();    /* Wait for the write completion. */
    }
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_LPC_CGC_SWR);

    return SDHI_SUCCESS;
} /* End of function r_sdhi_dev_init() */

/**********************************************************************************************************************
* Outline      : Finalize SDHI Host IP
* Function Name: r_sdhi_dev_finalize
* Description  : Finalizes to access SDHI.
* Arguments    : uint32_t           channel              ;   SDHI Channel No.
* Return Value : SDHI_SUCCESS                            ;   Successful operation
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t r_sdhi_dev_finalize(uint32_t channel)
{
    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_LPC_CGC_SWR);
    MSTP_SDHI = 1;              /* SDHI OFF */
    if (1 == MSTP_SDHI)
    {
        R_BSP_NOP();    /* Wait for the write completion. */
    }
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_LPC_CGC_SWR);

    return SDHI_SUCCESS;
} /* End of function r_sdhi_dev_finalize() */

/**********************************************************************************************************************
* Outline      : Disable ICU
* Function Name: R_SDHI_DisableIcuInt
* Description  : Disable the Interrupt Request Enable (IENn).
* Arguments    : uint32_t           channel              ;   SDHI Channel No.
*              : uint32_t           select               ;   Interrupt setting
* Return Value : SDHI_SUCCESS                            ;   Successful operation
*              : SDHI_ERR                                ;   Failed operation
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t R_SDHI_DisableIcuInt(uint32_t channel, uint32_t select)
{
#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    sdhi_sdhndl_t * p_hndl = 0;

    if ((uint32_t)((sizeof(g_sdhi_ip_base)) / sizeof(uint32_t)) <= channel)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_DEV, __LINE__);
        return SDHI_ERR;
    }

    p_hndl = SDHI_GET_HNDL(channel);
    if (0 == p_hndl)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_DEV, __LINE__);
        return SDHI_ERR;  /* not initialized */
    }

    if (select & SDHI_HWINT_PARAM)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_DEV, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    if (SDHI_CH0 == channel)    /* SDHI_CH0 */
    {
        /* ---- Disable interrupt request (IENn) of IER. ---- */
        if (select & SDHI_HWINT_BUFFER)
        {
            R_BSP_InterruptRequestDisable(VECT(SDHI,SBFAI));
        }

        if (select & SDHI_HWINT_ACCESS_CD)
        {
            R_BSP_InterruptRequestDisable(VECT(SDHI, CDETI));
            R_BSP_InterruptRequestDisable(VECT(SDHI, CACI));
            R_BSP_InterruptRequestDisable(VECT(SDHI, SDACI));
            IPR(SDHI,CDETI)    = 0;
            IPR(SDHI,CACI)     = 0;
            IPR(SDHI,SDACI)    = 0;
            if (0 == IPR(SDHI,SDACI))
            {
                R_BSP_NOP();    /* Wait for the write completion. */
            }
        }

        if (select & SDHI_HWINT_BUFFER)
        {
            IPR(SDHI,SBFAI)    = 0;
            if (0 == IPR(SDHI,SBFAI))
            {
                R_BSP_NOP();    /* Wait for the write completion. */
            }
        }
    }

    return SDHI_SUCCESS;

} /* End of function R_SDHI_DisableIcuInt() */

/**********************************************************************************************************************
* Outline      : Enable ICU
* Function Name: R_SDHI_EnableIcuInt
* Description  : Enable the Interrupt Request Enable (IENn).
* Arguments    : uint32_t           channel              ;   SDHI Channel No.
*              : uint32_t           select               ;   Interrupt setting
* Return Value : SDHI_SUCCESS                            ;   Successful operation
*              : SDHI_ERR                                ;   Failed operation
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t R_SDHI_EnableIcuInt(uint32_t channel, uint32_t select)
{
#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    sdhi_sdhndl_t * p_hndl = 0;

    if ((uint32_t)((sizeof(g_sdhi_ip_base)) / sizeof(uint32_t)) <= channel)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_DEV, __LINE__);
        return SDHI_ERR;
    }

    p_hndl = SDHI_GET_HNDL(channel);
    if (0 == p_hndl)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_DEV, __LINE__);
        return SDHI_ERR;  /* not initialized */
    }

    if (select & SDHI_HWINT_PARAM)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_DEV, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    if (SDHI_CH0 == channel)    /* SDHI_CH0 */
    {
        /* ---- Enable interrupt request (IENn) of IER. ---- */
        if (select & SDHI_HWINT_BUFFER)
        {
            IPR(SDHI,SBFAI)    = SDHI_CFG_CH0_INT_LEVEL_DMADTC;
        }

        if (select & SDHI_HWINT_ACCESS_CD)
        {
            R_BSP_InterruptRequestEnable(VECT(SDHI, CDETI));
            R_BSP_InterruptRequestEnable(VECT(SDHI, CACI));
            R_BSP_InterruptRequestEnable(VECT(SDHI, SDACI));
            IPR(SDHI,CDETI)    = SDHI_CFG_CH0_INT_LEVEL;
            IPR(SDHI,CACI)     = SDHI_CFG_CH0_INT_LEVEL;
            IPR(SDHI,SDACI)    = SDHI_CFG_CH0_INT_LEVEL;
        }

        if (select & SDHI_HWINT_BUFFER)
        {
            R_BSP_InterruptRequestEnable(VECT(SDHI,SBFAI));
            if (1 == IEN(SDHI,SBFAI))
            {
                R_BSP_NOP();    /* Wait for the write completion. */
            }
        }
    }

    return SDHI_SUCCESS;

} /* End of function R_SDHI_EnableIcuInt() */

/**********************************************************************************************************************
* Outline      : CD Terminal layout check
* Function Name: R_SDHI_CDLayout
* Description  : CD Terminal layout check.
* Arguments    : uint32_t           channel              ;   SDHI Channel No.
* Return Value : SDHI_SUCCESS                            ;   layout ACTIVE
*              : SDHI_ERR                                ;   layout no ACTIVE
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t R_SDHI_CDLayout(uint32_t channel)
{
    sdhi_status_t ret = SDHI_ERR;

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if ((uint32_t)((sizeof(g_sdhi_ip_base)) / sizeof(uint32_t)) <= channel)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_DEV, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

#if (SDHI_CFG_CH0_INCLUDED == 1)
    if (SDHI_CH0 == channel)
    {
        if (SDHI_CFG_CH0_CD_ACTIVE)
        {
            ret = SDHI_SUCCESS;
        }
    }
#endif  /* #if (SDHI_CFG_CH0_INCLUDED == 1) */

    return ret;
} /* End of function R_SDHI_CDLayout() */

/**********************************************************************************************************************
* Outline      : WP Terminal layout check
* Function Name: R_SDHI_WPLayout
* Description  : WP Terminal layout check.
* Arguments    : uint32_t           channel              ;   SDHI Channel No.
* Return Value : SDHI_SUCCESS                            ;   layout ACTIVE
*              : SDHI_ERR                                ;   layout no ACTIVE
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t R_SDHI_WPLayout(uint32_t channel)
{
    sdhi_status_t ret = SDHI_ERR;

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if ((uint32_t)((sizeof(g_sdhi_ip_base)) / sizeof(uint32_t)) <= channel)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_DEV, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

#if (SDHI_CFG_CH0_INCLUDED == 1)
    if (SDHI_CH0 == channel)
    {
        if (SDHI_CFG_CH0_WP_ACTIVE)
        {
            ret = SDHI_SUCCESS;
        }
    }
#endif  /* #if (SDHI_CFG_CH0_INCLUDED == 1) */

    return ret;
} /* End of function R_SDHI_WPLayout() */

/**********************************************************************************************************************
* Outline      : Card Access Option Register Init
* Function Name: r_sdhi_dev_sdopt_init
* Description  : Card Access Option Register Init.
* Arguments    : void
* Return Value : uint32_t
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
uint32_t r_sdhi_dev_sdopt_init(void)
{
    return ((SDHI_SDOPT_INIT | SDHI_CFG_SDOPT_TOP) | SDHI_CFG_SDOPT_CTOP);
} /* End of function r_sdhi_dev_sdopt_init() */

/**********************************************************************************************************************
* Outline      : SDHI TERMINAL SPEED check
* Function Name: R_SDHI_GetSpeedType
* Description  : SDHI TERMINAL SPEED check.
* Arguments    : uint32_t           channel              ;   SDHI Channel No.
* Return Value : SDHI_SUCCESS                            ;   High-speed
*              : SDHI_ERR                                ;   Default-speed
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t R_SDHI_GetSpeedType(uint32_t channel)
{
    sdhi_status_t ret = SDHI_ERR;

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if ((uint32_t)((sizeof(g_sdhi_ip_base)) / sizeof(uint32_t)) <= channel)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_DEV, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

#if (SDHI_CFG_CH0_INCLUDED == 1)
    if (SDHI_CH0 == channel)
    {
        ret = SDHI_ERR;
    }
#endif  /* #if (SDHI_CFG_CH0_INCLUDED == 1) */

    return ret;
} /* End of function R_SDHI_GetSpeedType() */

/**********************************************************************************************************************
* Outline      : SDHI CLKSEL BIT check
* Function Name: r_sdhi_check_clksel
* Description  : Check SDHI CLKSEL BIT.
* Arguments    : uint32_t           channel              ;   SDHI Channel No.
* Return Value : SDHI_SUCCESS                            ;   CLKSEL BIT: PCLK division by 1 setting permitted
*              : SDHI_ERR                                ;   CLKSEL BIT: PCLK division by 1 setting prohibited
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t r_sdhi_check_clksel(uint32_t channel)
{
    sdhi_status_t ret = SDHI_ERR;

#if (SDHI_CFG_CH0_INCLUDED == 1)
    if (SDHI_CH0 == channel)
    {
        ret = SDHI_SUCCESS;
    }
#endif  /* #if (SDHI_CFG_CH0_INCLUDED == 1) */

    return ret;
} /* End of function r_sdhi_check_clksel() */

/**********************************************************************************************************************
* Outline      : SDHI_MEM & SDHI_SDIO Interrupt Handler
* Function Name: R_SDHI_IntHandler0
* Description  : Checks the relevant elements (without masked) and call a callback function.
* Arguments    : uint32_t           channel              ;   SDHI Channel No.
* Return Value : None
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
void R_SDHI_IntHandler0(void * vect)
{
    sdhi_sdhndl_t   * p_hndl = 0;
    uint32_t          channel = 0;
    uint32_t          sdsts1 = 0;
    uint32_t          sdsts2 = 0;
    uint32_t          sdiosts = 0;

    channel = 0;
    p_hndl = SDHI_GET_HNDL(channel);

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if (0 == p_hndl)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_DEV, __LINE__);
        return;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    /* SDHI_MEM */
    if ((SDHI_CH0_INT_CDETI == IR(SDHI,CDETI)) || (SDHI_CH0_INT_CACI == IR(SDHI,CACI)))
    {
        sdsts1 = SDHI_INREG(p_hndl, SDHI_SDSTS1);
        sdsts2 = SDHI_INREG(p_hndl, SDHI_SDSTS2);
        if (sdsts1 | sdsts2)
        {
            if (p_hndl->int_callback)
            {
                (*p_hndl->int_callback)(sdsts1, sdsts2);
            }
        }
    }

    /* SDHI_SDIO */
    if (SDHI_CH0_INT_SDACI == IR(SDHI,SDACI))
    {
        sdiosts = SDHI_INREG(p_hndl, SDHI_SDIOSTS);
        if (sdiosts)
        {
            if (p_hndl->int_io_callback)
            {
                (*p_hndl->int_io_callback)(sdiosts);
            }
        }
    }

    return;

} /* End of function R_SDHI_IntHandler0() */

/**********************************************************************************************************************
* Outline      : Hardware Lock
* Function Name: r_sdhi_dev_hardware_lock
* Description  : Hardware Lock.
* Arguments    : uint32_t           channel     ; Channel No.
* Return Value : true                           ; Lock was acquired.
*              : false                          ; Lock was not acquired.
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
bool r_sdhi_dev_hardware_lock(uint32_t channel)
{
    return R_BSP_HardwareLock((mcu_lock_t)(BSP_LOCK_SDHI + channel));
} /* End of function r_sdhi_dev_hardware_lock() */

/**********************************************************************************************************************
* Outline      : Hardware Unlock
* Function Name: r_sdhi_dev_hardware_unlock
* Description  : Hardware Unlock.
* Arguments    : uint32_t           channel     ; Channel No.
* Return Value : true                           ; Lock was released.
*              : false                          ; Lock was not released.
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
bool r_sdhi_dev_hardware_unlock(uint32_t channel)
{
    return R_BSP_HardwareUnlock((mcu_lock_t)(BSP_LOCK_SDHI + channel));
} /* End of function r_sdhi_dev_hardware_unlock() */

#endif /* defined(BSP_MCU_RX231) */

/* End of File */
