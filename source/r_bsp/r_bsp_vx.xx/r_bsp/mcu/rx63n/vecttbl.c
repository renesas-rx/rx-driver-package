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
* File Name    : vecttbl.c
* Device(s)    : RX631/N
* Description  : Definition of the fixed vector table and user boot options.
***********************************************************************************************************************/
/***********************************************************************************************************************
* History : DD.MM.YYYY Version  Description
*         : 26.10.2011 1.00     First Release
*         : 17.02.2012 1.10     Made function names compliant with CS v4.0
*         : 13.03.2012 1.20     ID Code is now specified in r_bsp_config.h. It is still used here in Fixed_Vectors[].
*         : 26.06.2012 1.30     Brought in arrays from flash_options.c into here because they were in the same space.
*                               Also added optional callbacks to the ISRs. Now using sections instead of defining the
*                               address for a specific array. Added macros to define the OFS registers. Added option
*                               to choose endian for User App and User Boot. Add macro to set ROM Code Protection.
*         : 16.07.2012 1.40     Added code to handle exception interrupts better.
*         : 19.11.2012 1.50     Updated code to use 'BSP_' and 'BSP_CFG_' prefix for macros.
*         : 06.05.2013 1.60     Code auto detects endian from compiler macros now.
*         : 24.06.2013 1.70     Code now uses functions from mcu_interrupts.c for exception callbacks. All interrupts
*                               that map to NMI vector are now supported. Fixed bug in FPU flag clearing.
*         : 27.07.2018 1.71     Deleted the static analysis tool comment.
*         : xx.xx.xxxx 2.00     Deleted exception functions.
*                               (Exception functions moved to the common file (interrupts.c).)
*                               Added support for GNUC and ICCRX.
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
/* BSP configuration. */
#include "platform.h"

/* When using the user startup program, disable the following code. */
#if (BSP_CFG_STARTUP_DISABLE == 0)

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Exported global functions (to be accessed by other files)
***********************************************************************************************************************/
R_BSP_POR_FUNCTION(R_BSP_POWER_ON_RESET_FUNCTION);
R_BSP_UB_POR_FUNCTION(R_BSP_UB_POWER_ON_RESET_FUNCTION);

#ifdef BSP_MCU_EXCEP_SUPERVISOR_INST_ISR
R_BSP_PRAGMA_INTERRUPT_FUNCTION(excep_supervisor_inst_isr)
#endif
#ifdef BSP_MCU_EXCEP_ACCESS_ISR
R_BSP_PRAGMA_INTERRUPT_FUNCTION(excep_access_isr)
#endif
#ifdef BSP_MCU_EXCEP_UNDEFINED_INST_ISR
R_BSP_PRAGMA_INTERRUPT_FUNCTION(excep_undefined_inst_isr)
#endif
#ifdef BSP_MCU_EXCEP_FLOATING_POINT_ISR
R_BSP_PRAGMA_INTERRUPT_FUNCTION(excep_floating_point_isr)
#endif
#ifdef BSP_MCU_NON_MASKABLE_ISR
R_BSP_PRAGMA_INTERRUPT_FUNCTION(non_maskable_isr)
#endif
#ifdef BSP_MCU_UNDEFINED_INTERRUPT_SOURCE_ISR
R_BSP_PRAGMA_INTERRUPT_DEFAULT(undefined_interrupt_source_isr)
#endif
#ifdef BSP_MCU_BUS_ERROR_ISR
R_BSP_PRAGMA_INTERRUPT(bus_error_isr, VECT(BSC,BUSERR))
#endif

/***********************************************************************************************************************
* The following array fills in the UB codes to get into User Boot Mode, the MDEB register, and the User Boot reset
* vector.
***********************************************************************************************************************/
#ifdef __BIG
    #define MDE_VALUE (0xfffffff8)    /* big */
#else
    #define MDE_VALUE (0xffffffff)    /* little */
#endif

/* The UB Code A, UB Code B, and Endian select register B (MDEB) are located in the User Boot space. Immediately
   following the MDEB register is the User Boot Reset Vector so it is defined below as well. These settings will only
   be used when the MCU is reset in User Boot Mode. In order for the MCU to start up in User Boot Mode the following
   conditions must be met:
   1) UB code A is 55736572h and 426F6F74h.
   2) UB code B is FFFF FF07h and 0008 C04Ch.
   3) The low level is being input on the MD pin.
   4) The high level is being input on the PC7 pin. 
   Please see the Option-Setting Memory section of your MCU's HW manual for more information. */

/* 0xFF7FFFE8 - 0xFF7FFFEF : UB Code A register  
   0xFF7FFFF0 - 0xFF7FFFF7 : UB Code B register
   0xFF7FFFF8 - 0xFF7FFFFB : MDEB register
   0xFF7FFFFC - 0xFF7FFFFF : User Boot Reset Vector */

