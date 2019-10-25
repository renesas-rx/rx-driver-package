/*****************************************************************************
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
* Copyright (C) 2014-2019 Renesas Electronics Corporation. All rights reserved.
******************************************************************************/
/*****************************************************************************
* File Name    : r_dac_rx.c
* Description  : Functions for using DAC on RX devices.
******************************************************************************
* History : DD.MM.YYYY Version Description
*           31.03.2014 2.00    New API
*           05.09.2014 2.10    Added support for RX64M.
*           20.01.2015 2.20    Added support for RX113.
*           02.03.2015 2.30    Added support for RX71M.
*           10.06.2015 2.40    Added support for RX231.
*           30.09.2015 2.50    Added support for RX23T.
*           01.10.2015 2.60    Added support for RX130.
*           01.12.2015 2.70    Added support for RX230.
*           01.02.2016 2.80    Added support for RX24T.
*           31.08.2016 2.91    Added support for RX65N.
*           19.12.2016 3.00    Added support for RX24U.
*                              Corrected so that AMADSEL bit does not become "1",
*                               when synchronous conversion is enabled. (RX64M/RX71M/RX65N)
*           21.07.2017 3.10    Added support for RX65N-2M, RX130-512KB.
*           28.09.2018 3.20    Added support for RX66T.
*                              Updated follow GSCE Code rules 5.0
*           01.02.2019 3.30    Added support for RX72T, RX65N-64pin
*           20.05.2019 4.00    Added support for GNUC and ICCRX.
*           28.06.2019 4.10    Added support for RX23W.
*           15.08.2019 4.20    Added support for RX72M.
******************************************************************************/
/*****************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/
#include "platform.h"
#include "r_dac_rx_if.h"
#include "r_dac_rx_config.h"
/*****************************************************************************
Typedef definitions
******************************************************************************/

/*****************************************************************************
Macro definitions
******************************************************************************/
#define DAC_PRV_CHANS_OFF   (0x1F)
#if (DAC_CFG_NUM_CH == 1)
#define DAC_PRV_CHANS_ON    (0xDF)
#else
#define DAC_PRV_CHANS_ON    (0x5F)
#endif

/*****************************************************************************
Private global variables and functions
******************************************************************************/
static void power_on(void);
static void power_off(void);
static dac_err_t dac_set_options(dac_cfg_t *p_cfg);

