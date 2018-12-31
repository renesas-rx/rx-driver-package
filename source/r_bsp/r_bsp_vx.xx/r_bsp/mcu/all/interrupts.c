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
* Copyright (C) 2013 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : interrupts.c
* Description  : This module allows for callbacks to be registered for certain interrupts. 
*                And handle exception interrupts.
***********************************************************************************************************************/
/**********************************************************************************************************************
* History : DD.MM.YYYY Version  Description
*         : xx.xx.xxxx 1.00     First Release
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include "platform.h"

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#ifdef BSP_MCU_FLOATING_POINT
/* Defines CV, CO, CZ, CU, CX, and CE bits. */
#define BSP_PRV_FPU_CAUSE_FLAGS     (0x000000FC)
#endif

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Exported global variables (to be accessed by other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/
/* This array holds callback functions. */
static void (* g_bsp_vectors[BSP_INT_SRC_TOTAL_ITEMS])(void * pdata);

/***********************************************************************************************************************
* Function Name: R_BSP_InterruptRequestEnable
* Description  : Interrupt request is enabled. (IERm.IENj = 1)
* Arguments    : vector - Which vector to enable.
* Return Value : none
***********************************************************************************************************************/
void R_BSP_InterruptRequestEnable (uint32_t vector)
{
    uint32_t ier_reg_num;
    uint32_t ien_bit_num;
    uint8_t  *p_ier_addr;

    /* Calculate the register number. (IER[m].IENj)(m = vector_number / 8) */
    ier_reg_num = vector >> 3;

    /* Calculate the bit number. (IERm.IEN[j])(j = vector_number % 8) */
    ien_bit_num = vector & 0x00000007;

    /* Casting is valid because it matches the type to the right side or argument. */
    p_ier_addr = (uint8_t *)&ICU.IER[ier_reg_num].BYTE;
    R_BSP_BIT_SET(p_ier_addr, ien_bit_num);
} /* End of function R_BSP_InterruptRequestEnable() */

/***********************************************************************************************************************
* Function Name: R_BSP_InterruptRequestDisable
* Description  : Interrupt request is disabled. (IERm.IENj = 0)
* Arguments    : vector - Which vector to disable.
* Return Value : none
***********************************************************************************************************************/
void R_BSP_InterruptRequestDisable (uint32_t vector)
{
    uint32_t ier_reg_num;
    uint32_t ien_bit_num;
    uint8_t  *p_ier_addr;

    /* Calculate the register number. (IER[m].IENj)(m = vector_number / 8) */
    ier_reg_num = vector >> 3;

    /* Calculate the bit number. (IERm.IEN[j])(j = vector_number % 8) */
    ien_bit_num = vector & 0x00000007;

    /* Casting is valid because it matches the type to the right side or argument. */
    p_ier_addr = (uint8_t *)&ICU.IER[ier_reg_num].BYTE;
    R_BSP_BIT_CLEAR(p_ier_addr, ien_bit_num);
} /* End of function R_BSP_InterruptRequestDisable() */

/***********************************************************************************************************************
* Function Name: bsp_interrupt_open
* Description  : Initialize callback function array.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void bsp_interrupt_open (void)
{
    uint32_t i;

    /* WAIT_LOOP */
    for (i = 0; i < BSP_INT_SRC_TOTAL_ITEMS; i++)
    {
        /* Casting is valid because it matches the type to the right side or argument. */
        g_bsp_vectors[i] = FIT_NO_FUNC;
    }

#ifdef BSP_MCU_SOFTWARE_CONFIGURABLE_INTERRUPT
    /* Initialize mapped interrupts. */
    bsp_mapped_interrupt_open();
#endif
} /* End of function bsp_interrupt_open() */