#if BSP_CFG_USER_BOOT_ENABLE == 1

/* Use this array if you are using User Boot. Make sure to fill in valid address for UB Reset Vector. */
/* Allocate this space in the user boot sector. */
R_BSP_ATTRIB_SECTION_CHANGE_UBSETTINGS const uint32_t user_boot_settings[6] =
{
    0x55736572,                 //Required setting for UB Code A to get into User Boot
    0x426f6f74,                 //Required setting for UB Code A to get into User Boot
    0xffffff07,                 //Required setting for UB Code B to get into User Boot
    0x0008c04c,                 //Required setting for UB Code B to get into User Boot
    MDE_VALUE,
    (uint32_t) R_BSP_UB_POWER_ON_RESET_FUNCTION //This is the User Boot Reset Vector. When using User Boot put in the reset address here
};
R_BSP_ATTRIB_SECTION_CHANGE_END

#endif /* BSP_CFG_USER_BOOT_ENABLE == 1 */

#if defined(__ICCRX__)

#pragma public_equ = "__MDES", MDE_VALUE
#pragma public_equ = "__OFS1", BSP_CFG_OFS1_REG_VALUE
#pragma public_equ = "__OFS0", BSP_CFG_OFS0_REG_VALUE
#pragma public_equ = "__ROM_CODE", BSP_CFG_ROM_CODE_PROTECT_VALUE
#pragma public_equ = "__ID_BYTES_1_4", BSP_CFG_ID_CODE_LONG_1
#pragma public_equ = "__ID_BYTES_5_8", BSP_CFG_ID_CODE_LONG_2
#pragma public_equ = "__ID_BYTES_9_12", BSP_CFG_ID_CODE_LONG_3
#pragma public_equ = "__ID_BYTES_13_16", BSP_CFG_ID_CODE_LONG_4

#endif /* defined(__ICCRX__) */

