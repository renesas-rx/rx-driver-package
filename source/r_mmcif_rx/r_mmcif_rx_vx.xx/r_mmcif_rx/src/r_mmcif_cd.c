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
* Copyright (C) 2014 Renesas Electronics Corporation. All rights reserved.    
**********************************************************************************************************************/
/**********************************************************************************************************************
* System Name  : MMC Driver
* File Name    : r_mmcif_cd.c
* Version      : 1.05.00
* Device       : RX64M (LQFP-176)
* Abstract     : API & Sub module
* Tool-Chain   : For RX64M Group
*              :  e2 studio (Version 7.4.0)
* OS           : not use
* H/W Platform : RSK board for RX64M
* Description  : Interface file for MMC API for RX
* Limitation   : None
**********************************************************************************************************************/
/**********************************************************************************************************************
* History      : DD.MM.YYYY Version Description
*              : 03.09.2014 1.00    First Release
*              : 20.05.2019 1.05    Added support for GNUC and ICCRX.
*                                   Fixed coding style.
**********************************************************************************************************************/

/**********************************************************************************************************************
Includes <System Includes> , "Project Includes"
**********************************************************************************************************************/
/* Public interface header file for this package. */
#include "r_mmcif_rx_if.h"
/* Private header file for this package. */
#include "./src/r_mmcif_rx_private.h"


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
* Outline      : Set Card Detect Interrupt
* Function Name: R_MMCIF_Cd_Int
* Description  : Sets card detection interrupt.
* Arguments    : uint32_t           channel                 ;   MMC Channel No.
*              : int32_t            enable                  ;   Card detection interrupt mode
*              :                                            ;       MMC_CD_INT_ENABLEÅFEnables interrupt.
*              :                                            ;       MMC_CD_INT_DISABLEÅFDisables interrupt.
*              : mmc_status_t  (*callback)(int32_t)         ;   Interrupt callback function
* Return Value : MMC_SUCCESS                                ;   Successful operation
*              : MMC_ERR                                    ;   Failed operation
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
mmc_status_t R_MMCIF_Cd_Int(uint32_t channel, int32_t enable, mmc_status_t (*callback)(int32_t))
{
    uint32_t           ul_tmp = 0;
    mmc_mmchndl_t     *p_hndl = 0;

    /* Check channel. */
#if (BSP_CFG_PARAM_CHECKING_ENABLE)
    if ((uint32_t)(sizeof(MMC_CFG_IP_BASE) / sizeof(uint32_t)) <= channel)
    {
        R_MMCIF_Log_Func(MMC_DEBUG_ERR_ID, (uint32_t)MMC_CD, __LINE__);
        return MMC_ERR;
    }
#endif /* BSP_CFG_PARAM_CHECKING_ENABLE */

    p_hndl = MMC_GET_HNDL(channel);

#if (BSP_CFG_PARAM_CHECKING_ENABLE)
    if (0 == p_hndl)
    {
        R_MMCIF_Log_Func(MMC_DEBUG_ERR_ID, (uint32_t)MMC_CD, __LINE__);
        return MMC_ERR;  /* Not initialized */
    }
    if ((MMC_CD_INT_ENABLE != enable) && (MMC_CD_INT_DISABLE != enable))
    {
        R_MMCIF_Log_Func(MMC_DEBUG_ERR_ID, (uint32_t)MMC_CD, __LINE__);
        return MMC_ERR;  /* Parameter error */
    }
#endif /* BSP_CFG_PARAM_CHECKING_ENABLE */

    /* Check the interrupt setting. */
    if (0 != (p_hndl->int_det_mask & MMC_CEDETECT_MASK_DET))
    {
        r_mmcif_dev_disable_int(channel);
        ul_tmp = MMC_INREG(p_hndl, MMC_CEDETECT);
        ul_tmp &= (uint32_t)~MMC_CEDETECT_DET;
        MMC_OUTREG(p_hndl, MMC_CEDETECT, ul_tmp); /* Clear the insertion and removal bits. */
        if (ul_tmp == MMC_INREG(p_hndl, MMC_CEDETECT))
        {
            R_BSP_NOP();  /* Wait for the write completion. */
        }
        r_mmcif_dev_enable_int(channel);
    }

    if (MMC_SUCCESS == r_mmcif_dev_cd_layout(channel))
    {
        if (MMC_CD_INT_ENABLE == enable)
        {
            r_mmcif_set_det_mask(channel, MMC_CEDETECT_MASK_DET);
        }
        else    /* case MMC_CD_INT_DISABLE */
        {
            r_mmcif_clear_det_mask(channel, MMC_CEDETECT_MASK_DET);
        }
    }

    /* ---- Register the callback function. ---- */
    p_hndl->int_cd_callback = callback;

    return MMC_SUCCESS;
}


