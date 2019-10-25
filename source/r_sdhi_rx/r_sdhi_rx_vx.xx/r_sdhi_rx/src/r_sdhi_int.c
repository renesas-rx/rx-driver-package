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
* File Name    : r_sdhi_int.c
* Version      : 2.04
* Device       : RX
* Abstract     : API & Sub module
* Tool-Chain   : For RX e2_studio
* OS           : not use
* H/W Platform : RSK board for RX
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
#include "r_sdhi_rx_if.h"
#include ".\src\r_sdhi_rx_private.h"

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
* Outline      : Set SDIMSK1 and SDIMSK2 Interrupt Mask
* Function Name: R_SDHI_SetIntMask
* Description  : Sets the sdimsk1 and sdimsk2 according to the specified mask1 and mask2.
* Arguments    : uint32_t           channel              ;   SDHI Channel No.
*              : uint32_t           mask1                ;   SDHI_SDIMSK1 bits value
*              : uint32_t           mask2                ;   SDHI_SDIMSK2 bits value
* Return Value : SDHI_SUCCESS                            ;   Successful operation
*              : SDHI_ERR                                ;   Failed operation
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t R_SDHI_SetIntMask(uint32_t channel, uint32_t mask1, uint32_t mask2)
{
    sdhi_sdhndl_t * p_hndl = 0;
    uint32_t mask1_reg = 0;
    uint32_t mask2_reg = 0;

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if ((uint32_t)((sizeof(g_sdhi_ip_base)) / sizeof(uint32_t)) <= channel)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_INT, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    p_hndl = SDHI_GET_HNDL(channel);

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if (0 == p_hndl)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_INT, __LINE__);
        return SDHI_ERR;  /* not initialized */
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    /* ---- Set the sdimsk1 and sdimsk2 according to the specified mask1 and mask2. ---- */
    mask1_reg = (SDHI_INREG(p_hndl, SDHI_SDIMSK1) & (~mask1));
    mask2_reg = ((SDHI_INREG(p_hndl, SDHI_SDIMSK2) & (~mask2)) | SDHI_SDIMSK2_BIT11);

    /* ---- Set the SDIMSK1 and SDIMSK2 registers. ---- */
    SDHI_OUTREG(p_hndl, SDHI_SDIMSK1, mask1_reg);
    SDHI_OUTREG(p_hndl, SDHI_SDIMSK2, mask2_reg);
    if (SDHI_INREG(p_hndl, SDHI_SDIMSK2) == mask2_reg)
    {
        R_BSP_NOP();    /* Wait for the write completion. */
    }

    return SDHI_SUCCESS;
} /* End of function R_SDHI_SetIntMask() */

/**********************************************************************************************************************
* Outline      : Clear SDIMSK1 and SDIMSK2 Interrupt Mask
* Function Name: R_SDHI_ClearIntMask
* Description  : Clears the sdimsk1 and sdimsk2 according to the specified mask1 and mask2.
* Arguments    : uint32_t           channel              ;   SDHI Channel No.
*              : uint32_t           mask1                ;   SDHI_SDIMSK1 bits value
*              : uint32_t           mask2                ;   SDHI_SDIMSK2 bits value
* Return Value : SDHI_SUCCESS                            ;   Successful operation
*              : SDHI_ERR                                ;   Failed operation
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t R_SDHI_ClearIntMask(uint32_t channel, uint32_t mask1, uint32_t mask2)
{
    sdhi_sdhndl_t * p_hndl = 0;
    uint32_t mask1_reg = 0;
    uint32_t mask2_reg = 0;

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if ((uint32_t)((sizeof(g_sdhi_ip_base)) / sizeof(uint32_t)) <= channel)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_INT, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    p_hndl = SDHI_GET_HNDL(channel);

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if (0 == p_hndl)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_INT, __LINE__);
        return SDHI_ERR;  /* not initialized */
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    /* ---- Clear the sdimsk1 and sdimsk2 according to the specified mask1 and mask2. ---- */
    mask1_reg = (SDHI_INREG(p_hndl, SDHI_SDIMSK1) | mask1);
    mask2_reg = ((SDHI_INREG(p_hndl, SDHI_SDIMSK2) | mask2) | SDHI_SDIMSK2_BIT11);

    /* ---- Clear the SDIMSK1 and SDIMSK2 registers. ---- */
    SDHI_OUTREG(p_hndl, SDHI_SDIMSK1, mask1_reg);
    SDHI_OUTREG(p_hndl, SDHI_SDIMSK2, mask2_reg);
    if (SDHI_INREG(p_hndl, SDHI_SDIMSK2) == mask2_reg)
    {
        R_BSP_NOP();    /* Wait for the write completion. */
    }

    return SDHI_SUCCESS;
} /* End of function R_SDHI_ClearIntMask() */