/***********************************************************************************************************************
* Function Name: R_BSP_InterruptWrite
* Description  : Registers a callback function for supported interrupts. If FIT_NO_FUNC, NULL, or
*                any other invalid function address is passed for the callback argument then any previously registered
*                callbacks are unregistered. Use of FIT_NO_FUNC is preferred over NULL since access to the address
*                defined by FIT_NO_FUNC will cause a bus error which is easy for the user to catch. NULL typically
*                resolves to 0 which is a valid address on RX MCUs.
* Arguments    : vector -
*                    Which interrupt to register a callback for.
*                callback -
*                    Pointer to function to call when interrupt occurs.
* Return Value : BSP_INT_SUCCESS -
*                    Callback registered
*                BSP_INT_ERR_INVALID_ARG -
*                    Invalid function address input, any previous function has been unregistered
***********************************************************************************************************************/
bsp_int_err_t R_BSP_InterruptWrite (bsp_int_src_t vector,  bsp_int_cb_t callback)
{
    bsp_int_err_t err;

    err = BSP_INT_SUCCESS;

    /* Check for valid address. */
    if (((uint32_t)callback == (uint32_t)NULL) || ((uint32_t)callback == (uint32_t)FIT_NO_FUNC))
    {
        /* Casting is valid because it matches the type to the right side or argument. */
        g_bsp_vectors[vector] = FIT_NO_FUNC;
    }
    else
    {
        g_bsp_vectors[vector] = callback;
    }

    return err;
} /* End of function R_BSP_InterruptWrite() */

/***********************************************************************************************************************
* Function Name: R_BSP_InterruptRead
* Description  : Returns the callback function address for an interrupt.
* Arguments    : vector -
*                    Which interrupt to read the callback for.
*                callback -
*                    Pointer of where to store callback address.
* Return Value : BSP_INT_SUCCESS -
*                    Callback was registered and address has been stored in 'callback' parameter.
*                BSP_INT_ERR_NO_REGISTERED_CALLBACK -
*                    No valid callback has been registered for this interrupt source.
***********************************************************************************************************************/
bsp_int_err_t R_BSP_InterruptRead (bsp_int_src_t vector, bsp_int_cb_t * callback)
{
    bsp_int_err_t err;

    err = BSP_INT_SUCCESS;

    /* Check for valid address. */
    if (((uint32_t)g_bsp_vectors[vector] == (uint32_t)NULL) || ((uint32_t)g_bsp_vectors[vector] == (uint32_t)FIT_NO_FUNC))
    {
        err = BSP_INT_ERR_NO_REGISTERED_CALLBACK;
    }
    else
    {
        *callback = g_bsp_vectors[vector];
    }

    return err;
} /* End of function R_BSP_InterruptRead() */

/***********************************************************************************************************************
* Function Name: R_BSP_InterruptControl
* Description  : Executes specified command.
* Arguments    : vector -
*                    Which vector is being used.
*                cmd -
*                    Which command to execute.
*                pdata -
*                    Pointer to data to use with command.
*                    Currently only used for BSP_INT_CMD_GROUP_INTERRUPT_ENABLE, and points
*                    to a bsp_int_ctrl_t *. Other commands should supply FIT_NO_PTR.
* Return Value : BSP_INT_SUCCESS -
*                    Command executed successfully.
*                BSP_INT_ERR_NO_REGISTERED_CALLBACK -
*                    No valid callback has been registered for this interrupt source.
*                BSP_INT_ERR_INVALID_ARG -
*                    Invalid command sent in.
*                BSP_INT_ERR_GROUP_STILL_ENABLED -
*                    Not all group interrupts were disabled so group interrupt was not disabled.
***********************************************************************************************************************/
bsp_int_err_t R_BSP_InterruptControl (bsp_int_src_t vector, bsp_int_cmd_t cmd, void * pdata)
{
    bsp_int_err_t       err;
    bsp_int_cb_args_t   cb_args;

    err = BSP_INT_SUCCESS;

#ifdef BSP_MCU_GROUP_INTERRUPT
    /* nothing */
#else
    /* This code is only used to remove compiler info messages about these parameters not being used. */
    INTERNAL_NOT_USED(pdata);
#endif

    switch (cmd)
    {
        case (BSP_INT_CMD_CALL_CALLBACK):
        {
            /* Casting is valid because it matches the type to the right side or argument. */
            if (((uint32_t)g_bsp_vectors[vector] != (uint32_t)NULL) &&
                ((uint32_t)g_bsp_vectors[vector] != (uint32_t)FIT_NO_FUNC))
            {
                /* Fill in callback info. */
                cb_args.vector = vector;

                g_bsp_vectors[vector](&cb_args);
            }
            else
            {
                err = BSP_INT_ERR_NO_REGISTERED_CALLBACK;
            }
        }
        break;

        case (BSP_INT_CMD_INTERRUPT_ENABLE):
        {
            err = bsp_interrupt_enable_disable(vector, true);
        }
        break;

        case (BSP_INT_CMD_INTERRUPT_DISABLE):
        {
            err = bsp_interrupt_enable_disable(vector, false);
        }
        break;

#ifdef BSP_MCU_GROUP_INTERRUPT
        case (BSP_INT_CMD_GROUP_INTERRUPT_ENABLE):
        {
            /* Casting is valid because it matches the type to the right side or argument. */ 
            err = bsp_interrupt_group_enable_disable(vector, true, ((bsp_int_ctrl_t *)pdata)->ipl);
        }
        break;

        case (BSP_INT_CMD_GROUP_INTERRUPT_DISABLE):
        {
            err = bsp_interrupt_group_enable_disable(vector, false, 0);
        }
        break;
#endif

        default:
        {
            err = BSP_INT_ERR_INVALID_ARG;
        }
        break;
    }

    return err;
} /* End of function R_BSP_InterruptControl() */

