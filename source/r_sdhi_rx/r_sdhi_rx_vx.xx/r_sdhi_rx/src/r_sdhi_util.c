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
* File Name    : r_sdhi_util.c
* Version      : 2.05
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
*              : 30.07.2019 2.05    Added  WAIT LOOP 
**********************************************************************************************************************/

/**********************************************************************************************************************
Includes <System Includes> , "Project Includes"
**********************************************************************************************************************/
#include "r_sdhi_rx_if.h"
#include ".\src\r_sdhi_rx_private.h"

#ifdef SDHI_CFG_LONGQ_ENABLE                                /* Uses FIT LONGQ module                */
#include "r_longq_if.h"
#endif /* SDHI_CFG_LONGQ_ENABLE */

/**********************************************************************************************************************
Macro definitions
**********************************************************************************************************************/

/**********************************************************************************************************************
Typedef definitions
**********************************************************************************************************************/

/**********************************************************************************************************************
Private global variables and functions
**********************************************************************************************************************/
#ifdef SDHI_CFG_LONGQ_ENABLE                                /* Uses FIT LONGQ module                */
static longq_hdl_t     p_sdhi_long_que = 0;                 /* LONGQ module                         */
#endif /* SDHI_CFG_LONGQ_ENABLE */

/**********************************************************************************************************************
* Outline      : Control SD Clock
* Function Name: R_SDHI_SetClock
* Description  : Supplies or halts the SD clock.
* Arguments    : uint32_t           channel              ;   SDHI Channel No.
*              : uint32_t           div                  ;   PCLK Clock divide ratio for SDHI clock
*              : int32_t            enable               ;   SDHI_CLOCK_ENABLE or SDHI_CLOCK_DISABLE
* Return Value : SDHI_SUCCESS                            ;   Successful operation
*              : SDHI_ERR                                ;   Failed operation
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t R_SDHI_SetClock(uint32_t channel, uint32_t div, int32_t enable)
{
    sdhi_sdhndl_t   * p_hndl = 0;
    uint32_t            div_en = 0;
    int32_t             i = 0;

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if ((uint32_t)((sizeof(g_sdhi_ip_base)) / sizeof(uint32_t)) <= channel)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_UTIL, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    p_hndl = SDHI_GET_HNDL(channel);

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if (0 == p_hndl)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_UTIL, __LINE__);
        return SDHI_ERR;  /* not initialized */
    }

    if ((SDHI_CLOCK_ENABLE != enable) && (SDHI_CLOCK_DISABLE != enable))
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_UTIL, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    if ((SDHI_CLOCK_ENABLE == enable) && (r_sdhi_check_clksel(channel) == SDHI_ERR))
    {
        if (div >= SDHI_DIV_1)
        {
            SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_UTIL, __LINE__);
            return SDHI_ERR;
        }
    }
    /* WAIT_LOOP */
    for (i = 0; i < SDHI_SCLKDIVEN_LOOP_COUNT; i++)
    {
#ifdef USE_INFO2_CBSY
        if ((SDHI_INREG(p_hndl, SDHI_SDSTS2) & SDHI_SDIMSK2_CBSY) == 0)
        {
            break;
        }
#else
        if (SDHI_INREG(p_hndl, SDHI_SDSTS2) & SDHI_SDIMSK2_SCLKDIVEN)
        {
            break;
        }
#endif
    }

    if (SDHI_SCLKDIVEN_LOOP_COUNT == i)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_UTIL, __LINE__);
        SDHI_OUTREG(p_hndl, SDHI_SDCLKCR, 0);       /* Halt */
        if (0 == SDHI_INREG(p_hndl, SDHI_SDCLKCR))
        {
            R_BSP_NOP();    /* Wait for the write completion. */
        }
        return SDHI_ERR;
    }

    if (SDHI_CLOCK_ENABLE == enable)
    {
        div_en = SDHI_INREG(p_hndl, SDHI_SDCLKCR);
        div_en = div_en & (~SDHI_CLOCK_MASK_SCLKEN);

        SDHI_OUTREG(p_hndl, SDHI_SDCLKCR, (uint32_t)div_en);
        if (SDHI_CLOCK_MASK_SCLKEN != SDHI_INREG(p_hndl, SDHI_SDCLKCR))
        {
            R_BSP_NOP();    /* Wait for the SDHI_CLOCK_MASK_SCLKEN Clear. */
        }

        if ((div == SDHI_DIV_1) || (div == SDHI_DIV_2))
        {
            /* Do nothing. */
        }
        else if (div >= SDHI_DIV_1)
        {
            div = SDHI_DIV_1;
        }
        else if (div >= SDHI_DIV_512)
        {
            div = SDHI_DIV_512;
        }
        else if (div >= SDHI_DIV_256)
        {
            div = SDHI_DIV_256;
        }
        else if (div >= SDHI_DIV_128)
        {
            div = SDHI_DIV_128;
        }
        else if (div >= SDHI_DIV_64)
        {
            div = SDHI_DIV_64;
        }
        else if (div >= SDHI_DIV_32)
        {
            div = SDHI_DIV_32;
        }
        else if (div >= SDHI_DIV_16)
        {
            div = SDHI_DIV_16;
        }
        else if (div >= SDHI_DIV_8)
        {
            div = SDHI_DIV_8;
        }
        else if (div >= SDHI_DIV_4)
        {
            div = SDHI_DIV_4;
        }
        else
        {
            /* Do nothing */
        }
        
        SDHI_OUTREG(p_hndl, SDHI_SDCLKCR, (uint32_t)div);
        if (SDHI_CLOCK_MASK_SCLKEN != SDHI_INREG(p_hndl, SDHI_SDCLKCR))
        {
            R_BSP_NOP();    /* Wait for the SDHI_CLOCK_MASK_SCLKEN Clear. */
        }

        SDHI_OUTREG(p_hndl, SDHI_SDCLKCR, (uint32_t)(div | SDHI_CLOCK_MASK_SCLKEN));
        if (SDHI_CLOCK_MASK_SCLKEN == SDHI_INREG(p_hndl, SDHI_SDCLKCR))
        {
            R_BSP_NOP();    /* Wait for the write completion. */
        }
    }
    else
    {
        SDHI_OUTREG(p_hndl, SDHI_SDCLKCR, 0);       /* Halt */
        if (0 == SDHI_INREG(p_hndl, SDHI_SDCLKCR))
        {
            R_BSP_NOP();    /* Wait for the write completion. */
        }
    }

    return SDHI_SUCCESS;
} /* End of function R_SDHI_SetClock() */