/**********************************************************************************************************************
* Outline      : Clear SDSTS1 and SDSTS2
* Function Name: R_SDHI_ClearSdstsReg
* Description  : Clear the SDSTS1 and SDSTS2 registers according to the specified clear_sdsts1 and clear_sdsts2.
* Arguments    : uint32_t           channel              ;   SDHI Channel No.
*              : uint32_t           clear_sdsts1         ;   sdsts1 clear bits value
*              : uint32_t           clear_sdsts2         ;   sdsts2 clear bits value
* Return Value : SDHI_SUCCESS                            ;   Successful operation
*              : SDHI_ERR                                ;   Failed operation
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t R_SDHI_ClearSdstsReg(uint32_t channel, uint32_t clear_sdsts1, uint32_t clear_sdsts2)
{
    sdhi_sdhndl_t   * p_hndl = 0;
    uint32_t            sdsts1 = 0;
    uint32_t            sdsts2 = 0;

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if ((uint32_t)((sizeof(g_sdhi_ip_base)) / sizeof(uint32_t)) <= channel)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_INT, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    p_hndl = SDHI_GET_HNDL(channel);

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if (0 == p_hndl)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_INT, __LINE__);
        return SDHI_ERR;  /* not initialized */
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    /* ---- Clear the SDSTS1 and SDSTS2 according to the specified clear_sdsts1 and clear_sdsts2. ---- */
    sdsts1 = (SDHI_INREG(p_hndl, SDHI_SDSTS1) & (~clear_sdsts1));
    sdsts2 = ((SDHI_INREG(p_hndl, SDHI_SDSTS2) & (~clear_sdsts2)) | SDHI_SDIMSK2_BIT11);

    /* Clear the SDSTS1 and SDSTS2 registers. */
    SDHI_OUTREG(p_hndl, SDHI_SDSTS1, sdsts1);
    SDHI_OUTREG(p_hndl, SDHI_SDSTS2, sdsts2);

    if (SDHI_INREG(p_hndl, SDHI_SDSTS2) == sdsts2)
    {
        R_BSP_NOP();    /* Wait for the write completion. */
    }

    return SDHI_SUCCESS;
} /* End of function R_SDHI_ClearSdstsReg() */

/**********************************************************************************************************************
* Outline      : Set SDHI_SDIOSTS Interrupt Mask
* Function Name: R_SDHI_SetSdioIntMask
* Description  : Sets the sdioimsk according to the specified mask and sets the SDIOIMSK register.
* Arguments    : uint32_t       channel                  ; SDHI Channel No.
*              : uint32_t       mask                     ; SDHI_SDIOIMSK bits value
* Return Value : SDHI_SUCCESS                            ; Successful operation
*              : SDHI_ERR                                ;   Failed operation
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t R_SDHI_SetSdioIntMask(uint32_t channel, uint32_t mask)
{
    sdhi_sdhndl_t       * p_hndl = 0;
    uint32_t mask_reg = 0;

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if ((uint32_t)((sizeof(g_sdhi_ip_base)) / sizeof(uint32_t)) <= channel)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_INT, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    p_hndl = SDHI_GET_HNDL(channel);

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if (0 == p_hndl)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_INT, __LINE__);
        return SDHI_ERR;  /* not initialized */
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    /* ---- Set the sdioimsk according to the specified mask. ---- */
    mask_reg = ((SDHI_INREG(p_hndl, SDHI_SDIOIMSK) & (~mask)) | SDHI_SDIOSTS_INIT);

    /* ---- Set the SDIOIMSK register. ---- */
    SDHI_OUTREG(p_hndl, SDHI_SDIOIMSK, mask_reg);
    if (SDHI_INREG(p_hndl, SDHI_SDIOIMSK) == mask_reg)
    {
        R_BSP_NOP();    /* Wait for the write completion. */
    }

    return SDHI_SUCCESS;

} /* End of function R_SDHI_SetSdioIntMask() */

