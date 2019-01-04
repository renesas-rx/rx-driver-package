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
* Copyright (C) 2015-2017 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : r_glcdc_config.h
* Description  : sample program of TSIP main program.
***********************************************************************************************************************/
/***********************************************************************************************************************
* History : DD.MM.YYYY Version Description
*         : 04.01.2019 0.80    First Release
***********************************************************************************************************************/

#include <stdio.h>
#include <string.h>

#include "platform.h"

#define LCD_X_INITCONTROLLER 0x01 /* Initializing the display controller */
#define LCD_X_SETLUTENTRY    0x04 /* Setting an entry of the LUT */
#define LCD_X_ON             0x05 /* Switching the display on */

#define XSIZE_PHYS 480
#define YSIZE_PHYS 272

//
// Timing
//
#define SYNC_H       4
#define SYNC_V       1
#define BGSYNC_HP    2
#define BGSYNC_VP    2
#define BGHSIZE_HP  (BGSYNC_HP + SYNC_H)
#define BGVSIZE_VP  (BGSYNC_VP + SYNC_V)
#define GRC_VS      (BGVSIZE_VP - BGSYNC_VP)
#define GRC_HS      (BGHSIZE_HP - BGSYNC_HP)

// Color depth
//
#define BITS_PER_PIXEL 8  // Allowed values: 1, 4, 8, 16, 32

//
// Frame buffer
//
#define FRAMEBUFFER_START 0x800000  // Begin of Expansion RAM

//
// Buffer size and stride
//
#define BYTES_PER_LINE   ((BITS_PER_PIXEL * XSIZE_PHYS) / 8)
#define LINE_OFFSET      (((BYTES_PER_LINE + 63) / 64) * 64)
#define VXSIZE_PHYS      ((LINE_OFFSET * 8) / BITS_PER_PIXEL)
#define BYTES_PER_BUFFER (LINE_OFFSET * YSIZE_PHYS)

//
// Brightness & COntrast
//
#define _BRIGHTNESS 0x200
#define _CONTRAST   0x80

void R_GLCDC_CONFIG_Open(void);
void R_GLCDC_CONFIG_Close(void);
uint8_t * lcd_get_frame_buffer_pointer(void);

static void _VSYNC_ISR(void * p);
static void _DisplayOnOff(int OnOff);
static void LCD_shutdownController(void);