/**********************************************************************************************************************
* Outline      : Control SD Bus Width
* Function Name: R_SDHI_SetBus
* Description  : Changes the SD bus width.
* Arguments    : uint32_t           channel              ;   SDHI Channel No.
*              : int32_t            width                ;   SDHI_PORT_1BIT or SDHI_PORT_4BIT
* Return Value : SDHI_SUCCESS                            ;   Successful operation
*              : SDHI_ERR                                ;   Failed operation
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t R_SDHI_SetBus(uint32_t channel, int32_t width)
{
    sdhi_sdhndl_t    * p_hndl = 0;
    uint32_t              reg = 0;

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if ((uint32_t)((sizeof(g_sdhi_ip_base)) / sizeof(uint32_t)) <= channel)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_UTIL, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    p_hndl = SDHI_GET_HNDL(channel);

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if (0 == p_hndl)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_UTIL, __LINE__);
        return SDHI_ERR;  /* not initialized */
    }

    if ((SDHI_PORT_1BIT != width) && (SDHI_PORT_4BIT != width))
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_UTIL, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    /* ==== Change the SD bus width. ==== */
    if (SDHI_PORT_1BIT == width) /* 1bit */
    {
        reg = SDHI_INREG(p_hndl, SDHI_SDOPT);
        reg = (reg | SDHI_SDOPT_WIDTH);
        SDHI_OUTREG(p_hndl, SDHI_SDOPT, reg);
        if (SDHI_INREG(p_hndl, SDHI_SDOPT) == reg)
        {
            R_BSP_NOP();    /* Wait for the write completion. */
        }
    }
    else    /* 4bits */
    {
        reg = SDHI_INREG(p_hndl, SDHI_SDOPT);
        reg = (reg & (~SDHI_SDOPT_WIDTH));
        SDHI_OUTREG(p_hndl, SDHI_SDOPT, reg);
        if (SDHI_INREG(p_hndl, SDHI_SDOPT) == reg)
        {
            R_BSP_NOP();    /* Wait for the write completion. */
        }
    }

    return SDHI_SUCCESS;
} /* End of function R_SDHI_SetBus() */

