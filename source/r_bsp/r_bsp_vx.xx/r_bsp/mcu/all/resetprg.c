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
* Copyright (C) 2014 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : resetprg.c
* Description  : Defines post-reset routines that are used to configure the MCU prior to the main program starting. 
*                This is where the program counter starts on power-up or reset.
***********************************************************************************************************************/
/***********************************************************************************************************************
* History : DD.MM.YYYY Version   Description
*         : 28.02.2019 3.00      Merged processing of all devices.
*                                Added support for GNUC and ICCRX.
*                                Fixed coding style.
*                                Renamed following macro definitions.
*                                - BSP_PRV_PSW_INIT
*                                - BSP_PRV_FPSW_INIT
*                                - BSP_PRV_FPU_ROUND
*                                - BSP_PRV_FPU_DENOM
*                                Added following macro definitions.
*                                - BSP_PRV_DPSW_INIT
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#if defined(__CCRX__)
/* Defines MCU configuration functions used in this file */
#include    <_h_c_lib.h>
#endif /* defined(__CCRX__) */

/* Define the target platform */
#include    "platform.h"

/* When using the user startup program, disable the following code. */
#if BSP_CFG_STARTUP_DISABLE == 0

/* Declaration of stack size. */
#if BSP_CFG_USER_STACK_ENABLE == 1
R_BSP_PRAGMA_STACKSIZE_SU(BSP_CFG_USTACK_BYTES)
#endif
R_BSP_PRAGMA_STACKSIZE_SI(BSP_CFG_ISTACK_BYTES)

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
/* If the user chooses only 1 stack then the 'U' bit will not be set and the CPU will always use the interrupt stack. */
#if BSP_CFG_USER_STACK_ENABLE == 1
    #define BSP_PRV_PSW_INIT  (0x00030000)
#else
    #define BSP_PRV_PSW_INIT  (0x00010000)
#endif

#if defined(__CCRX__) || defined(__GNUC__)

#ifdef BSP_MCU_FLOATING_POINT
    /* Initialize FPSW for floating-point operations */
#define BSP_PRV_FPSW_INIT (0x00000000)  /* Currently nothing set by default. */
#ifdef BSP_MCU_DOUBLE_PRECISION_FLOATING_POINT
    /* Initialize DPSW for double-precision floating-point operations */
#define BSP_PRV_DPSW_INIT (0x00000000)  /* Currently nothing set by default. */
#endif

#ifdef __ROZ
#define BSP_PRV_FPU_ROUND (0x00000001)  /* Let FPSW RMbits=01 (round to zero) */
#else
#define BSP_PRV_FPU_ROUND (0x00000000)  /* Let FPSW RMbits=00 (round to nearest) */
#endif
#ifdef __DOFF
#define BSP_PRV_FPU_DENOM (0x00000100)  /* Let FPSW DNbit=1 (denormal as zero) */
#else
#define BSP_PRV_FPU_DENOM (0x00000000)  /* Let FPSW DNbit=0 (denormal as is) */
#endif
#endif

#endif /* defined(__CCRX__), defined(__GNUC__) */

/***********************************************************************************************************************
Pre-processor Directives
***********************************************************************************************************************/
/* Set this as the entry point from a power-on reset */
#if defined(__CCRX__)
#pragma entry PowerON_Reset_PC
#endif /* defined(__CCRX__) */

/***********************************************************************************************************************
External function Prototypes
***********************************************************************************************************************/
/* Initialize C runtime environment */
extern void _INITSCT(void);

#if defined(CPPAPP)
/* Initialize C++ global class object */
extern void _CALL_INIT(void);
#endif

#if BSP_CFG_USER_WARM_START_CALLBACK_PRE_INITC_ENABLED != 0
/* If user is requesting warm start callback functions then these are the prototypes. */
void BSP_CFG_USER_WARM_START_PRE_C_FUNCTION(void);
#endif

#if BSP_CFG_USER_WARM_START_CALLBACK_POST_INITC_ENABLED != 0
/* If user is requesting warm start callback functions then these are the prototypes. */
void BSP_CFG_USER_WARM_START_POST_C_FUNCTION(void);
#endif