/*****************************************************************************
* Function Name: R_DAC_Open
* Description  : Initializes the DAC peripheral
* Arguments    : p_cfg -
*                     pointer to configuration structure
* Return Value : DAC_SUCCESS -
*                    DAC initialized successfully
*                DAC_ERR_NULL_PTR
*                    p_cfg is NULL
*                DAC_ERR_INVALID_ARG
*                    illegal unit number (RX64M, RX71M, RX65N, RX651, RX72M)
*                DAC_ERR_LOCK_FAILED -
*                    DAC already opened
*                DAC_ERR_ADC_NOT_POWERED -
*                    Cannot sync because ADC is not powered
*                DAC_ERR_ADC_CONVERTING
*                    Cannot sync with ADC because it is doing a conversion
******************************************************************************/
dac_err_t R_DAC_Open(dac_cfg_t *p_cfg)
{
dac_err_t err;

#if (DAC_CFG_PARAM_CHECKING_ENABLE == 1)
    /* Cast from void* to dac_cfg_t* is valid */
    if ((NULL == p_cfg) || (FIT_NO_PTR == p_cfg))
    {
        return DAC_ERR_NULL_PTR;
    }
#if defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX65N) || defined(BSP_MCU_RX72M)
    if (p_cfg->sync_unit > 1)
    {
        return DAC_ERR_INVALID_ARG;
    }
#endif
#endif
    if (R_BSP_HardwareLock(BSP_LOCK_DA) == false)
    {
        return DAC_ERR_LOCK_FAILED;
    }

    power_on();

    /* Select output for DA Module*/
#if defined(BSP_MCU_RX66_ALL) || defined(BSP_MCU_RX72T)

    /* Turn off all output select*/
    DA.DADSELR.BYTE = 0;

    /* Select output to DA*/
    switch(p_cfg->out_sel_da)
    {
    case DAC_OUT_DA_OFF:
        {
            /* Not select output to DA0 */
            DA.DADSELR.BIT.OUTDA0 = 0;

            /*Not select output to DA1*/
            DA.DADSELR.BIT.OUTDA1 = 0;
            break;
        }
    case DAC_OUT_SEL_DA0:
        {
            /* Select output to DA0 */
            DA.DADSELR.BIT.OUTDA0 = 1;
            break;
        }
    case DAC_OUT_SEL_DA1:
        {
            /* Select output to DA1 */
            DA.DADSELR.BIT.OUTDA1 = 1;
            break;
        }
    default:
        {
            break;
        }
    }

    /* Select output to reference voltage */
    switch(p_cfg->out_sel_ref)
    {
    case DAC_OUT_REF_OFF:
        {
            /* Not select output to VREF0 */
            DA.DADSELR.BIT.OUTREF0 = 0;

            /* Not select output to VREF1 */
            DA.DADSELR.BIT.OUTREF1 = 0;
            break;
        }
    case DAC_OUT_SEL_REF0:
        {
            /* Select output to VREF0 */
            DA.DADSELR.BIT.OUTREF0 = 1;
            break;
        }
    case DAC_OUT_SEL_REF1:
        {
            /* Select output to VREF1 */
            DA.DADSELR.BIT.OUTREF1 = 1;
            break;
        }
    default:
        {
            break;
        }
    }
#endif   /* End of #if (defined(BSP_MCU_RX66_ALL) */

    /* Set registers to default state */
    /* channel output disabled */
    DA.DACR.BYTE = DAC_PRV_CHANS_OFF;

    /* ch0 data = 0 */
    DA.DADR0 = 0;
#if (DAC_CFG_NUM_CH > 1)
    /* ch1 data = 0 */
    DA.DADR1 = 0;
#endif

    /* Set options */
    err = dac_set_options(p_cfg);
    if (DAC_SUCCESS != err)
    {
        R_BSP_HardwareUnlock(BSP_LOCK_DA);
    }

    return err;
}
/************************************************************************
* End of function R_DAC_Open
*************************************************************************/