/**********************************************************************************************************************
* Outline      : Check Card Insertion
* Function Name: R_MMCIF_Get_CardDetection
* Description  : Checks card insertion.
* Arguments    : uint32_t           channel             ;   MMC Channel No.
* Return Value : MMC_SUCCESS                            ;   A card is inserted.
*              : MMC_ERR                                ;   A card is not inserted.
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
mmc_status_t R_MMCIF_Get_CardDetection(uint32_t channel)
{
#if (BSP_CFG_PARAM_CHECKING_ENABLE)
    mmc_mmchndl_t     *p_hndl = 0;

    /* Check channel. */
    if ((uint32_t)(sizeof(MMC_CFG_IP_BASE) / sizeof(uint32_t)) <= channel)
    {
        R_MMCIF_Log_Func(MMC_DEBUG_ERR_ID, (uint32_t)MMC_CD, __LINE__);
        return MMC_ERR;
    }

    p_hndl = MMC_GET_HNDL(channel);
    if (0 == p_hndl)
    {
        R_MMCIF_Log_Func(MMC_DEBUG_ERR_ID, (uint32_t)MMC_CD, __LINE__);
        return MMC_ERR;  /* Not initialized */
    }
#endif /* BSP_CFG_PARAM_CHECKING_ENABLE */

    return r_mmcif_Get_CardDetection(channel);
}


/**********************************************************************************************************************
* Outline      : Check Card Insertion
* Function Name: r_mmcif_Get_CardDetection
* Description  : Checks card insertion.
* Arguments    : uint32_t           channel             ;   MMC Channel No.
* Return Value : MMC_SUCCESS                            ;   A card is inserted.
*              : MMC_ERR                                ;   A card is not inserted.
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
mmc_status_t r_mmcif_Get_CardDetection(uint32_t channel)
{
    mmc_mmchndl_t     *p_hndl = 0;
    uint32_t           reg = 1;

    p_hndl = MMC_GET_HNDL(channel);

    reg = MMC_INREG(p_hndl, MMC_CEDETECT);

    if (MMC_SUCCESS == r_mmcif_dev_cd_layout(channel))
    {
        reg &= MMC_CEDETECT_MASK_CDSIG;     /* Check CD level. */
    }
    else
    {
        reg = 0;    /* In eMMC, the CD pin check is unnecessary. */
    }
    if (0 == reg)
    {
        return MMC_SUCCESS;  /* Inserted */
    }

    R_MMCIF_Log_Func(MMC_DEBUG_ERR_ID, (uint32_t)MMC_CD, __LINE__);
    return MMC_ERR;     /* No card */
}


