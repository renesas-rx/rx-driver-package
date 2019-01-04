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
* Copyright (C) 2015-2018 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : main.c
* Description  : sample program of TSIP main program.
***********************************************************************************************************************/
/***********************************************************************************************************************
* History : DD.MM.YYYY Version Description
*         : 26.08.2017 1.00    First Release
***********************************************************************************************************************/

#include <stdio.h>
#include <string.h>

#include "platform.h"
#include "r_glcdc_rx_if.h"
#include "r_simple_glcdc_config_rx_if.h"
#include "r_glcdc_rx_pinset.h"

#pragma section _FRAME_BUFFER
uint8_t frame_buffer[XSIZE_PHYS * YSIZE_PHYS * BITS_PER_PIXEL / 8];
#pragma section

void R_SIMPLE_GLCDC_CONFIG_Open(void)
{
	  glcdc_cfg_t glcdc_cfg;
	  glcdc_err_t ret;

	  R_GLCDC_PinSet();
	  //
	  // Set DISP signal on P97 (Pin31)
	  //
	  PORT6.PDR.BIT.B3  = 1;  // Port direction: output
	  PORT6.PODR.BIT.B3 = 1;  // DISP on
	  //
	  // Switch backlight on, no PWM
	  //
	  PORT6.PDR.BIT.B6  = 1;
	  PORT6.PODR.BIT.B6 = 1;
	  //
	  // Set the BGEN.SWRST bit to 1 to release the GLCDC from a software reset
	  //
	  // R_GLCDC_Open function release the GLCDC from a software reset.
	  //
	  // Set the frequency of the LCD_CLK and PXCLK to suit the format and set the PANELCLK.CLKEN bit to 1
	  //
	  glcdc_cfg.output.clksrc = GLCDC_CLK_SRC_INTERNAL;   			  // Select PLL clock
	  glcdc_cfg.output.clock_div_ratio = GLCDC_PANEL_CLK_DIVISOR_24;  // 240 / 24 = 10 MHz
	  																  // No frequency division
	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  // Enable LCD_CLK output
	  //
	  // Definition of Background Screen
	  //
	  										   	   	   	   	   	   	  // Horizontal cycle (whole control screen)
	  	  	  	  	  	  	  	  	  	  	   	   	   	   	   	   	  // Vertical cycle (whole control screen)
	  glcdc_cfg.output.htiming.front_porch = 5;         			  // Horizontal Synchronization Signal Assertion Position
	  glcdc_cfg.output.vtiming.front_porch = 8;         			  // Vertical Synchronization Assertion Position
	  glcdc_cfg.output.htiming.back_porch = 40;                        // Horizontal Active Pixel Start Position (min. 6 pixels)
	  glcdc_cfg.output.vtiming.back_porch = 8;
	  glcdc_cfg.output.htiming.display_cyc = XSIZE_PHYS;              // Horizontal Active Pixel Width
	  glcdc_cfg.output.vtiming.display_cyc = YSIZE_PHYS;              // Vertical Active Display Width
	  glcdc_cfg.output.htiming.sync_width = 1;        				  // Vertical Active Display Start Position (min. 3 lines)
	  glcdc_cfg.output.vtiming.sync_width = 1;

	  //
	  // Graphic 1 configuration
	  //
	  glcdc_cfg.input[GLCDC_FRAME_LAYER_1].p_base = NULL;			  // Disable Graphics 1

	  //
	  // Graphic 2 configuration
	  //
	  													  	  	  	  // Enable reading of the frame buffer
	  glcdc_cfg.input[GLCDC_FRAME_LAYER_2].p_base = frame_buffer;   // Specify the start address of the frame buffer
	  glcdc_cfg.input[GLCDC_FRAME_LAYER_2].offset = LINE_OFFSET;      // Offset value from the end address of the line to the start address of the next line
	  																  // Single Line Data Transfer Count
	  																  // Single Frame Line Count
	  glcdc_cfg.input[GLCDC_FRAME_LAYER_2].format = GLCDC_IN_FORMAT_CLUT8;     // Frame Buffer
	  glcdc_cfg.blend[GLCDC_FRAME_LAYER_2].visible = true;
	  glcdc_cfg.blend[GLCDC_FRAME_LAYER_2].blend_control = GLCDC_BLEND_CONTROL_NONE;	// Display Screen Control (current graphics)
	  glcdc_cfg.blend[GLCDC_FRAME_LAYER_2].frame_edge = false;        // Rectangular Alpha Blending Area Frame Display Control
	  glcdc_cfg.input[GLCDC_FRAME_LAYER_2].frame_edge = false;        // Graphics Area Frame Display Control
	  																  // Alpha Blending Control (Per-pixel alpha blending)
	  glcdc_cfg.input[GLCDC_FRAME_LAYER_2].coordinate.y = 0;          // Graphics Area Vertical Start Position
	  glcdc_cfg.input[GLCDC_FRAME_LAYER_2].vsize = YSIZE_PHYS;        // Graphics Area Vertical Width
	  glcdc_cfg.input[GLCDC_FRAME_LAYER_2].coordinate.x = 0;          // Graphics Area Horizontal Start Position
	  glcdc_cfg.input[GLCDC_FRAME_LAYER_2].hsize = XSIZE_PHYS;        // Graphics Area Horizontal Width
	  glcdc_cfg.blend[GLCDC_FRAME_LAYER_2].start_coordinate.x = 0;    // Rectangular Alpha Blending Area Vertical Start Position
	  glcdc_cfg.blend[GLCDC_FRAME_LAYER_2].end_coordinate.x= YSIZE_PHYS; // Rectangular Alpha Blending Area Vertical Width
	  glcdc_cfg.blend[GLCDC_FRAME_LAYER_2].start_coordinate.y = 0;    // Rectangular Alpha Blending Area Horizontal Start Position
	  glcdc_cfg.blend[GLCDC_FRAME_LAYER_2].end_coordinate.y= XSIZE_PHYS; // Rectangular Alpha Blending Area Horizontal Width
	  																  // Graphic 2 Register Value Reflection Enable
	  //
	  // Timing configuration
	  //
	  //   Horizontal Synchronization Signal Reference Timing Offset (not support)
	  //   Signal Reference Timing Select (not support)
	  //   STVB Signal Assertion Timing
	  //   STVB Signal Pulse Width
	  //   STHB Signal Pulse Width
	  glcdc_cfg.output.tcon_vsync = GLCDC_TCON_PIN_0;           // TCON0 Output Signal Select STVA (VSYNC)
	  glcdc_cfg.output.tcon_hsync = GLCDC_TCON_PIN_2;           // TCON2 Output Signal Select STHA (HSYNC)
	  glcdc_cfg.output.tcon_de    = GLCDC_TCON_PIN_3;           // TCON3 Output Signal Select DE (DEN)
	  glcdc_cfg.output.data_enable_polarity = GLCDC_SIGNAL_POLARITY_HIACTIVE;
	  glcdc_cfg.output.hsync_polarity = GLCDC_SIGNAL_POLARITY_LOACTIVE;
	  glcdc_cfg.output.vsync_polarity = GLCDC_SIGNAL_POLARITY_LOACTIVE;
	  glcdc_cfg.output.sync_edge = GLCDC_SIGNAL_SYNC_EDGE_RISING;
	  //
	  // Output interface
	  //
	  //   Serial RGB Data Output Delay Control (0 cycles) (not support)
	  //   Serial RGB Scan Direction Select (forward) (not support)
	  //   Pixel Clock Division Control (no division)
	  glcdc_cfg.output.format = GLCDC_OUT_FORMAT_16BITS_RGB565; // Output Data Format Select (RGB565)
	  glcdc_cfg.output.color_order = GLCDC_COLOR_ORDER_RGB; ///**/GLCDC_COLOR_ORDER_BGR;  	// Pixel Order Control (B-G-R)
	  glcdc_cfg.output.endian = GLCDC_ENDIAN_LITTLE;  			// Bit Endian Control (Little endian)
	  //
	  // Brightness Adjustment
	  //
	  glcdc_cfg.output.brightness.b = _BRIGHTNESS;  // B
	  glcdc_cfg.output.brightness.g = _BRIGHTNESS;  // G
	  glcdc_cfg.output.brightness.r = _BRIGHTNESS;  // R
	  //
	  // Contrast Adjustment Value
	  //
	  glcdc_cfg.output.contrast.b = _CONTRAST;  // B
	  glcdc_cfg.output.contrast.g = _CONTRAST;  // G
	  glcdc_cfg.output.contrast.r = _CONTRAST;  // R
	  //
	  // Disable Gamma
	  //
	  glcdc_cfg.output.gamma.enable = false;
	  //
	  // Disable Chromakey
	  //
	  glcdc_cfg.chromakey[GLCDC_FRAME_LAYER_2].enable = false;
	  //
	  // Disable Dithering
	  //
	  glcdc_cfg.output.dithering.dithering_on = false;
	  //
	  // CLUT Adjustment Value
	  //
	  glcdc_cfg.clut[GLCDC_FRAME_LAYER_2].enable = false;
	  //
	  // Enable VPOS ISR
	  //
	  //   Detecting Scanline Setting
	  glcdc_cfg.detection.vpos_detect = true;		         	// Enable detection of specified line notification in graphic 2
	  glcdc_cfg.interrupt.vpos_enable = true;		           	// Enable VPOS interrupt request
	  //   Interrupt Priority Level (r_glcdc_rx_config.h)
	  //   Interrupt Request Enable
	  //   Clears the STMON.VPOS flag
	  //   VPOS (line detection)
	  glcdc_cfg.detection.gr1uf_detect = false;
	  glcdc_cfg.detection.gr2uf_detect = false;
	  glcdc_cfg.interrupt.gr1uf_enable = false;
	  glcdc_cfg.interrupt.gr2uf_enable = false;
	  //
	  // Set function to be called on VSYNC
	  //
	  glcdc_cfg.p_callback = (void (*)(glcdc_callback_args_t *))_VSYNC_ISR;
	  //
	  // Register Reflection
	  //
	  ret = R_GLCDC_Open(&glcdc_cfg);
	  if (ret != GLCDC_SUCCESS) {
		  while(1);
	  }

	  memset(frame_buffer, 0, sizeof(frame_buffer));
	  _DisplayOnOff(1);
}

void R_SIMPLE_GLCDC_CONFIG_Close(void)
{
	_DisplayOnOff(0);
}

uint8_t *lcd_get_frame_buffer_pointer(void)
{
	return frame_buffer;
}

static void _VSYNC_ISR(void * p)
{
	GLCDC.STCLR.BIT.VPOSCLR = 1;                  // Clears the STMON.VPOS flag
}

static void _DisplayOnOff(int OnOff) {
  if (OnOff) {
    R_GLCDC_Control(GLCDC_CMD_START_DISPLAY, FIT_NO_FUNC);  // Enable background generation block
    PORT6.PODR.BIT.B6 = 1;
  } else {
	R_GLCDC_Control(GLCDC_CMD_STOP_DISPLAY, FIT_NO_FUNC);  	// Disable background generation block
    PORT6.PODR.BIT.B6 = 0;
  }
}
