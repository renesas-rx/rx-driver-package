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
 * File Name    : r_lpc_rx210_private.h
 * Description  : Definitions used by the r_lpc_rx210.c module.
 ***********************************************************************************************************************/
/***********************************************************************************************************************
 * History : DD.MM.YYYY Version Description           
 *         : 01.10.2016 1.00    First Release
 ***********************************************************************************************************************/

#ifndef R_LPC_RX210_PRIVATE_H
    #define R_LPC_RX210_PRIVATE_H

/***********************************************************************************************************************
 Error checking for user defined macros
***********************************************************************************************************************/
#if ((LPC_CFG_SW_STANDBY_OPTIONS == 1) || (LPC_CFG_SW_STANDBY_OPTIONS == 5))
 #error "Illegal Software standby option for this chip version"
#endif

/***********************************************************************************************************************
 Macro definitions
 ***********************************************************************************************************************/
/* Internally used definitions */
    #define LPC_TRANSITION_ONGOING (1)
    #define LPC_TRANSITION_COMPLETE (0)

    #define LPC_OSC_STOP_ENABLED (1)

    #define LPC_SUB_CLOCK (0x0300)
    #define LPC_LOCO      (0x0000)

    #define LPC_CLOCK_SWITCH_ENABLE (0x01)
    #define LPC_CLOCK_SWITCH_DISABLE (0x00)

    #define LPC_CLOCK_INACTIVE    (0x01)
    #define LPC_CLOCK_ACTIVE      (0x00)

    #if (BSP_CFG_MCU_VCC_MV > 2699)
        #define LPC_MAX_BCK_HIGH_SPEED_FREQ_HZ    (25000000)
        #define LPC_MAX_FCK_HIGH_SPEED_FREQ_HZ    (32000000)
        #define LPC_MAX_ICK_HIGH_SPEED_FREQ_HZ    (50000000)
        #define LPC_MAX_PCKB_HIGH_SPEED_FREQ_HZ   (32000000)
        #define LPC_MAX_PCKD_HIGH_SPEED_FREQ_HZ   (50000000)
        
        #define LPC_MAX_BCK_MID1A_SPEED_FREQ_HZ   (25000000)
        #define LPC_MAX_FCK_MID1A_SPEED_FREQ_HZ   (32000000)
        #define LPC_MAX_ICK_MID1A_SPEED_FREQ_HZ   (32000000)
        #define LPC_MAX_PCKB_MID1A_SPEED_FREQ_HZ  (32000000)
        #define LPC_MAX_PCKD_MID1A_SPEED_FREQ_HZ  (32000000)
        
        #define LPC_MAX_BCK_MID1B_SPEED_FREQ_HZ   (25000000)
        #define LPC_MAX_FCK_MID1B_SPEED_FREQ_HZ   (32000000)
        #define LPC_MAX_ICK_MID1B_SPEED_FREQ_HZ   (32000000)
        #define LPC_MAX_PCKB_MID1B_SPEED_FREQ_HZ  (32000000)
        #define LPC_MAX_PCKD_MID1B_SPEED_FREQ_HZ  (32000000)
        
        #define LPC_MAX_BCK_LOW1_SPEED_FREQ_HZ    (8000000)
        #define LPC_MAX_FCK_LOW1_SPEED_FREQ_HZ    (8000000)
        #define LPC_MAX_ICK_LOW1_SPEED_FREQ_HZ    (8000000)
        #define LPC_MAX_PCKB_LOW1_SPEED_FREQ_HZ   (8000000)
        #define LPC_MAX_PCKD_LOW1_SPEED_FREQ_HZ   (8000000)
        
        #define LPC_MAX_BCK_LOW2_SPEED_FREQ_HZ    (32768)
        #define LPC_MAX_FCK_LOW2_SPEED_FREQ_HZ    (32768)
        #define LPC_MAX_ICK_LOW2_SPEED_FREQ_HZ    (32768)
        #define LPC_MAX_PCKB_LOW2_SPEED_FREQ_HZ   (32768)
        #define LPC_MAX_PCKD_LOW2_SPEED_FREQ_HZ   (32768)

    #elif (BSP_CFG_MCU_VCC_MV > 1799)
        #define LPC_MAX_BCK_HIGH_SPEED_FREQ_HZ    (0)
        #define LPC_MAX_FCK_HIGH_SPEED_FREQ_HZ    (0)
        #define LPC_MAX_ICK_HIGH_SPEED_FREQ_HZ    (0)
        #define LPC_MAX_PCKB_HIGH_SPEED_FREQ_HZ   (0)
        #define LPC_MAX_PCKD_HIGH_SPEED_FREQ_HZ   (0)
        
        #define LPC_MAX_BCK_MID1A_SPEED_FREQ_HZ   (25000000)
        #define LPC_MAX_FCK_MID1A_SPEED_FREQ_HZ   (32000000)
        #define LPC_MAX_ICK_MID1A_SPEED_FREQ_HZ   (32000000)
        #define LPC_MAX_PCKB_MID1A_SPEED_FREQ_HZ  (32000000)
        #define LPC_MAX_PCKD_MID1A_SPEED_FREQ_HZ  (32000000)
        
        #define LPC_MAX_BCK_MID1B_SPEED_FREQ_HZ   (25000000)
        #define LPC_MAX_FCK_MID1B_SPEED_FREQ_HZ   (32000000)
        #define LPC_MAX_ICK_MID1B_SPEED_FREQ_HZ   (32000000)
        #define LPC_MAX_PCKB_MID1B_SPEED_FREQ_HZ  (32000000)
        #define LPC_MAX_PCKD_MID1B_SPEED_FREQ_HZ  (32000000)
        
        #define LPC_MAX_BCK_LOW1_SPEED_FREQ_HZ    (4000000)
        #define LPC_MAX_FCK_LOW1_SPEED_FREQ_HZ    (4000000)
        #define LPC_MAX_ICK_LOW1_SPEED_FREQ_HZ    (4000000)
        #define LPC_MAX_PCKB_LOW1_SPEED_FREQ_HZ   (4000000)
        #define LPC_MAX_PCKD_LOW1_SPEED_FREQ_HZ   (4000000)
        
        #define LPC_MAX_BCK_LOW2_SPEED_FREQ_HZ    (32768)
        #define LPC_MAX_FCK_LOW2_SPEED_FREQ_HZ    (32768)
        #define LPC_MAX_ICK_LOW2_SPEED_FREQ_HZ    (32768)
        #define LPC_MAX_PCKB_LOW2_SPEED_FREQ_HZ   (32768)
        #define LPC_MAX_PCKD_LOW2_SPEED_FREQ_HZ   (32768)

    #elif (BSP_CFG_MCU_VCC_MV > 1619)
        #define LPC_MAX_BCK_HIGH_SPEED_FREQ_HZ    (0)
        #define LPC_MAX_FCK_HIGH_SPEED_FREQ_HZ    (0)
        #define LPC_MAX_ICK_HIGH_SPEED_FREQ_HZ    (0)
        #define LPC_MAX_PCKB_HIGH_SPEED_FREQ_HZ   (0)
        #define LPC_MAX_PCKD_HIGH_SPEED_FREQ_HZ   (0)
        
        #define LPC_MAX_BCK_MID1A_SPEED_FREQ_HZ   (20000000)
        #define LPC_MAX_FCK_MID1A_SPEED_FREQ_HZ   (20000000)
        #define LPC_MAX_ICK_MID1A_SPEED_FREQ_HZ   (20000000)
        #define LPC_MAX_PCKB_MID1A_SPEED_FREQ_HZ  (20000000)
        #define LPC_MAX_PCKD_MID1A_SPEED_FREQ_HZ  (20000000)
        
        #define LPC_MAX_BCK_MID1B_SPEED_FREQ_HZ   (20000000)
        #define LPC_MAX_FCK_MID1B_SPEED_FREQ_HZ   (20000000)
        #define LPC_MAX_ICK_MID1B_SPEED_FREQ_HZ   (20000000)
        #define LPC_MAX_PCKB_MID1B_SPEED_FREQ_HZ  (20000000)
        #define LPC_MAX_PCKD_MID1B_SPEED_FREQ_HZ  (20000000)
        
        #define LPC_MAX_BCK_LOW1_SPEED_FREQ_HZ    (2000000)
        #define LPC_MAX_FCK_LOW1_SPEED_FREQ_HZ    (2000000)
        #define LPC_MAX_ICK_LOW1_SPEED_FREQ_HZ    (2000000)
        #define LPC_MAX_PCKB_LOW1_SPEED_FREQ_HZ   (2000000)
        #define LPC_MAX_PCKD_LOW1_SPEED_FREQ_HZ   (2000000)
        
        #define LPC_MAX_BCK_LOW2_SPEED_FREQ_HZ    (32768)
        #define LPC_MAX_FCK_LOW2_SPEED_FREQ_HZ    (32768)
        #define LPC_MAX_ICK_LOW2_SPEED_FREQ_HZ    (32768)
        #define LPC_MAX_PCKB_LOW2_SPEED_FREQ_HZ   (32768)
        #define LPC_MAX_PCKD_LOW2_SPEED_FREQ_HZ   (32768)
    #endif

    #define LPC_MEDIUM_SPD (0x02)
    #define LPC_HIGH_SPD (0x00)

    #define LPC_CLOCK_INACTIVE                  (0x01)
    #define LPC_CLOCK_ACTIVE                    (0x00)
    
    #define LPC_ACS_CHK_MSTPCRA                 (0xFFFFFFCF)
    #define LPC_ACS_CHK_MSTPCRB                 (0xFFFFFFFF)
    #define LPC_ACS_CHK_MSTPCRC                 (0xFFFF0000)

    #define LPC_OFS0_REG_VALUE                  (*(volatile uint32_t *)0xFFFFFF8C)

/***********************************************************************************************************************
 Typedef definitions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 Exported global variables
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 Exported global functions (to be accessed by other files)
 ***********************************************************************************************************************/
#endif /* R_LPC_RX210_PRIVATE_H */