/**********************************************************************************************************************
* Outline      : Set CE_DETECT interrupt mask
* Function Name: r_mmcif_det_mask
* Description  : Sets int_det_mask depend on the mask bits value.
* Arguments    : uint32_t           channel             ;    MMC Channel No.
*              : uint32_t           mask                ;    Mask bits value
* Return Value : MMC_SUCCESS                            ;    Successful operation
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
mmc_status_t r_mmcif_set_det_mask(uint32_t channel, uint32_t mask)
{
    mmc_mmchndl_t    *p_hndl = 0;
    uint32_t          ul_tmp = 0;

    p_hndl = MMC_GET_HNDL(channel);

    r_mmcif_dev_disable_int(channel);

    /* ---- Set int_det_mask. ---- */
    p_hndl->int_det_mask |= mask;

    /* ---- Set hardware mask. ---- */
    ul_tmp = MMC_INREG(p_hndl, MMC_CEDETECT);
    ul_tmp |= (p_hndl->int_det_mask | MMC_CEDETECT_DET);
                                            /* NOTE: If write CEDETECT register, must set CDFALL bit and CDRISE bit. */
    MMC_OUTREG(p_hndl, MMC_CEDETECT, ul_tmp);
    if (MMC_INREG(p_hndl, MMC_CEDETECT) == ul_tmp)
    {
        R_BSP_NOP();  /* Wait for the write completion. */
    }

    r_mmcif_dev_enable_int(channel);

    return MMC_SUCCESS;
}


/**********************************************************************************************************************
* Outline      : Clear CE_DETECT interrupt mask
* Function Name: r_mmcif_clear_det_mask
* Description  : Clears int_det_mask depend on the mask bits value.
* Arguments    : uint32_t           channel             ;    MMC Channel No.
*              : uint32_t           mask                ;    Mask bits value
* Return Value : MMC_SUCCESS                            ;    Successful operation
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
mmc_status_t r_mmcif_clear_det_mask(uint32_t channel, uint32_t mask)
{
    mmc_mmchndl_t    *p_hndl = 0;
    uint32_t          ul_tmp = 0;

    p_hndl = MMC_GET_HNDL(channel);

    r_mmcif_dev_disable_int(channel);

    /* ---- Clear int_det_mask. ---- */
    p_hndl->int_det_mask &= ~mask;

    /* ---- Clear hardware mask. ---- */
    ul_tmp = MMC_INREG(p_hndl, MMC_CEDETECT);
    ul_tmp &= ~MMC_CEDETECT_MASK_DET;
    ul_tmp |= (p_hndl->int_det_mask | MMC_CEDETECT_DET);
                                            /* NOTE: If write CEDETECT register, must set CDFALL bit and CDRISE bit. */
    MMC_OUTREG(p_hndl, MMC_CEDETECT, ul_tmp);
    if (MMC_INREG(p_hndl, MMC_CEDETECT) == ul_tmp)
    {
        R_BSP_NOP();  /* Wait for the write completion. */
    }

    r_mmcif_dev_enable_int(channel);

    return MMC_SUCCESS;
}


/**********************************************************************************************************************
* Outline      : Get CE_DETECT interrupt elements
* Function Name: r_mmcif_get_det_int
* Description  : Gets CE_DETECT bits examine enabled elements hear after, 
*              : clears CE_DETECT bits save those bits to int_det.
* Arguments    : uint32_t           channel             ;   MMC Channel No.
* Return Value : MMC_SUCCESS                            ;   Any interrupt occured
*              : MMC_ERR                                ;   No interrupt occured
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
mmc_status_t r_mmcif_get_det_int(uint32_t channel)
{
    mmc_mmchndl_t          *p_hndl = 0;
    uint32_t                reg = 0;
    uint32_t                flag = 0;

    p_hndl = MMC_GET_HNDL(channel);

    /* Get CE_DETECT bits. */
    reg = MMC_INREG(p_hndl, MMC_CEDETECT);
    flag =  (reg & (p_hndl->int_det_mask << 8u));
    
    /* Clear CE_DETECT bits. */
    reg &= ~MMC_CEDETECT_DET;
    MMC_OUTREG(p_hndl, MMC_CEDETECT, reg);
    if (MMC_INREG(p_hndl, MMC_CEDETECT) == reg)
    {
        R_BSP_NOP();  /* Wait for the write completion. */
    }

    /* Save enabled elements. */
    p_hndl->int_det |= flag;
    if (flag)
    {
        return MMC_SUCCESS;  /* Any interrupt occurred. */
    }

    R_MMCIF_Log_Func(MMC_DEBUG_ERR_ID, (uint32_t)MMC_CD, __LINE__);
    return MMC_ERR;     /* No interrupt occurred. */
}


/* End of File */