/* When using the user startup program, disable the following code. */
#if (BSP_CFG_STARTUP_DISABLE == 0)

#ifdef BSP_MCU_EXCEP_SUPERVISOR_INST_ISR
/***********************************************************************************************************************
* Function name: excep_supervisor_inst_isr
* Description  : Supervisor Instruction Violation ISR
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
R_BSP_ATTRIB_INTERRUPT void excep_supervisor_inst_isr(void)
{
    /* If user has registered a callback for this exception then call it. */
    R_BSP_InterruptControl(BSP_INT_SRC_EXC_SUPERVISOR_INSTR, BSP_INT_CMD_CALL_CALLBACK, FIT_NO_PTR);
} /* End of function excep_supervisor_inst_isr() */
#endif

#ifdef BSP_MCU_EXCEP_ACCESS_ISR
/***********************************************************************************************************************
* Function name: excep_access_isr
* Description  : Access exception ISR
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
R_BSP_ATTRIB_INTERRUPT void excep_access_isr(void)
{
    /* If user has registered a callback for this exception then call it. */
    R_BSP_InterruptControl(BSP_INT_SRC_EXC_ACCESS, BSP_INT_CMD_CALL_CALLBACK, FIT_NO_PTR);
} /* End of function excep_access_isr() */
#endif

#ifdef BSP_MCU_EXCEP_UNDEFINED_INST_ISR
/***********************************************************************************************************************
* Function name: excep_undefined_inst_isr
* Description  : Undefined instruction exception ISR
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
R_BSP_ATTRIB_INTERRUPT void excep_undefined_inst_isr(void)
{
    /* If user has registered a callback for this exception then call it. */
    R_BSP_InterruptControl(BSP_INT_SRC_EXC_UNDEFINED_INSTR, BSP_INT_CMD_CALL_CALLBACK, FIT_NO_PTR);
} /* End of function excep_undefined_inst_isr() */
#endif

#ifdef BSP_MCU_EXCEP_FLOATING_POINT_ISR
/***********************************************************************************************************************
* Function name: excep_floating_point_isr
* Description  : Floating point exception ISR
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
R_BSP_ATTRIB_INTERRUPT void excep_floating_point_isr(void)
{
    /* Used for reading FPSW register. */
    uint32_t temp_fpsw;

    /* If user has registered a callback for this exception then call it. */
    R_BSP_InterruptControl(BSP_INT_SRC_EXC_FPU, BSP_INT_CMD_CALL_CALLBACK, FIT_NO_PTR);

    /* Get current FPSW. */
    temp_fpsw = (uint32_t)R_BSP_GET_FPSW();

    /* Clear only the FPU exception flags. */
    R_BSP_SET_FPSW(temp_fpsw & ((uint32_t)~BSP_PRV_FPU_CAUSE_FLAGS));
} /* End of function excep_floating_point_isr() */
#endif