#if BSP_CFG_RTOS_USED == 1  //FreeRTOS
/* A function is used to create a main task, rtos's objects required to be available in advance. */
extern void Processing_Before_Start_Kernel(void);
#endif

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/
/* Power-on reset function declaration */
R_BSP_POR_FUNCTION(R_BSP_STARTUP_FUNCTION);

/* Main program function declaration */
#if BSP_CFG_RTOS_USED == 0    /* Non-OS */
extern void R_BSP_MAIN_FUNCTION(void);
#endif

/***********************************************************************************************************************
* Function name: PowerON_Reset_PC
* Description  : This function is the MCU's entry point from a power-on reset.
*                The following steps are taken in the startup code:
*                1. The User Stack Pointer (USP) and Interrupt Stack Pointer (ISP) are both set immediately after entry 
*                   to this function. The USP and ISP stack sizes are set in the file bsp_config.h.
*                2. The interrupt vector base register is set to point to the beginning of the relocatable interrupt 
*                   vector table.
*                3. The MCU is setup for floating point operations by setting the initial value of the Floating Point 
*                   Status Word (FPSW).
*                4. The MCU operating frequency is set by configuring the Clock Generation Circuit (CGC) in
*                   operating_frequency_set.
*                5. Calls are made to functions to setup the C runtime environment which involves initializing all 
*                   initialed data, zeroing all uninitialized variables, and configuring STDIO if used
*                   (calls to _INITSCT and init_iolib).
*                6. Board-specific hardware setup, including configuring I/O pins on the MCU, in hardware_setup.
*                7. Global interrupts are enabled by setting the I bit in the Program Status Word (PSW), and the stack 
*                   is switched from the ISP to the USP.  The initial Interrupt Priority Level is set to zero, enabling 
*                   any interrupts with a priority greater than zero to be serviced.
*                8. The processor is optionally switched to user mode.  To run in user mode, set the macro 
*                   BSP_CFG_RUN_IN_USER_MODE above to a 1.
*                9. The bus error interrupt is enabled to catch any accesses to invalid or reserved areas of memory.
*
*                Once this initialization is complete, the user's main() function is called.  It should not return.
* Arguments    : none
* Return value : none
***********************************************************************************************************************/
R_BSP_POR_FUNCTION(R_BSP_STARTUP_FUNCTION)
{
    /* Stack pointers are setup prior to calling this function - see comments above */

    /* You can use auto variables in this funcion but such variables other than register variables 
     * will be unavailable after you change the stack from the I stack to the U stack (if change). */

    /* The bss sections have not been cleared and the data sections have not been initialized 
     * and constructors of C++ objects have not been executed until the _INITSCT() is executed. */
#if defined(__GNUC__)
#if BSP_CFG_USER_STACK_ENABLE == 1
    INTERNAL_NOT_USED(ustack_area);
#endif
    INTERNAL_NOT_USED(istack_area);
#endif

#if defined(__CCRX__) || defined(__GNUC__)

    /* Initialize the Interrupt Table Register */
    R_BSP_SET_INTB(R_BSP_SECTOP_INTVECTTBL);

#ifdef BSP_MCU_EXCEPTION_TABLE
    /* Initialize the Exception Table Register */
    R_BSP_SET_EXTB(R_BSP_SECTOP_EXCEPTVECTTBL);
#endif

#ifdef BSP_MCU_FLOATING_POINT
#ifdef __FPU
    /* Initialize the Floating-Point Status Word Register. */
    R_BSP_SET_FPSW(BSP_PRV_FPSW_INIT | BSP_PRV_FPU_ROUND | BSP_PRV_FPU_DENOM);
#endif
#endif

#ifdef BSP_MCU_DOUBLE_PRECISION_FLOATING_POINT
#ifdef __DPFPU
    /* Initialize the Double-Precision Floating-Point Status Word Register. */
    R_BSP_SET_DPSW(BSP_PRV_DPSW_INIT | BSP_PRV_FPU_ROUND | BSP_PRV_FPU_DENOM);
#endif
#endif

#ifdef BSP_MCU_TRIGONOMETRIC
#ifdef __TFU
    R_BSP_INIT_TFU();
#endif
#endif

#endif /* defined(__CCRX__), defined(__GNUC__) */

    /* Switch to high-speed operation */ 
    mcu_clock_setup();

    /* If the warm start Pre C runtime callback is enabled, then call it. */
#if BSP_CFG_USER_WARM_START_CALLBACK_PRE_INITC_ENABLED == 1
    BSP_CFG_USER_WARM_START_PRE_C_FUNCTION();
#endif

    /* Initialize C runtime environment */
    _INITSCT();

#if defined(CPPAPP)
    /* Initialize C++ global class object */
    _CALL_INIT();
#endif

    /* Initialize RAM */
    bsp_ram_initialize();

    /* If the warm start Post C runtime callback is enabled, then call it. */
#if BSP_CFG_USER_WARM_START_CALLBACK_POST_INITC_ENABLED == 1
    BSP_CFG_USER_WARM_START_POST_C_FUNCTION();
#endif

#if BSP_CFG_IO_LIB_ENABLE == 1
    /* Comment this out if not using I/O lib */
#if defined(__CCRX__)
    init_iolib();
#endif /* defined(__CCRX__) */
#endif

    /* Initialize MCU interrupt callbacks. */
    bsp_interrupt_open();

    /* Initialize register protection functionality. */
    bsp_register_protect_open();

    /* Configure the MCU and board hardware */
    hardware_setup();

    /* Enable interrupt and select the I stack or the U stack */
    R_BSP_SET_PSW(BSP_PRV_PSW_INIT);

#if BSP_CFG_RUN_IN_USER_MODE == 1
    /* Change the MCU's user mode from supervisor to user */
    #if BSP_CFG_USER_STACK_ENABLE == 1
        R_BSP_CHG_PMUSR();
    #else
        #error "Settings of BSP_CFG_RUN_IN_USER_MODE and BSP_CFG_USER_STACK_ENABLE are inconsistent with each other."
    #endif
#endif

    /* Enable the bus error interrupt to catch accesses to illegal/reserved areas of memory */
    R_BSP_InterruptControl(BSP_INT_SRC_BUS_ERROR, BSP_INT_CMD_INTERRUPT_ENABLE, FIT_NO_PTR);

#if BSP_CFG_RTOS_USED == 0    /* Non-OS */
    /* Call the main program function (should not return) */
    R_BSP_MAIN_FUNCTION();
#elif BSP_CFG_RTOS_USED == 1    /* FreeRTOS */
    /* Lock the channel that system timer of RTOS is using. */
    #if (((BSP_CFG_RTOS_SYSTEM_TIMER) >=0) && ((BSP_CFG_RTOS_SYSTEM_TIMER) <= 3))
        if (R_BSP_HardwareLock((mcu_lock_t)(BSP_LOCK_CMT0 + BSP_CFG_RTOS_SYSTEM_TIMER)) == false)
        {
            /* WAIT_LOOP */
            while(1);
        }
    #else
        #error "Setting BSP_CFG_RTOS_SYSTEM_TIMER is invalid."
    #endif

    /* Prepare the necessary tasks, FreeRTOS's resources... required to be executed at the beginning
     * after vTaskStarScheduler() is called. Other tasks can also be created after starting scheduler at any time */
    Processing_Before_Start_Kernel();

    /* Call the kernel startup (should not return) */
    vTaskStartScheduler();
#elif BSP_CFG_RTOS_USED == 2    /* SEGGER embOS */
#elif BSP_CFG_RTOS_USED == 3    /* Micrium MicroC/OS */
#elif BSP_CFG_RTOS_USED == 4    /* Renesas RI600V4 & RI600PX */
#endif

#if BSP_CFG_IO_LIB_ENABLE == 1
    /* Comment this out if not using I/O lib - cleans up open files */
#if defined(__CCRX__)
    close_all();
#endif /* defined(__CCRX__) */
#endif

    /* Infinite loop is intended here. */
    /* WAIT_LOOP */
    while(1)
    {
        /* Infinite loop. Put a breakpoint here if you want to catch an exit of main(). */
        R_BSP_NOP();
    }
} /* End of function PowerON_Reset_PC() */

#endif /* BSP_CFG_STARTUP_DISABLE == 0 */

