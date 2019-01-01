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
* File Name    : vecttbl.c
* Device(s)    : RX65N
* Description  : Definition of the exception vector table, reset vector, and user boot options.
***********************************************************************************************************************/
/***********************************************************************************************************************
* History : DD.MM.YYYY Version   Description
*         : 01.10.2016 1.00      First Release
*         : 15.05.2017 2.00      Deleted unnecessary comments.
*                                Added the following setting.
*                                - Setting of Expansion RAM.
*                                - Setting of Bank Mode Select.
*                                - Setting of Startup Bank Switch.
*                                Added the bsp startup module disable function.
*         : 27.07.2018 2.01      Modified the comment of START_BANK_VALUE.
*                                Changed the setting of ID code protection.
*         : xx.xx.xxxx 3.00      Deleted exception functions.
*                                (Exception functions moved to the common file (interrupts.c).)
*                                Added support for GNUC and ICCRX.
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

#if defined(BSP_MCU_RX65N_2MB) /* In the case of 1.5 or 2Mbyte ROM capacity. */
    #if (BSP_CFG_CODE_FLASH_BANK_MODE == 0)
        #define BANK_MODE_VALUE (0xffffff8f)    /* dual */
    #else
        #define BANK_MODE_VALUE (0xffffffff)    /* linear */
    #endif
#else
    #define BANK_MODE_VALUE (0xffffffff)    /* linear */
#endif

#if defined(BSP_MCU_RX65N_2MB) /* In the case of 1.5 or 2Mbyte ROM capacity. */
    #if (BSP_CFG_CODE_FLASH_START_BANK == 0)
        /* The address range of bank 1 from FFE00000h to FFEFFFFFh and bank 0 from FFF00000h to FFFFFFFFh. */
        #define START_BANK_VALUE (0xffffffff)
    #else
        /* The address range of bank 1 from FFF00000h to FFFFFFFFh and bank 0 from FFE00000h to FFEFFFFFh. */
        #define START_BANK_VALUE (0xfffffff8)
    #endif
#endif

#if defined(__CCRX__)

#pragma address __MDEreg     = 0xFE7F5D00
#pragma address __OFS0reg    = 0xFE7F5D04
#pragma address __OFS1reg    = 0xFE7F5D08
#pragma address __TMINFreg   = 0xFE7F5D10
#if defined(BSP_MCU_RX65N_2MB)
#pragma address __BANKSELreg = 0xFE7F5D20
#endif
#pragma address __SPCCreg    = 0xFE7F5D40
#pragma address __TMEFreg    = 0xFE7F5D48
#pragma address __OSIS1reg   = 0xFE7F5D50
#pragma address __OSIS2reg   = 0xFE7F5D54
#pragma address __OSIS3reg   = 0xFE7F5D58
#pragma address __OSIS4reg   = 0xFE7F5D5C
#pragma address __FAWreg     = 0xFE7F5D64
#pragma address __ROMCODEreg = 0xFE7F5D70

const unsigned long __MDEreg     = (MDE_VALUE & BANK_MODE_VALUE);
const unsigned long __OFS0reg    = BSP_CFG_OFS0_REG_VALUE;
const unsigned long __OFS1reg    = BSP_CFG_OFS1_REG_VALUE;
const unsigned long __TMINFreg   = 0xffffffff;
#if defined(BSP_MCU_RX65N_2MB)
const unsigned long __BANKSELreg = START_BANK_VALUE;
#endif
const unsigned long __SPCCreg    = 0xffffffff;
const unsigned long __TMEFreg    = BSP_CFG_TRUSTED_MODE_FUNCTION;
const unsigned long __OSIS1reg   = BSP_CFG_ID_CODE_LONG_1;
const unsigned long __OSIS2reg   = BSP_CFG_ID_CODE_LONG_2;
const unsigned long __OSIS3reg   = BSP_CFG_ID_CODE_LONG_3;
const unsigned long __OSIS4reg   = BSP_CFG_ID_CODE_LONG_4;
const unsigned long __FAWreg     = BSP_CFG_FAW_REG_VALUE;
const unsigned long __ROMCODEreg = BSP_CFG_ROMCODE_REG_VALUE;

#elif defined(__GNUC__)

const unsigned long __MDEreg     __attribute__ ((section(".ofs1"))) = (MDE_VALUE & BANK_MODE_VALUE);
const unsigned long __OFS0reg    __attribute__ ((section(".ofs1"))) = BSP_CFG_OFS0_REG_VALUE;
const unsigned long __OFS1reg    __attribute__ ((section(".ofs1"))) = BSP_CFG_OFS1_REG_VALUE;
const unsigned long __TMINFreg   __attribute__ ((section(".ofs2"))) = 0xffffffff;
#if defined(BSP_MCU_RX65N_2MB)
//const unsigned long __BANKSELreg __attribute__ ((section(".ofs2"))) = START_BANK_VALUE;
#endif
const unsigned long __SPCCreg    __attribute__ ((section(".ofs3"))) = 0xffffffff;
const unsigned long __TMEFreg    __attribute__ ((section(".ofs4"))) = BSP_CFG_TRUSTED_MODE_FUNCTION;
const unsigned long __OSIS1reg   __attribute__ ((section(".ofs5"))) = BSP_CFG_ID_CODE_LONG_1;
const unsigned long __OSIS2reg   __attribute__ ((section(".ofs5"))) = BSP_CFG_ID_CODE_LONG_2;
const unsigned long __OSIS3reg   __attribute__ ((section(".ofs5"))) = BSP_CFG_ID_CODE_LONG_3;
const unsigned long __OSIS4reg   __attribute__ ((section(".ofs5"))) = BSP_CFG_ID_CODE_LONG_4;
const unsigned long __FAWreg     __attribute__ ((section(".ofs6"))) = BSP_CFG_FAW_REG_VALUE;
const unsigned long __ROMCODEreg __attribute__ ((section(".ofs7"))) = BSP_CFG_ROMCODE_REG_VALUE;