/**********************************************************************************************************************
* Outline      : Clear SDHI_SDIOIMSK interrupt Mask
* Function Name: R_SDHI_ClearSdioIntMask 
* Description  : Clears the sdioimsk according to the specified mask and clears the SDIOIMSK register.
* Arguments    : uint32_t       channel                  ; SDHI Channel No.
*              : uint32_t       mask                     ; SDHI_SDIOIMSK bits value
* Return Value : SDHI_SUCCESS                            ; Successful operation
*              : SDHI_ERR                                ;   Failed operation
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t R_SDHI_ClearSdioIntMask(uint32_t channel, uint32_t mask)
{
    sdhi_sdhndl_t       * p_hndl = 0;
    uint32_t mask_reg = 0;

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if ((uint32_t)((sizeof(g_sdhi_ip_base)) / sizeof(uint32_t)) <= channel)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_INT, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    p_hndl = SDHI_GET_HNDL(channel);

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if (0 == p_hndl)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_INT, __LINE__);
        return SDHI_ERR;  /* not initialized */
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    /* ---- Clear the sdioimsk. ---- */
    mask_reg = ((SDHI_INREG(p_hndl, SDHI_SDIOIMSK) | mask) | SDHI_SDIOSTS_INIT);

    /* ---- Clear the SDIOIMSK register. ---- */
    SDHI_OUTREG(p_hndl, SDHI_SDIOIMSK, mask_reg);
    if (SDHI_INREG(p_hndl, SDHI_SDIOIMSK) == mask_reg)
    {
        R_BSP_NOP();    /* Wait for the write completion. */
    }

    return SDHI_SUCCESS;

} /* End of function R_SDHI_ClearSdioIntMask() */

/**********************************************************************************************************************
* Outline      : Clear SDIOSTS
* Function Name: R_SDHI_ClearSdiostsReg
* Description  : Clear the SDIOSTS register according to the specified clear value.
* Arguments    : uint32_t           channel              ;   SDHI Channel No.
*              : uint32_t           clear                ;   Clear bits value
* Return Value : SDHI_SUCCESS                            ;   Successful operation
*              : SDHI_ERR                                ;   Failed operation
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t R_SDHI_ClearSdiostsReg(uint32_t channel, uint32_t clear)
{
    sdhi_sdhndl_t   * p_hndl = 0;
    uint32_t          sdiosts = 0;

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if ((uint32_t)((sizeof(g_sdhi_ip_base)) / sizeof(uint32_t)) <= channel)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_INT, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    p_hndl = SDHI_GET_HNDL(channel);

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if (0 == p_hndl)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_INT, __LINE__);
        return SDHI_ERR;  /* not initialized */
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    /* ---- Clear the SDIOSTS according to the specified clear value. ---- */
    sdiosts = ((SDHI_INREG(p_hndl, SDHI_SDIOSTS) & (~clear)) | SDHI_SDIOSTS_INIT);

    /* Clear the SDIOSTS register. */
    SDHI_OUTREG(p_hndl, SDHI_SDIOSTS, sdiosts);
    if (SDHI_INREG(p_hndl, SDHI_SDIOSTS) == sdiosts)
    {
        R_BSP_NOP();    /* Wait for the write completion. */
    }

    return SDHI_SUCCESS;
} /* End of function R_SDHI_ClearSdiostsReg() */