/**********************************************************************************************************************
* Outline      : Check Hardware Write Protect
* Function Name: R_SDHI_GetWP
* Description  : Checks the hardware write protect using the SDHI register.
* Arguments    : uint32_t           channel              ;   SDHI Channel No.
*              : uint32_t         * p_wp                 ;   SDHI_SDSTS1 WP bit
* Return Value : SDHI_SUCCESS                            ;   Successful operation
*              : SDHI_ERR                                ;   Failed operation
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t R_SDHI_GetWP(uint32_t channel, uint32_t * p_wp)
{
    sdhi_sdhndl_t   * p_hndl = 0;

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if ((uint32_t)((sizeof(g_sdhi_ip_base)) / sizeof(uint32_t)) <= channel)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_UTIL, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    p_hndl = SDHI_GET_HNDL(channel);

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if (0 == p_hndl)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_UTIL, __LINE__);
        return SDHI_ERR;  /* not initialized */
    }

    if (0 == p_wp)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_UTIL, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    /*  Check the SDHI_SDSTS1 WP bit.  */
    *p_wp = ((~SDHI_INREG(p_hndl, SDHI_SDSTS1)) & SDHI_SDSTS1_WP) >> SDHI_BIT_SHIFT_7;

    return SDHI_SUCCESS;
} /* End of function R_SDHI_GetWP() */

/**********************************************************************************************************************
* Outline      : Get Response and Check Response Errors
* Function Name: R_SDHI_GetResp
* Description  : Gets the response value from the RESP register.
*              : After finishing this function, start the data transfer. 
* Arguments    : uint32_t           channel              ;   SDHI Channel No.
*              : sdhi_get_resp_t    * p_resp_reg         ;   Response register
*              :    sdrsp10                              ;   SDRSP10 register
*              :    sdrsp32                              ;   SDRSP32 register
*              :    sdrsp54                              ;   SDRSP54 register
*              :    sdrsp76                              ;   SDRSP76 register
* Return Value : SDHI_SUCCESS                            ;   Successful operation
*              : SDHI_ERR                                ;   Failed operation
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t R_SDHI_GetResp(uint32_t channel, sdhi_get_resp_t * p_resp_reg)
{
    sdhi_sdhndl_t * p_hndl = 0;

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if ((uint32_t)((sizeof(g_sdhi_ip_base)) / sizeof(uint32_t)) <= channel)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_UTIL, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    p_hndl = SDHI_GET_HNDL(channel);

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if (0 == p_hndl)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_UTIL, __LINE__);
        return SDHI_ERR;  /* not initialized */
    }

    if (0 == p_resp_reg)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_UTIL, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    p_resp_reg->sdrsp76 = SDHI_INREG(p_hndl, SDHI_SDRESP76);
    p_resp_reg->sdrsp54 = SDHI_INREG(p_hndl, SDHI_SDRESP54);
    p_resp_reg->sdrsp32 = SDHI_INREG(p_hndl, SDHI_SDRESP32);
    p_resp_reg->sdrsp10 = SDHI_INREG(p_hndl, SDHI_SDRESP10);

    return SDHI_SUCCESS;
} /* End of function R_SDHI_GetResp() */