#ifdef BSP_MCU_NON_MASKABLE_ISR
/***********************************************************************************************************************
* Function name: non_maskable_isr
* Description  : Non-maskable interrupt ISR
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
R_BSP_ATTRIB_INTERRUPT void non_maskable_isr(void)
{
#ifdef BSP_MCU_NMI_EXC_NMI_PIN
    /* Determine what is the cause of this interrupt. */
    if (1 == ICU.NMISR.BIT.NMIST)
    {
        /* NMI pin interrupt is requested. */
        R_BSP_InterruptControl(BSP_INT_SRC_EXC_NMI_PIN, BSP_INT_CMD_CALL_CALLBACK, FIT_NO_PTR);

        /* Clear NMI pin interrupt flag. */
        ICU.NMICLR.BIT.NMICLR = 1;
    }
#endif

#ifdef BSP_MCU_NMI_OSC_STOP_DETECT
    if (1 == ICU.NMISR.BIT.OSTST)
    {
        /* Oscillation stop detection interrupt is requested. */
        R_BSP_InterruptControl(BSP_INT_SRC_OSC_STOP_DETECT, BSP_INT_CMD_CALL_CALLBACK, FIT_NO_PTR);

        /* Clear oscillation stop detect flag. */
        ICU.NMICLR.BIT.OSTCLR = 1;
    }
#endif

#ifdef BSP_MCU_NMI_WDT_ERROR
    if (1 == ICU.NMISR.BIT.WDTST)
    {
        /* WDT underflow/refresh error interrupt is requested. */
        R_BSP_InterruptControl(BSP_INT_SRC_WDT_ERROR, BSP_INT_CMD_CALL_CALLBACK, FIT_NO_PTR);

        /* Clear WDT flag. */
        ICU.NMICLR.BIT.WDTCLR = 1;
    }
#endif

#ifdef BSP_MCU_NMI_LVD
    if (1 == ICU.NMISR.BIT.LVDST)
    {
        /* Voltage monitoring 1 interrupt is requested. */
        R_BSP_InterruptControl(BSP_INT_SRC_LVD1, BSP_INT_CMD_CALL_CALLBACK, FIT_NO_PTR);
    }
#endif

#ifdef BSP_MCU_NMI_IWDT_ERROR
    if (1 == ICU.NMISR.BIT.IWDTST)
    {
        /* IWDT underflow/refresh error interrupt is requested. */
        R_BSP_InterruptControl(BSP_INT_SRC_IWDT_ERROR, BSP_INT_CMD_CALL_CALLBACK, FIT_NO_PTR);

        /* Clear IWDT flag. */
        ICU.NMICLR.BIT.IWDTCLR = 1;
    }
#endif

#ifdef BSP_MCU_NMI_LVD1
    if (1 == ICU.NMISR.BIT.LVD1ST)
    {
        /* Voltage monitoring 1 interrupt is requested. */
        R_BSP_InterruptControl(BSP_INT_SRC_LVD1, BSP_INT_CMD_CALL_CALLBACK, FIT_NO_PTR);

        /* Clear LVD1 flag. */
        ICU.NMICLR.BIT.LVD1CLR = 1;
    }
#endif

#ifdef BSP_MCU_NMI_LVD2
    if (1 == ICU.NMISR.BIT.LVD2ST)
    {
        /* Voltage monitoring 1 interrupt is requested. */
        R_BSP_InterruptControl(BSP_INT_SRC_LVD2, BSP_INT_CMD_CALL_CALLBACK, FIT_NO_PTR);

        /* Clear LVD2 flag. */
        ICU.NMICLR.BIT.LVD2CLR = 1;
    }
#endif

#ifdef BSP_MCU_NMI_VBATT
    if (1 == ICU.NMISR.BIT.VBATST)
    {
        /* VBATT monitoring interrupt is requested. */
        R_BSP_InterruptControl(BSP_INT_SRC_VBATT, BSP_INT_CMD_CALL_CALLBACK, FIT_NO_PTR);

        /* Clear LVD2 flag. */
        ICU.NMICLR.BIT.VBATCLR = 1;
    }
#endif