/**********************************************************************************************************************
* Outline      : Register SDSTS1 or SDSTS2 Interrupt Callback Function
* Function Name: R_SDHI_IntCallback
* Description  : Registers the callback function of SDSTS1 or SDSTS2 interrupt.
*              : If the SDSTS1 or SDSTS2 interrupt are occurred, calls callback function.
* Arguments    : uint32_t           channel              ;   SDHI Channel No.
*              : sdhi_status_t      (* callback)(uint32_t, uint32_t)    ;   Callback function
* Return Value : SDHI_SUCCESS                            ;   Generate interrupt
*              : SDHI_ERR                                ;   Not generated interrupt
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t R_SDHI_IntCallback(uint32_t channel, sdhi_status_t (* callback)(uint32_t, uint32_t))
{
    sdhi_sdhndl_t * p_hndl = 0;

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if ((uint32_t)((sizeof(g_sdhi_ip_base)) / sizeof(uint32_t)) <= channel)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_INT, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    p_hndl = SDHI_GET_HNDL(channel);

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if (0 == p_hndl)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_INT, __LINE__);
        return SDHI_ERR;  /* Not initialized */
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    p_hndl->int_callback = callback;

    return SDHI_SUCCESS;
} /* End of function R_SDHI_IntCallback() */

/**********************************************************************************************************************
* Outline      : SD Buffer Access Interrupt Callback Function
* Function Name: R_SDHI_IntSDBuffCallback
* Description  : Registers the callback function of SD Buffer Access interrupt.
*              : If the SD Buffer Access interrupt are occurred, calls callback function.
* Arguments    : uint32_t           channel              ;   SDHI Channel No.
*              : sdhi_status_t  (* callback)(void *)     ;   Callback function
* Return Value : SDHI_SUCCESS                            ;   Generate interrupt
*              : SDHI_ERR                                ;   Not generated interrupt
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t R_SDHI_IntSDBuffCallback(uint32_t channel, sdhi_status_t (* callback)(void *))
{
    sdhi_sdhndl_t * p_hndl = 0;

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if ((uint32_t)((sizeof(g_sdhi_ip_base)) / sizeof(uint32_t)) <= channel)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_INT, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    p_hndl = SDHI_GET_HNDL(channel);

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if (0 == p_hndl)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_INT, __LINE__);
        return SDHI_ERR;  /* Not initialized */
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    p_hndl->int_sdbuff_callback = callback;

    return SDHI_SUCCESS;
} /* End of function R_SDHI_IntSDBuffCallback() */

/**********************************************************************************************************************
* Outline      : Register SDHI_SDIOSTS Interrupt Callback Function
* Function Name: R_SDHI_IntSdioCallback
* Description  : Registers the callback function of SDIOSTS interrupt.
*              : If the SDIOSTS interrupt is occurred, call callback function.
* Arguments    : uint32_t       channel                 ; SDHI Channel No.
*              : int32_t        (* callback)(uint32_t)  ; callback function
* Return Value : SDHI_SUCCESS                           ; Successful operation
*              : SDHI_ERR                               ; Failed operation
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t R_SDHI_IntSdioCallback(uint32_t channel, sdhi_status_t (* callback)(uint32_t))
{
    sdhi_sdhndl_t       * p_hndl = 0;

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if ((uint32_t)((sizeof(g_sdhi_ip_base)) / sizeof(uint32_t)) <= channel)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_INT, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    p_hndl = SDHI_GET_HNDL(channel);

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if (0 == p_hndl)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_INT, __LINE__);
        return SDHI_ERR;  /* not initialized */
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    p_hndl->int_io_callback = callback;

    return SDHI_SUCCESS;

} /* End of function R_SDHI_IntSdioCallback() */

/* End of File */