/*****************************************************************************
* Function Name: dac_set_options
* Description  : Configures MCU-unique options.
*                Note: See Application Note for this module for pin configuration requirements.
* Arguments    : p_cfg -
*                     pointer to configuration structure
* Return Value : DAC_SUCCESS -
*                    DAC configured successfully
*                DAC_ERR_INVALID_ARG -
*                    Could not set reference voltage
*                DAC_ERR_ADC_NOT_POWERED
*                    Cannot sync because ADC is not powered
*                DAC_ERR_ADC_CONVERTING
*                    Cannot sync because ADC is converting
******************************************************************************/
static dac_err_t dac_set_options(dac_cfg_t *p_cfg)
{
    dac_err_t err = DAC_SUCCESS;

    /* OPTION: SELECT DAC VOLTAGE REFERENCE */

#if defined(BSP_MCU_RX113) || defined(BSP_MCU_RX231) || defined(BSP_MCU_RX230) || defined(BSP_MCU_RX23W)
#if (DAC_CFG_PARAM_CHECKING_ENABLE == 1)
    /* CHECK PARAMETERS */
#if defined(BSP_MCU_RX23W)
    if ((p_cfg->ref_voltage != DAC_REFV_AVCC0_AVSS0) && (p_cfg->ref_voltage != DAC_REFV_INTERNAL_AVSS0))
    {
        return DAC_ERR_INVALID_ARG;
    }
#else
    if ((p_cfg->ref_voltage != DAC_REFV_AVCC0_AVSS0) && (p_cfg->ref_voltage != DAC_REFV_INTERNAL_AVSS0) && (p_cfg->ref_voltage != DAC_REFV_VREFH_VREFL))
    {
        return DAC_ERR_INVALID_ARG;
    }
#endif
#endif

    DA.DAVREFCR.BYTE = p_cfg->ref_voltage;
#endif

    /* OPTION: FORMAT */
    DA.DADPR.BIT.DPSEL = (uint8_t)((true == p_cfg->fmt_flush_right ) ? 0 : 1);


    /* OPTION: SYNCHRONIZE WITH ADC */

#if defined(BSP_MCU_RX113) || defined(BSP_MCU_RX130) || defined(BSP_MCU_RX231) || defined(BSP_MCU_RX230) || defined(BSP_MCU_RX23W) || defined(BSP_MCU_RX63_ALL) 
    if (p_cfg->sync_with_adc == false)
    {
        DA.DAADSCR.BIT.DAADST = 0;      // do not sync with ADC
    }
    else if ((SYSTEM.MSTPCRA.BIT.MSTPA17 == 0) && (S12AD.ADCSR.BIT.ADST == 1))
    {
        err = DAC_ERR_ADC_CONVERTING;
    }
    else if (SYSTEM.MSTPCRA.BIT.MSTPA17 == 1)
    {
        err = DAC_ERR_ADC_NOT_POWERED;
    }
    else
    {
        DA.DAADSCR.BIT.DAADST = 1;      // set sync with ADC
    }
#elif defined(BSP_MCU_RX64_ALL) || defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX65N) || defined(BSP_MCU_RX66_ALL) || defined(BSP_MCU_RX72T) || defined(BSP_MCU_RX72M)
    if (false == p_cfg->sync_with_adc )
    {
        /* Not sync with ADC */
        DA.DAADSCR.BIT.DAADST = 0;
    }
    else
    {
#if defined(BSP_MCU_RX64_ALL) || defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX65N) || defined(BSP_MCU_RX72M)
        /*
         * This function returns an error because unit0 can not be selected
         * when synchronous conversion is enabled.
         */
        if (0 == p_cfg->sync_unit)
        {
            if ((0 == SYSTEM.MSTPCRA.BIT.MSTPA17) && (1 == S12AD.ADCSR.BIT.ADST))
            {
                err = DAC_ERR_ADC_CONVERTING;
            }
            else if (1 == SYSTEM.MSTPCRA.BIT.MSTPA17)
            {
                err = DAC_ERR_ADC_NOT_POWERED;
            }
            else
            {
                err = DAC_ERR_INVALID_ARG;
            }
        }
        else if ((0 == SYSTEM.MSTPCRA.BIT.MSTPA16) && (1 == S12AD1.ADCSR.BIT.ADST))
        {
            err = DAC_ERR_ADC_CONVERTING;
        }
        else if (1 == SYSTEM.MSTPCRA.BIT.MSTPA16)
        {
            err = DAC_ERR_ADC_NOT_POWERED;
        }
#elif defined(BSP_MCU_RX66_ALL) || defined(BSP_MCU_RX72T)

        /* Checking value of MSTPA23 bit & ADST bit */
        if ((0 == SYSTEM.MSTPCRA.BIT.MSTPA23) && (1 == S12AD2.ADCSR.BIT.ADST))
        {
            err = DAC_ERR_ADC_CONVERTING;
        }

        /* Checking value of MSTPA23 bit  */
        else if (1 == SYSTEM.MSTPCRA.BIT.MSTPA23)
        {
            err = DAC_ERR_ADC_NOT_POWERED;
        }
#endif
        else
        {
        /* Do Nothing */
          
        }

        if (DAC_SUCCESS == err)
        {
#if !(defined(BSP_MCU_RX66_ALL) || defined(BSP_MCU_RX72T)) 
            DA.DAADUSR.BIT.AMADSEL1 = p_cfg->sync_unit;
#endif
            /* Set sync with ADC */
            DA.DAADSCR.BIT.DAADST = 1;
        }
    }
#elif defined(BSP_MCU_RX24U)
    if (p_cfg->sync_with_adc == false)
    {
        DA.DAADSCR.BIT.DAADST = 0;      // do not sync with ADC
    }
    else if ((SYSTEM.MSTPCRA.BIT.MSTPA23 == 0) && (S12AD2.ADCSR.BIT.ADST == 1))
    {
        err = DAC_ERR_ADC_CONVERTING;
    }
    else if (SYSTEM.MSTPCRA.BIT.MSTPA23 == 1)
    {
        err = DAC_ERR_ADC_NOT_POWERED;
    }
    else
    {
        DA.DAADSCR.BIT.DAADST = 1;      // set sync with ADC
    }
#endif

    /* OPTION: TURN CHANNEL CONVERTER OFF WHEN CHANNEL OUTPUT IS DISABLED */
#if defined(BSP_MCU_RX21_ALL) || defined(BSP_MCU_RX63_ALL) || defined(BSP_MCU_RX64_ALL) || defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX65N) \
 || defined(BSP_MCU_RX66_ALL) || defined(BSP_MCU_RX72T) || defined(BSP_MCU_RX72M)

    DA.DACR.BIT.DAE = (uint8_t)((true == p_cfg->ch_conv_off_when_output_off ) ? 0 : 1);
#endif

    return err;
}
/************************************************************************
* End of function dac_set_options
*************************************************************************/