#elif defined(__ICCRX__)

#pragma public_equ = "__MDE", (MDE_VALUE & BANK_MODE_VALUE)
#pragma public_equ = "__OFS0", BSP_CFG_OFS0_REG_VALUE
#pragma public_equ = "__OFS1", BSP_CFG_OFS1_REG_VALUE
#pragma public_equ = "__TMINF", 0xffffffff
#if defined(BSP_MCU_RX65N_2MB)
#pragma public_equ = "__BANKSEL", START_BANK_VALUE
#endif
#pragma public_equ = "__SPCC", 0xffffffff
#pragma public_equ = "__TMEF", BSP_CFG_TRUSTED_MODE_FUNCTION
#pragma public_equ = "__OSIS_1", BSP_CFG_ID_CODE_LONG_1
#pragma public_equ = "__OSIS_2", BSP_CFG_ID_CODE_LONG_2
#pragma public_equ = "__OSIS_3", BSP_CFG_ID_CODE_LONG_3
#pragma public_equ = "__OSIS_4", BSP_CFG_ID_CODE_LONG_4
#pragma public_equ = "__FAW", BSP_CFG_FAW_REG_VALUE
#pragma public_equ = "__ROM_CODE", BSP_CFG_ROMCODE_REG_VALUE

#endif /* defined(__CCRX__), defined(__GNUC__), defined(__ICCRX__) */

/***********************************************************************************************************************
* The following array fills in the exception vector table.
***********************************************************************************************************************/
#if defined(__CCRX__) || defined(__GNUC__)
R_BSP_ATTRIB_SECTION_CHANGE_EXCEPTVECT void (* const Except_Vectors[])(void) =
{
    /* Offset from EXTB: Reserved area - must be all 0xFF */
    (void (*)(void))0xFFFFFFFF,  /* 0x00 - Reserved */
    (void (*)(void))0xFFFFFFFF,  /* 0x04 - Reserved */
    (void (*)(void))0xFFFFFFFF,  /* 0x08 - Reserved */
    (void (*)(void))0xFFFFFFFF,  /* 0x0c - Reserved */
    (void (*)(void))0xFFFFFFFF,  /* 0x10 - Reserved */
    (void (*)(void))0xFFFFFFFF,  /* 0x14 - Reserved */
    (void (*)(void))0xFFFFFFFF,  /* 0x18 - Reserved */
    (void (*)(void))0xFFFFFFFF,  /* 0x1c - Reserved */
    (void (*)(void))0xFFFFFFFF,  /* 0x20 - Reserved */
    (void (*)(void))0xFFFFFFFF,  /* 0x24 - Reserved */
    (void (*)(void))0xFFFFFFFF,  /* 0x28 - Reserved */
    (void (*)(void))0xFFFFFFFF,  /* 0x2c - Reserved */
    (void (*)(void))0xFFFFFFFF,  /* 0x30 - Reserved */
    (void (*)(void))0xFFFFFFFF,  /* 0x34 - Reserved */
    (void (*)(void))0xFFFFFFFF,  /* 0x38 - Reserved */
    (void (*)(void))0xFFFFFFFF,  /* 0x3c - Reserved */
    (void (*)(void))0xFFFFFFFF,  /* 0x40 - Reserved */
    (void (*)(void))0xFFFFFFFF,  /* 0x44 - Reserved */
    (void (*)(void))0xFFFFFFFF,  /* 0x48 - Reserved */
    (void (*)(void))0xFFFFFFFF,  /* 0x4c - Reserved */

    /* Exception vector table */
    excep_supervisor_inst_isr,         /* 0x50  Exception(Supervisor Instruction) */
    excep_access_isr,                  /* 0x54  Exception(Access exception) */
    undefined_interrupt_source_isr,    /* 0x58  Reserved */
    excep_undefined_inst_isr,          /* 0x5c  Exception(Undefined Instruction) */
    undefined_interrupt_source_isr,    /* 0x60  Reserved */
    excep_floating_point_isr,          /* 0x64  Exception(Floating Point) */
    undefined_interrupt_source_isr,    /* 0x68  Reserved */
    undefined_interrupt_source_isr,    /* 0x6c  Reserved */
    undefined_interrupt_source_isr,    /* 0x70  Reserved */
    undefined_interrupt_source_isr,    /* 0x74  Reserved */
    non_maskable_isr,                  /* 0x78  NMI */
};
R_BSP_ATTRIB_SECTION_CHANGE_END
#endif /* defined(__CCRX__), defined(__GNUC__) */

/***********************************************************************************************************************
* The following array fills in the reset vector.
***********************************************************************************************************************/
#if defined(__CCRX__) || defined(__GNUC__)
R_BSP_ATTRIB_SECTION_CHANGE_RESETVECT void (* const Reset_Vector[])(void) =
{
    R_BSP_POWER_ON_RESET_FUNCTION                   /* 0xfffffffc  RESET */
};
R_BSP_ATTRIB_SECTION_CHANGE_END
#endif /* defined(__CCRX__), defined(__GNUC__) */

#endif /* BSP_CFG_STARTUP_DISABLE == 0 */
