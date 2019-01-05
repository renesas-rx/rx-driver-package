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
* Copyright (C) 2014-2016 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : r_t4_http_server_rx_if.h
* Version      : 1.04
* Description  : This is HTTP server function header file.
***********************************************************************************************************************/
/***********************************************************************************************************************
* History : DD.MM.YYYY Version Description
*         : 01.04.2011 1.00    First Release
*         : 09.05.2014 1.03    Corresponded to FIT Modules
*         : 09.05.2014 1.04    Clean up source code
*         : 01.04.2016 1.05    XML file update
***********************************************************************************************************************/
#ifndef _R_T4_HTTP_SERVER_RX_IF_H_
#define _R_T4_HTTP_SERVER_RX_IF_H_

/******************************************************************************
Macro definitions
******************************************************************************/
/* Version Number of API. */
#define T4_HTTP_SERVER_VERSION_MAJOR       (1)
#define T4_HTTP_SERVER_VERSION_MINOR       (05)

typedef struct sys_time_
{
    uint32_t sec;
    uint32_t min;
    uint32_t hour;
    uint32_t day;
    uint32_t month;
    uint32_t year;
}SYS_TIME;

typedef struct httpd_resource_type
{
    uint8_t *uri;        /* URI: Uniform Resource Identifier */
    uint8_t type;        /* 0: no type, 1: CGI, 2:TEXT */
    uint8_t *hdr;        /* response header */
    uint32_t hdr_size;   /* response header size */
    uint8_t *body;       /* response body */
    uint32_t body_size;  /* response body size */
    ER(*cgi_func)(ID cepid, void *res_info);
} HTTPD_RESOURCE;

#define QUERY_PARAM_SIZE (80)

typedef struct httpd_resource_info_type
{
    HTTPD_RESOURCE res;
    uint8_t   param[QUERY_PARAM_SIZE+1];  /* parameter after "?" */
} HTTPD_RESOURCE_INFO;

/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/
void          R_httpd(void);
uint32_t      R_T4_HTTP_SERVER_GetVersion(void);
void          R_httpd_pending_release_request(ID cepid);

#endif /* _R_T4_HTTP_SERVER_RX_IF_H_ */

/* Copyright 2011 RENESAS ELECTRONICS CORPORATION */