/*****************************************************************************
* Function Name: power_on
* Description  : This function provides enables power to the DAC peripheral.
* Arguments    : none
* Return Value : none
******************************************************************************/
static void power_on(void)
{

    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_LPC_CGC_SWR);  // unlock

    /* power on DAC */
    MSTP(DA) = 0;
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_LPC_CGC_SWR);   // re-lock

    return;
}
/* End of function power_on */


/*****************************************************************************
* Function Name: power_off
* Description  : This function removes power to the DAC peripheral
* Arguments    : none
* Return Value : none
******************************************************************************/
static void power_off(void)
{

    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_LPC_CGC_SWR);  // unlock

    /* power off DAC */
    MSTP(DA) = 1;
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_LPC_CGC_SWR);   // re-lock

    return;
}
/* End of function power_off */


/*****************************************************************************
* Function Name: R_DAC_Write
* Description  : Puts data in DAC data register for conversion on a DAC channel.
* Arguments    : chan -
*                    channel referred to
*                data -
*                    data to be written to register
* Return Value : DAC_SUCCESS -
*                    data written to register
*                DAC_ERR_BAD_CHAN -
*                    channel number invalid for part
******************************************************************************/
dac_err_t R_DAC_Write(uint8_t const chan, uint16_t data)
{
dac_err_t   err=DAC_SUCCESS;


#if (DAC_CFG_PARAM_CHECKING_ENABLE == 1)
    if (chan >= DAC_NUM_CH)
    {
        return DAC_ERR_BAD_CHAN;
    }
#endif

#if (DAC_CFG_NUM_CH == 1)
    DA.DADR0 = data;
#else
    if (DAC_CH0 == chan)
    {
        /* Write data to register of channel 0 */
        DA.DADR0 = data;
    }
    else
    {
        /* Write data to register of channel 1 */
        DA.DADR1 = data;
    }
#endif

    return err;
}
/************************************************************************
* End of function R_DAC_Write
*************************************************************************/