/**********************************************************************************************************************
* Outline      : Get Buffer Register Address
* Function Name: R_SDHI_GetBuffRegAddress
* Description  : Gets the Buffer Register Address.
* Arguments    : uint32_t           channel              ;   SDHI Channel No.
*              : uint32_t           * p_reg_buff         ;   Buffer Register Address
* Return Value : SDHI_SUCCESS                            ;   Successful operation
*              : SDHI_ERR                                ;   Failed operation
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t R_SDHI_GetBuffRegAddress(uint32_t channel, uint32_t * p_reg_buff)
{
    sdhi_sdhndl_t  * p_hndl = 0;

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if ((uint32_t)((sizeof(g_sdhi_ip_base)) / sizeof(uint32_t)) <= channel)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_UTIL, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    p_hndl = SDHI_GET_HNDL(channel);

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if (0 == p_hndl)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_UTIL, __LINE__);
        return SDHI_ERR;  /* Not initialized */
    }

    /*  Check the parameter.  */
    if (0 == p_reg_buff)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_UTIL, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    *p_reg_buff = (uint32_t)(p_hndl->reg_base + SDHI_SDBUFR);

    return SDHI_SUCCESS;
} /* End of function R_SDHI_GetBuffRegAddress() */

/**********************************************************************************************************************
* Outline      : Set SDHI Register
* Function Name: R_SDHI_OutReg
* Description  : Sets SDHI register.
* Arguments    : uint32_t           channel              ;   SDHI Channel No.
*              : uint32_t           reg                  ;   SDHI register
*              : uint32_t           data                 ;   Data
* Return Value : SDHI_SUCCESS                            ;   Successful operation
*              : SDHI_ERR                                ;   Failed operation
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t R_SDHI_OutReg(uint32_t channel, uint32_t reg, uint32_t data)
{
    sdhi_sdhndl_t  * p_hndl = 0;

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if ((uint32_t)((sizeof(g_sdhi_ip_base)) / sizeof(uint32_t)) <= channel)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_UTIL, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    p_hndl = SDHI_GET_HNDL(channel);

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if (0 == p_hndl)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_UTIL, __LINE__);
        return SDHI_ERR;  /* Not initialized */
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    SDHI_OUTREG(p_hndl, reg, data);
    if (0 == SDHI_INREG(p_hndl, reg))
    {
        R_BSP_NOP();  /* Wait for the write completion. */
    }

    return SDHI_SUCCESS;
} /* End of function R_SDHI_OutReg() */

/**********************************************************************************************************************
* Outline      : Get SDHI Register
* Function Name: R_SDHI_InReg
* Description  : Gets SDHI register.
* Arguments    : uint32_t           channel              ;   SDHI Channel No.
*              : uint32_t           reg                  ;   SDHI register
*              : uint32_t           * p_data             ;   Data pointer
* Return Value : SDHI_SUCCESS                            ;   Successful operation
*              : SDHI_ERR                                ;   Failed operation
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t R_SDHI_InReg(uint32_t channel, uint32_t reg, uint32_t * p_data)
{
    sdhi_sdhndl_t  * p_hndl = 0;

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if ((uint32_t)((sizeof(g_sdhi_ip_base)) / sizeof(uint32_t)) <= channel)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_UTIL, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    p_hndl = SDHI_GET_HNDL(channel);

#if (SDHI_CFG_PARAM_CHECKING_ENABLE == 1)
    if (0 == p_hndl)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_UTIL, __LINE__);
        return SDHI_ERR;  /* Not initialized */
    }

    if (0 == p_data)
    {
        SDHI_LOG_FUNC(SDHI_DEBUG_ERR_ID, (uint32_t)SDHI_UTIL, __LINE__);
        return SDHI_ERR;
    }
#endif /* SDHI_CFG_PARAM_CHECKING_ENABLE */

    *p_data = SDHI_INREG(p_hndl, reg);
    
    return SDHI_SUCCESS;
} /* End of function R_SDHI_InReg() */

/**********************************************************************************************************************
* Outline      : Get Version
* Function Name: R_SDHI_GetVersion
* Description  : Returns the version of this module. The version number is
*                encoded such that the top two bytes are the major version
*                number and the bottom two bytes are the minor version number.
* Arguments    : none
* Return Value : version number
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
uint32_t  R_SDHI_GetVersion(void)
{
    uint32_t const version = ((RX_SDHI_VERSION_MAJOR << SDHI_BIT_SHIFT_16) | RX_SDHI_VERSION_MINOR);

    return version;
} /* End of function R_SDHI_GetVersion() */

