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
* File Name    : r_lpc_rx_config.h
* Description  : Configures this code
************************************************************************************************************************
 * History : DD.MM.YYYY Version Description
 *         : 01.10.2016 1.00    First Release
***********************************************************************************************************************/
#ifndef LPC_CONFIG_HEADER_FILE
#define LPC_CONFIG_HEADER_FILE

/***********************************************************************************************************************
Configuration Options
***********************************************************************************************************************/
/* SPECIFY WHETHER TO INCLUDE CODE FOR API PARAMETER CHECKING */
/* Setting to BSP_CFG_PARAM_CHECKING_ENABLE utilizes the system default setting */
/* Setting to 1 includes parameter checking; 0 compiles out parameter checking */
#define LPC_CFG_PARAM_CHECKING_ENABLE       (BSP_CFG_PARAM_CHECKING_ENABLE)

#if defined(BSP_MCU_RX210)
/*
 * This configuration option is for the RX210 only.  Set LPC_CFG_SW_STANDBY_OPTIONS to one of the below options
[Chip version B]
b2 b0
0 0 0: Power is supplied to HOCO in software standby mode. The voltage detection circuit (LVD) is active and
       the low power consumption function by the power-on reset circuit (POR) is disabled.
0 1 x: Power is not supplied to HOCO in software standby mode. The voltage detection circuit (LVD) is active and
       the low power consumption function by the power-on reset circuit (POR) is disabled.
1 0 0: Power is supplied to HOCO in software standby mode. The voltage detection circuit (LVD) is stopped and
       the low power consumption function by the power-on reset circuit (POR) is enabled.
1 1 x: Power is not supplied to HOCO in software standby mode. The voltage detection circuit (LVD) is stopped and
       the low power consumption function by the power-on reset circuit (POR) is enabled.

[Chip versions A and C]
b2 b0
0 0 0: Power is supplied to flash memory and HOCO in software standby mode. The voltage detection circuit (LVD)
       is active and the low power consumption function by the power-on reset circuit (POR)is disabled.
0 1 0: Power is supplied to flash memory but not supplied to HOCO in software standby mode.
       The voltage detection circuit (LVD) is active and the low power consumption function by the power-on
       reset circuit (POR) is disabled.
0 1 1: Power is not supplied to flash memory or HOCO in software standby mode. The voltage detection circuit (LVD)
       is active and the low power consumption function by the power-on reset circuit (POR) is disabled.
1 0 0: Power is supplied to flash memory and HOCO in software standby mode. The voltage detection circuit (LVD)
       is stopped and the low power consumption function by the power-on reset circuit (POR) is enabled.
1 1 0: Power is supplied to flash memory but not supplied to HOCO in software standby mode.
       The voltage detection circuit (LVD) is stopped and the low power consumption function by the power-on
       reset circuit (POR) is enabled.
1 1 1: Power is not supplied to flash memory or HOCO in software standby mode.
       The voltage detection circuit (LVD) is stopped and the low power consumption function
       by the power-on reset circuit (POR) is enabled.
Settings other than above are prohibited.
 */
#define LPC_CFG_SW_STANDBY_OPTIONS (0x06)

#endif /* BSP_MCU_RX210 */

#endif /* LPC_CONFIG_HEADER_FILE */