/*****************************************************************************
* Function Name: R_DAC_Control
* Description  : This function is used for special hardware configuration and
*                operational control.
* Arguments    : chan -
*                    channel(s) to operate on
*                cmd -
*                    command to run
* Return Value : DAC_SUCCESS -
*                    Command completed successfully.
*                DAC_ERR_INVALID_CMD -
*                    Unknown command
*                DAC_ERR_BAD_CHAN -
*                    channel number invalid
******************************************************************************/
dac_err_t R_DAC_Control(uint8_t const chan, dac_cmd_t const cmd)
{
dac_err_t   err=DAC_SUCCESS;


#if (DAC_CFG_PARAM_CHECKING_ENABLE == 1)
    if (chan >= DAC_NUM_CH)
    {
        return DAC_ERR_BAD_CHAN;
    }
#endif

    switch (cmd)
    {
    case (DAC_CMD_OUTPUT_ON):
#if (DAC_CFG_NUM_CH == 1)
        DA.DACR.BIT.DAOE0 = 1;
#else
    {
        if (DAC_CH0 == chan)
        {
            /* Enable D/A conversion, analog output of channel 0 */
            DA.DACR.BIT.DAOE0 = 1;
        }
        else
        {
            /* Enable D/A conversion, analog output of channel 1 */
            DA.DACR.BIT.DAOE1 = 1;
        }
    }
#endif
    break;

    case (DAC_CMD_OUTPUT_OFF):
#if (DAC_CFG_NUM_CH == 1)
        DA.DACR.BIT.DAOE0 = 0;
#else
    {
        if (DAC_CH0 == chan)
        {
            /* Disable analog output of channel 0 */
            DA.DACR.BIT.DAOE0 = 0;
        }
        else
        {
            /* Disable analog output of channel 1 */
            DA.DACR.BIT.DAOE1 = 0;
        }
    }
#endif
    break;

#if defined(BSP_MCU_RX64_ALL) || defined(BSP_MCU_RX71M)
    case (DAC_CMD_AMP_ON):
        if (chan == DAC_CH0)
        {
            DA.DAAMPCR.BIT.DAAMP0 = 1;
        }
        else
        {
            DA.DAAMPCR.BIT.DAAMP1 = 1;
        }
    break;

    case (DAC_CMD_AMP_OFF):
        if (chan == DAC_CH0)
        {
            DA.DAAMPCR.BIT.DAAMP0 = 0;
        }
        else
        {
            DA.DAAMPCR.BIT.DAAMP1 = 0;
        }
    break;
#endif

    default:
    {
        err = DAC_ERR_INVALID_CMD;
    break;
    }
    }

    return err;
}
/************************************************************************
* End of function R_DAC_Control
*************************************************************************/


/*****************************************************************************
* Function Name: R_DAC_Close
* Description  : Shuts down the DAC peripheral
*
* Arguments    : none
* Return Value : DAC_SUCCESS -
*                    Peripheral disabled
******************************************************************************/
dac_err_t R_DAC_Close(void)
{
    /* Stop conversion/disable channels */
    DA.DACR.BYTE = DAC_PRV_CHANS_OFF;
    
    /* ch0 data = 0 */
    DA.DADR0 = 0;
#if (DAC_CFG_NUM_CH > 1)
    /* ch1 data = 0 */
    DA.DADR1 = 0;
#endif
    /* Right justified */
    DA.DADPR.BIT.DPSEL = 0;
#if defined(BSP_MCU_RX113) || defined(BSP_MCU_RX130) || defined(BSP_MCU_RX231) || defined(BSP_MCU_RX230) \
 || defined(BSP_MCU_RX63_ALL) || defined(BSP_MCU_RX64_ALL) || defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX65N) \
 || defined(BSP_MCU_RX24U) || defined(BSP_MCU_RX66_ALL) || defined(BSP_MCU_RX72T) || defined(BSP_MCU_RX23W) || defined(BSP_MCU_RX72M) 
    /* Not sync with ADC */
    DA.DAADSCR.BIT.DAADST = 0;
#endif

#if defined(BSP_MCU_RX64_ALL) || defined(BSP_MCU_RX71M)
    DA.DAAMPCR.BIT.DAAMP0 = 0;          // not used AMP0
    DA.DAAMPCR.BIT.DAAMP1 = 0;          // not used AMP1
#endif

#if defined(BSP_MCU_RX64_ALL) || defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX65N) || defined(BSP_MCU_RX72M)
    DA.DAADUSR.BIT.AMADSEL1 = 0;        // not sync unit1
#endif

#if defined(BSP_MCU_RX66_ALL) || defined(BSP_MCU_RX72T)
    /* Turn DAC output select off */
    DA.DADSELR.BYTE = 0;
#endif
    power_off();

    R_BSP_HardwareUnlock(BSP_LOCK_DA);

    return DAC_SUCCESS;
}
/************************************************************************
* End of function R_DAC_Close
*************************************************************************/

/*****************************************************************************
* Function Name: R_DAC_GetVersion
* Description  : Returns the version of this module. The version number is 
*                encoded such that the top two bytes are the major version
*                number and the bottom two bytes are the minor version number.
* Arguments    : none
* Return Value : version number
******************************************************************************/
uint32_t  R_DAC_GetVersion(void)
{
uint32_t const version = (DAC_VERSION_MAJOR << 16) | DAC_VERSION_MINOR;

    return version;
}
/************************************************************************
* End of function R_DAC_GetVersion
*************************************************************************/