#ifdef SDHI_CFG_LONGQ_ENABLE                                /* Uses FIT LONGQ module                */
/**********************************************************************************************************************
* Outline      : Get Log HNDL Address
* Function Name: R_SDHI_SetLogHdlAddress
* Description  : Gets the Log HNDL Address.
* Arguments    : uint32_t           user_byte_que        ;   user_byte_que
* Return Value : SDHI_SUCCESS                            ;   Successful operation
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t R_SDHI_SetLogHdlAddress(uint32_t user_long_que)
{
    p_sdhi_long_que = (longq_hdl_t)user_long_que;

    return SDHI_SUCCESS;
} /* End of function R_SDHI_SetLogHdlAddress() */

/**********************************************************************************************************************
* Outline      : Store Debugging Information
* Function Name: r_sdhi_log
* Description  : Stores the debugging information.
* Arguments    : uint32_t           flg                  ;   Breakpoint processing
*              : uint32_t           fid                  ;   SDHI driver file No.
*              : uint32_t           line                 ;   SDHI driver line No.
* Return Value : 0      ; Successful
*              : 1      ; Error
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
uint32_t r_sdhi_log(uint32_t flg, uint32_t fid, uint32_t line)
{
    longq_err_t err = LONGQ_SUCCESS;
    uint32_t    ul_tmp = 0;

    /* Long to Byte */
    ul_tmp = 0;                                                        /* sdhi_log   Reserved               */
    ul_tmp = (ul_tmp | (line << SDHI_BIT_SHIFT_8));                    /* sdhi_log   LINE                   */
    ul_tmp = (ul_tmp | (fid  << SDHI_BIT_SHIFT_21));                   /* sdhi_log   FID FILE No.           */
    ul_tmp = (ul_tmp | (SDHI_DRIVER_ID << SDHI_BIT_SHIFT_27));         /* sdhi_log   FID DRIVER No.         */
    ul_tmp = (ul_tmp | (flg  << SDHI_BIT_SHIFT_31));                   /* sdhi_log   Breakpoint processing  */

    /* error log white */
    err = R_LONGQ_Put(p_sdhi_long_que, ul_tmp);

    if (err != LONGQ_SUCCESS)
    {
        return 1;
    }

    /* Breakpoint processing */
    if (0x80000000 == (ul_tmp & 0x80000000))
    {
        return 1;
    }

    return 0;
} /* End of function r_sdhi_log() */
#else
/**********************************************************************************************************************
* Outline      : Get Log HNDL Address
* Function Name: R_SDHI_SetLogHdlAddress
* Description  : Gets the Log HNDL Address.
* Arguments    : uint32_t           user_byte_que        ;   user_byte_que
* Return Value : SDHI_SUCCESS                            ;   Successful operation
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
sdhi_status_t R_SDHI_SetLogHdlAddress(uint32_t user_long_que)
{
    return SDHI_SUCCESS;
} /* End of function R_SDHI_SetLogHdlAddress() */
#endif /* SDHI_CFG_LONGQ_ENABLE */

/**********************************************************************************************************************
* Outline      : Store Debugging Information
* Function Name: R_SDHI_Log
* Description  : Stores the debugging information.
* Arguments    : uint32_t           flg                  ;   Breakpoint processing
*              : uint32_t           fid                  ;   SDHI driver file No.
*              : uint32_t           line                 ;   SDHI driver line No.
* Return Value : 0
*----------------------------------------------------------------------------------------------------------------------
* Notes        : None
**********************************************************************************************************************/
uint32_t R_SDHI_Log(uint32_t flg, uint32_t fid, uint32_t line)
{
    SDHI_LOG_FUNC(flg, fid, line);
    return 0;
} /* End of function R_SDHI_Log() */

/* End of File */