#ifdef BSP_MCU_NMI_ECCRAM
    if (1 == ICU.NMISR.BIT.ECCRAMST)
    {
        /* ECCRAM Error interrupt is requested. */
        R_BSP_InterruptControl(BSP_INT_SRC_ECCRAM, BSP_INT_CMD_CALL_CALLBACK, FIT_NO_PTR);

        /* Clear ECCRAM flags. */
        ECCRAM.ECCRAM1STS.BIT.ECC1ERR = 0;
        ECCRAM.ECCRAM2STS.BIT.ECC2ERR = 0;
    }
#endif

#ifdef BSP_MCU_NMI_RAM
    if (1 == ICU.NMISR.BIT.RAMST)
    {
        if(1 == RAM.RAMSTS.BIT.RAMERR)
        {
            /* RAM Error interrupt is requested. */
            R_BSP_InterruptControl(BSP_INT_SRC_RAM, BSP_INT_CMD_CALL_CALLBACK, FIT_NO_PTR);

            /* Clear RAM flags. */
            RAM.RAMSTS.BIT.RAMERR = 0;
        }

        if(1 == RAM.EXRAMSTS.BIT.EXRAMERR)
        {
            /* Expansion RAM Error interrupt is requested. */
            R_BSP_InterruptControl(BSP_INT_SRC_EXRAM, BSP_INT_CMD_CALL_CALLBACK, FIT_NO_PTR);

            /* Clear Expansion RAM flags. */
            RAM.EXRAMSTS.BIT.EXRAMERR = 0;
        }
    }
#endif

#ifdef BSP_MCU_NMI_RAM_ECCRAM
    if (1 == ICU.NMISR.BIT.RAMST)
    {
        if(1 == RAM.RAMSTS.BIT.RAMERR)
        {
            /* RAM Error interrupt is requested. */
            R_BSP_InterruptControl(BSP_INT_SRC_RAM, BSP_INT_CMD_CALL_CALLBACK, FIT_NO_PTR);

            /* Clear RAM flags. */
            RAM.RAMSTS.BIT.RAMERR = 0;

            /* Clear ECCRAM flags. */
            RAM.ECCRAM1STS.BIT.ECC1ERR = 0;
            RAM.ECCRAM2STS.BIT.ECC2ERR = 0;
        }

    }
#endif
} /* End of function non_maskable_isr() */
#endif /* BSP_MCU_NON_MASKABLE_ISR */

#ifdef BSP_MCU_UNDEFINED_INTERRUPT_SOURCE_ISR
/***********************************************************************************************************************
* Function name: undefined_interrupt_source_isr
* Description  : All undefined interrupt vectors point to this function.
*                Set a breakpoint in this function to determine which source is creating unwanted interrupts.
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
R_BSP_ATTRIB_INTERRUPT void undefined_interrupt_source_isr(void)
{
    /* If user has registered a callback for this exception then call it. */
    R_BSP_InterruptControl(BSP_INT_SRC_UNDEFINED_INTERRUPT, BSP_INT_CMD_CALL_CALLBACK, FIT_NO_PTR);
} /* End of function undefined_interrupt_source_isr() */
#endif

#ifdef BSP_MCU_BUS_ERROR_ISR
/***********************************************************************************************************************
* Function name: bus_error_isr
* Description  : By default, this demo code enables the Bus Error Interrupt. This interrupt will fire if the user tries 
*                to access code or data from one of the reserved areas in the memory map, including the areas covered 
*                by disabled chip selects. A nop() statement is included here as a convenient place to set a breakpoint 
*                during debugging and development, and further handling should be added by the user for their 
*                application.
* Arguments    : none
* Return value : none
***********************************************************************************************************************/
R_BSP_ATTRIB_INTERRUPT void bus_error_isr (void)
{
    /* Clear the bus error */
    BSC.BERCLR.BIT.STSCLR = 1;

    /* 
        To find the address that was accessed when the bus error occurred, read the register BSC.BERSR2.WORD.
        The upper 13 bits of this register contain the upper 13-bits of the offending address (in 512K byte units)
    */

    /* If user has registered a callback for this exception then call it. */
    R_BSP_InterruptControl(BSP_INT_SRC_BUS_ERROR, BSP_INT_CMD_CALL_CALLBACK, FIT_NO_PTR);
} /* End of function bus_error_isr() */
#endif

#endif /* BSP_CFG_STARTUP_DISABLE == 0 */