/***********************************************************************************************************************
* The following array fills in the option function select registers, fixed vector table, and the ID code protection 
* bytes.
***********************************************************************************************************************/
#if defined(__CCRX__) || defined(__GNUC__)
R_BSP_ATTRIB_SECTION_CHANGE_FIXEDVECT void * const Fixed_Vectors[] =
{
    /* The Endian select register S (MDES), Option function select register 1 (OFS1), and Option function select 
       register 0 (OFS0) are located in User ROM. */
    
    /* 0xFFFFFF80 - 0xFFFFFF83 : MDES register
       0xFFFFFF84 - 0xFFFFFF87 : Reserved space (0xFF's)
       0xFFFFFF88 - 0xFFFFFF8B : OFS1 register
       0xFFFFFF8C - 0xFFFFFF8F : OFS0 register */

    (void *)MDE_VALUE,      //Endian
    (void *)0xFFFFFFFF,     //Reserved space
    /* Configure whether voltage detection 0 circuit and HOCO are enabled after reset. 
       OFS1 - Option Function Select Register 1 
       b31:b9 Reserved (set to 1)
       b8     HOCOEN - Enable/disable HOCO oscillation after a reset (0=enable, 1=disable)
       b7:b3  Reserved (set to 1)
       b2     LVDAS - Choose to enable/disable Voltage Detection 0 Circuit after a reset (0=enable, 1=disable)
       b1:b0  Reserved (set to 1) */
    (void *)BSP_CFG_OFS1_REG_VALUE,     //Defined in r_bsp_config.h
    /* Configure WDT and IWDT settings. 
       OFS0 - Option Function Select Register 0 
       b31:b29 Reserved (set to 1)
       b28     WDTRSTIRQS - WDT Reset Interrupt Request - What to do on underflow (0=take interrupt, 1=reset MCU)
       b27:b26 WDTRPSS - WDT Window Start Position Select - (0=25%, 1=50%, 2=75%, 3=100%,don't use)
       b25:b24 WDTRPES - WDT Window End Position Select - (0=75%, 1=50%, 2=25%, 3=0%,don't use)
       b23:b20 WDTCKS - WDT Clock Frequency Division Ratio - (1=/4, 4=/64, 0xF=/128, 6=/512, 7=/2048, 8=/8192)
       b19:b18 WDTTOPS - WDT Timeout Period Select - (0=1024 cycles, 1=4096, 2=8192, 3=16384)
       b17     WDTSTRT - WDT Start Mode Select - (0=auto-start after reset, halt after reset)
       b16:b15 Reserved (set to 1)
       b14     IWDTSLCSTP - IWDT Sleep Mode Count Stop Control - (0=can't stop count, 1=stop w/some low power modes)
       b13     Reserved (set to 1)
       b12     IWDTRSTIRQS - IWDT Reset Interrupt Request - What to do on underflow (0=take interrupt, 1=reset MCU)
       b11:b10 IWDTRPSS - IWDT Window Start Position Select - (0=25%, 1=50%, 2=75%, 3=100%,don't use)
       b9:b8   IWDTRPES - IWDT Window End Position Select - (0=75%, 1=50%, 2=25%, 3=0%,don't use)
       b7:b4   IWDTCKS - IWDT Clock Frequency Division Ratio - (0=none, 2=/16, 3 = /32, 4=/64, 0xF=/128, 5=/256)
       b3:b2   IWDTTOPS - IWDT Timeout Period Select - (0=1024 cycles, 1=4096, 2=8192, 3=16384)
       b1      IWDTSTRT - IWDT Start Mode Select - (0=auto-start after reset, halt after reset)
       b0      Reserved (set to 1) */
    (void *)BSP_CFG_OFS0_REG_VALUE,

    /* 0xffffff90 through 0xffffff9f: Reserved area - must be all 0xFF */
    (void *)0xFFFFFFFF,   /* 0xffffff90 - Reserved */
    (void *)0xFFFFFFFF,   /* 0xffffff94 - Reserved */
    (void *)0xFFFFFFFF,   /* 0xffffff98 - Reserved */

    /* The 32-bit area immediately below (0xffffff9c through 0xffffff9f) is a special area that allows the ROM to be 
       protected from reading or writing by a parallel programmer. Please refer to the HW manual for appropriate 
       settings. The default (all 0xff) places no restrictions and therefore allows reads and writes by a parallel 
       programmer. */
    (void *)BSP_CFG_ROM_CODE_PROTECT_VALUE,   /* 0xffffff9C - ROM Code Protection */

    /* The memory are immediately below (0xffffffa0 through 0xffffffaf) is a special area that allows the on-chip 
       firmware to be protected. See the section "ID Code Protection" in the HW manual for details on how to enable 
       protection. Setting the four long words below to non-0xFF values will enable protection.  Do this only after 
       carefully review the HW manual */
   
    /* 0xffffffA0 through 0xffffffaf: ID Code Protection. The ID code is specified using macros in r_bsp_config.h.  */
    (void *) BSP_CFG_ID_CODE_LONG_1,  /* 0xffffffA0 - Control code and ID code */
    (void *) BSP_CFG_ID_CODE_LONG_2,  /* 0xffffffA4 - ID code (cont.) */
    (void *) BSP_CFG_ID_CODE_LONG_3,  /* 0xffffffA8 - ID code (cont.) */
    (void *) BSP_CFG_ID_CODE_LONG_4,  /* 0xffffffAC - ID code (cont.) */
  
    /* 0xffffffB0 through 0xffffffcf: Reserved area */
    (void *) 0xFFFFFFFF,  /* 0xffffffB0 - Reserved */
    (void *) 0xFFFFFFFF,  /* 0xffffffB4 - Reserved */
    (void *) 0xFFFFFFFF,  /* 0xffffffB8 - Reserved */
    (void *) 0xFFFFFFFF,  /* 0xffffffBC - Reserved */
    (void *) 0xFFFFFFFF,  /* 0xffffffC0 - Reserved */
    (void *) 0xFFFFFFFF,  /* 0xffffffC4 - Reserved */
    (void *) 0xFFFFFFFF,  /* 0xffffffC8 - Reserved */
    (void *) 0xFFFFFFFF,  /* 0xffffffCC - Reserved */

    /* Fixed vector table */
    (void *) excep_supervisor_inst_isr,         /* 0xffffffd0  Exception(Supervisor Instruction) */
    (void *) undefined_interrupt_source_isr,    /* 0xffffffd4  Reserved */
    (void *) undefined_interrupt_source_isr,    /* 0xffffffd8  Reserved */
    (void *) excep_undefined_inst_isr,          /* 0xffffffdc  Exception(Undefined Instruction) */
    (void *) undefined_interrupt_source_isr,    /* 0xffffffe0  Reserved */
    (void *) excep_floating_point_isr,          /* 0xffffffe4  Exception(Floating Point) */
    (void *) undefined_interrupt_source_isr,    /* 0xffffffe8  Reserved */
    (void *) undefined_interrupt_source_isr,    /* 0xffffffec  Reserved */
    (void *) undefined_interrupt_source_isr,    /* 0xfffffff0  Reserved */
    (void *) undefined_interrupt_source_isr,    /* 0xfffffff4  Reserved */
    (void *) non_maskable_isr,                  /* 0xfffffff8  NMI */
    (void *) R_BSP_POWER_ON_RESET_FUNCTION      /* 0xfffffffc  RESET */
};
R_BSP_ATTRIB_SECTION_CHANGE_END
#endif /* defined(__CCRX__), defined(__GNUC__) */

#endif /* BSP_CFG_STARTUP_DISABLE == 0 */

