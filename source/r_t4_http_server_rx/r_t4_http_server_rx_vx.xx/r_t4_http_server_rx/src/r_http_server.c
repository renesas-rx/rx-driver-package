/*******************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only
* intended for use with Renesas products. No other uses are authorized. This
* software is owned by Renesas Electronics Corporation and is protected under
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
* LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
* AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR
* ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software
* and to discontinue the availability of this software. By using this software,
* you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2011-2014 Renesas Electronics Corporation. All rights reserved.
*******************************************************************************/
/*******************************************************************************
* File Name    : r_http_server.c
* Version      : 1.01
* Device(s)    :
* Tool-Chain   :
* OS           : none
* H/W Platform :
* Description  : HTTP server code.
******************************************************************************/
/******************************************************************************
* History : DD.MM.YYYY Version Description
*         : 01.04.2011 1.00    First Release
*         : 09.05.2014 1.03    Corresponded to FIT Modules
*         : 09.05.2014 1.04    Clean up source code
******************************************************************************/

/******************************************************************************
Includes <System Includes> , "Project Includes"
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "r_t4_itcpip.h"
#include "r_t4_file_driver_rx_if.h"
#include "r_http_server_config.h"
#include "r_t4_http_server_rx_if.h"

/******************************************************************************
Macro definitions
******************************************************************************/
#define CONST const

#define RESTYPE_NON     0x00
#define RESTYPE_CGI     0x01
#define RESTYPE_FILE    0x02
#define RESTYPE_INDEXES 0x03
#define HTTP_SRV

#define HTTPD_CLOSED                0
#define HTTPD_ACCEPT                1
#define HTTPD_WAIT_REQUEST          2
#define HTTPD_CGI_PENDING           3
#define HTTPD_CGI_GEN_RESPONSE      4
#define HTTPD_SEND_RESPONSE_HEADER  5
#define HTTPD_SEND_RESPONSE_BODY    6
#define HTTPD_CLOSING               7

#define METHOD_GET     1
#define METHOD_HEAD    2
#define METHOD_ERR     -1

#define HTTP_ERROR_CODE_404 404
#define HTTP_ERROR_CODE_500 500
#define HTTP_ERROR_CODE_501 501

#define SNAME_LEN 12
#define SNAME_LIMIT_LEN 10

/******************************************************************************
Typedef definitions
******************************************************************************/
typedef struct http_cep_
{
    uint8_t status;
    uint8_t current_cgi_func_index;
    uint8_t hdr_buff[HDR_BUF_SIZE];
    uint8_t body_buff[BODY_BUF_SIZE];
    uint8_t rcv_buff[RCV_BUF_SIZE];
    int32_t remain_data;
    int32_t now_data;
    int32_t total_rcv_len;
    int32_t fileid;
    T_IPV4EP dstaddr;
    HTTPD_RESOURCE_INFO res_info;
    FILE_LIST file_list[HTTP_MAX_FILE_LIST];
    DATE_INFO current_file_date_info;
} HTTP_CEP;

/******************************************************************************
Private global variables and functions
******************************************************************************/
static ER httpd_rcv_request(ID cepid, uint8_t *buf, int16_t buf_len, HTTPD_RESOURCE_INFO *res_info, ER rcv_dat_size);
static ER httpd_parse_request(ID cepid, uint8_t *buf, HTTPD_RESOURCE_INFO *res_info);
static ER httpd_parse_request_not_found(ID cepid);
static ER httpd_parse_request_internal_server_error(ID cepid);
static ER httpd_parse_request_not_implemented(ID cepid);
static ER httpd_gen_response_cgi(ID cepid, ER ercd);

static ER gen_response_header(ID cepid, uint8_t *uri, uint8_t *buf, uint32_t content_length);
static ER gen_response_error_header(uint8_t *buf, int32_t error_code);
static ER gen_response_indexes(ID cepid, uint8_t *dir_name);
static ER gen_response_cgi(ID cepid, uint8_t *buf);

static ER set_http_resource(ID cepid, uint32_t hdr_size, uint8_t *hdr_buff, uint32_t body_size, uint8_t *body_buff);
static ER set_lastmodified_date(ID cepid, uint8_t *buf, DATE_INFO *date_info);
static ER set_content_type(int16_t extension_no, uint8_t *buf);
static ER set_content_length(uint32_t content_size, uint8_t *buf);
static ER set_allow(uint8_t *buf);

static ER get_extension_no(uint8_t *buf);
static ER align_file_name_space(const uint8_t *src, uint8_t *dst, uint8_t **space);
static ER extension_check(uint8_t *buf);
static ER method_check(uint8_t *buf);
static ER strcat_with_check_len(uint8_t *src, uint8_t *concat, int32_t size);
static ER is_request_cgi(uint8_t *buf);

static uint8_t *get_time(void);
static uint8_t *conv_month_for_systime(int32_t file_month);
static uint8_t *conv_day_for_systime(int32_t tmp_year, int32_t tmp_month, int32_t tmp_day);

static const uint8_t day_data[7][4] = {{"Sun"}, {"Mon"}, {"Tue"}, {"Wed"}, {"Thu"}, {"Fri"}, {"Sat"}};
static const uint8_t month_data[12][4] = {{"Jan"}, {"Feb"}, {"Mar"}, {"Apr"}, {"May"}, {"Jun"}, {"Jul"}, {"Aug"}, {"Sep"}, {"Oct"}, {"Nov"}, {"Dec"}};

/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/
HTTP_CEP http_cep[HTTP_TCP_CEP_NUM];
uint8_t lf_code[] = LF_CODE;
ER http_callback(ID cepid, FN fncd , VP p_parblk);

/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/
extern char _T4_Version[];
extern SYS_TIME *get_sys_time(void);

/*------------ PROCESS -----------*/

/******************************************************************************
* Function Name : R_httpd
* Description   : Watch the HTTP server status.
* Argument      : none
* Return Value  : none
******************************************************************************/
void R_httpd(void)
{
    ER   ercd;
    uint32_t  i;
    static int32_t init_flag = 0;

    /* Initialize process */
    if (init_flag == 0)
    {
        memset(http_cep, 0, sizeof(http_cep));
        init_flag = 1;
        for (i = 0; i < HTTP_TCP_CEP_NUM; i++)
        {
            http_cep[i].fileid = -1;
        }
    }

    /* This function checks all communication endpoint */
    for (i = 0; i < HTTP_TCP_CEP_NUM; i++)
    {
        switch (http_cep[i].status)
        {
            case HTTPD_CLOSED:
                /* accept */
                ercd = tcp_acp_cep(i + HTTP_START_TCP_CEP + 1, i + HTTP_START_TCP_CEP + 1, &http_cep[i].dstaddr, TMO_NBLK);
                if (ercd != E_WBLK)
                {
                    while (1);
                }
                http_cep[i].status = HTTPD_ACCEPT;
                break;
            case HTTPD_CGI_GEN_RESPONSE:
                if (cgi_file_name_table[http_cep[i].current_cgi_func_index].cgi_cb_func != NULL)
                {
                    ercd = cgi_file_name_table[ http_cep[i].current_cgi_func_index].cgi_cb_func(i + 1, &http_cep[i].res_info.res);

                    ercd = httpd_gen_response_cgi(i + 1, ercd);
                    if (ercd == 0)
                    {
                        /* start transmitting response header */
                        tcp_snd_dat(i + HTTP_START_TCP_CEP + 1, (VP)http_cep[i].res_info.res.hdr, http_cep[i].res_info.res.hdr_size, TMO_NBLK);
                        http_cep[i].status = HTTPD_SEND_RESPONSE_HEADER;
                        http_cep[i].now_data = 0;
                        http_cep[i].remain_data = http_cep[i].res_info.res.hdr_size;
                    }
                    else
                    {
                        /* no transmit when there is no body data */
                        http_cep[i].now_data = 0;
                        http_cep[i].remain_data = 0;
                        http_cep[i].status = HTTPD_CLOSING;
                        tcp_cls_cep(i + HTTP_START_TCP_CEP + 1, TMO_NBLK);
                    }
                }
                else
                {
                    while (1);
                }
            default:
                break;
        }
    }
}

void R_httpd_pending_release_request(ID cepid)
{
    if (cepid <= 0)
    {
        while (1);  /* System error !! */
    }

    http_cep[cepid-1].status = HTTPD_CGI_GEN_RESPONSE;
}

static ER httpd_gen_response_cgi(ID cepid, ER ercd)
{
    if (ercd < 0 || http_cep[cepid-1].res_info.res.body == 0 || http_cep[cepid-1].res_info.res.body_size == 0)
    {
        return httpd_parse_request_internal_server_error(cepid);
    }

    /* generate response header */
    if (gen_response_cgi(cepid, http_cep[cepid-1].hdr_buff) < 0)
    {
        return httpd_parse_request_internal_server_error(cepid);
    }
    /* set response information */
    set_http_resource(cepid, strlen((char*)http_cep[cepid-1].hdr_buff), http_cep[cepid-1].hdr_buff, strlen((char*)http_cep[cepid-1].body_buff), http_cep[cepid-1].body_buff);

    return 0;
}


static ER httpd_rcv_request(ID cepid, uint8_t *buf, int16_t buf_len, HTTPD_RESOURCE_INFO *res_info, ER rcv_dat_size)
{
    uint8_t *ptr;
    ER ercd;

    if (rcv_dat_size <= 0)
    {
        http_cep[cepid-1].total_rcv_len = 0;
        return -1;
    }

    http_cep[cepid-1].total_rcv_len += rcv_dat_size;

    if (http_cep[cepid-1].total_rcv_len > (buf_len - 1))
    {
        /* If buffer is full,do not receive. */
        http_cep[cepid-1].total_rcv_len = 0;
        return -1;
    }
    buf[http_cep[cepid-1].total_rcv_len] = '\0';

    ptr = (uint8_t *)strstr((char*)buf, "\r\n\r\n");
    if (ptr != 0)
    {
        *(ptr + 4) = '\0';
        http_cep[cepid-1].total_rcv_len = 0;
//  if (httpd_parse_request(cepid, buf, res_info) < 0)

        ercd = httpd_parse_request(cepid, buf, res_info);

        /* pend sending */
        if (ercd == -2)
        {
            return -2;
        }
        else if (ercd < 0)
        {
            return -1;
        }
        return 0;
    }

    /* continue receiving */
    return -3;
}


static ER httpd_parse_request(ID cepid, uint8_t *buf, HTTPD_RESOURCE_INFO *res_info)
{
    uint8_t  *p, *q;
    uint8_t  ch = 0;

    int8_t method_no = 0;
    uint8_t tmp_path[256];

    uint32_t i;
    ER ercd;

    memset(&res_info->res, 0, sizeof(HTTPD_RESOURCE));

    /* check method */
    method_no = method_check(buf);
    if (method_no == METHOD_ERR)
    {
        return httpd_parse_request_not_implemented(cepid);
    }
    if (method_no == METHOD_GET)
    {
        buf += (sizeof("GET ") - 1);
    }
    else
    {
        buf += (sizeof("HEAD ") - 1);
    }

    p = buf;
    q = NULL;
    while (1)
    {
        ch = *p++;
        if (ch == ' ' || ch == '&')    /* first argument is only available */
        {
            *(p - 1) = '\0';
            break;
        }
        else if (ch == '?')
        {
            q = p;
        }
        else if (ch == '\0')
        {
            return httpd_parse_request_internal_server_error(cepid);
        }
    }
    if (q != NULL)
    {
        strncpy((char*)res_info->param, (const char*)q, QUERY_PARAM_SIZE - 1);
        res_info->param[QUERY_PARAM_SIZE-1] = '\0';
    }

    /* generate file path */
    tmp_path[0] = 0;
    strcat_with_check_len(tmp_path, (uint8_t*)httpd_root_directry, sizeof(tmp_path));
    strcat_with_check_len(tmp_path, (uint8_t*)buf, sizeof(tmp_path));

    /* case : specified directory */
    if ((uint8_t)(tmp_path[strlen((char*)tmp_path)-1]) == '/')
    {
        if (indexes == 0)
        {
            strcpy((char*)tmp_path, (const char *)httpd_root_directry);
            strcat((char*)tmp_path, (const char *)default_file_name);
        }
        else
        {
            /* generate indexes page */
            if (gen_response_indexes(cepid, (uint8_t*)tmp_path) == 0)
            {
                /* set response information */
                set_http_resource(cepid, strlen((char*)http_cep[cepid-1].hdr_buff), http_cep[cepid-1].hdr_buff, strlen((char*)http_cep[cepid-1].body_buff), http_cep[cepid-1].body_buff);
                http_cep[cepid-1].res_info.res.type = RESTYPE_INDEXES;
                return 0;
            }
            else
            {
                return httpd_parse_request_internal_server_error(cepid);
            }
        }
    }
    if (is_request_cgi(tmp_path) != 0)
    {
        /* search resources */
        http_cep[cepid-1].fileid = file_open((uint8_t *)tmp_path, FILE_READ);
        if (http_cep[cepid-1].fileid < 0)
        {
            return httpd_parse_request_not_found(cepid);
        }
        get_file_info(http_cep[cepid-1].fileid, &http_cep[cepid-1].current_file_date_info);
        http_cep[cepid-1].res_info.res.type = RESTYPE_FILE;

        /* generate response header */
        if (gen_response_header(cepid, buf, http_cep[cepid-1].hdr_buff, get_file_size(http_cep[cepid-1].fileid)) < 0)
        {
            return httpd_parse_request_internal_server_error(cepid);
        }
        /* set response information */
        set_http_resource(cepid, strlen((char*)http_cep[cepid-1].hdr_buff), http_cep[cepid-1].hdr_buff, get_file_size(http_cep[cepid-1].fileid), http_cep[cepid-1].body_buff);
        return 0;
    }

    else
    {
        /* serch resources for CGI */
        for (i = 0; i < MAX_CGI_FILE; i++)
        {
            if (strstr((const char*)tmp_path, (const char*)cgi_file_name_table[i].file_name))
            {
                break; // exist cgi file
            }
        }
        if (i == MAX_CGI_FILE)
        {
            return httpd_parse_request_not_found(cepid);
        }

        /* CGI fuction pointer null check */
        if (NULL == cgi_file_name_table[i].cgi_func)
        {
            return httpd_parse_request_not_found(cepid);
        }

        /* Regist CGI function */
        http_cep[cepid-1].res_info.res.type = RESTYPE_CGI;
        http_cep[cepid-1].res_info.res.cgi_func = cgi_file_name_table[i].cgi_func;
        http_cep[cepid-1].res_info.res.body = http_cep[cepid-1].body_buff;

        /* execute cgi function */
        ercd = (*http_cep[cepid-1].res_info.res.cgi_func)(cepid, &http_cep[cepid-1].res_info.res);

        /* -2: CGI pending */
        if (ercd == -2)
        {
            http_cep[cepid-1].current_cgi_func_index = i;
            return -2;
        }
        /* 0: Normal termination, -1: Internal error */
        else
        {
            ercd = httpd_gen_response_cgi(cepid, ercd);
        }

        return ercd;
    }

    /* erro path */
    return -1;
}

static ER httpd_parse_request_not_found(ID cepid)
{
    /* generate 404 Not Found response header */
    if (gen_response_error_header(http_cep[cepid-1].hdr_buff, HTTP_ERROR_CODE_404) < 0)
    {
        return httpd_parse_request_internal_server_error(cepid);
    }

    /* set response information */
    set_http_resource(cepid, strlen((char*)http_cep[cepid-1].hdr_buff), http_cep[cepid-1].hdr_buff, 0, http_cep[cepid-1].body_buff);

    return 0;
}

/* default error */
static ER httpd_parse_request_internal_server_error(ID cepid)
{

    /* generate 500 Internal Server Error response header */
    if (gen_response_error_header(http_cep[cepid-1].hdr_buff, HTTP_ERROR_CODE_500) < 0)
    {
        return -1;
    }

    /* set response information */
    set_http_resource(cepid, strlen((char*)http_cep[cepid-1].hdr_buff), http_cep[cepid-1].hdr_buff, 0, http_cep[cepid-1].body_buff);

    return 0;
}

static ER httpd_parse_request_not_implemented(ID cepid)
{
    /* generate 501 Not Implemented response header */
    if (gen_response_error_header(http_cep[cepid-1].hdr_buff, HTTP_ERROR_CODE_501) < 0)
    {
        return httpd_parse_request_internal_server_error(cepid);
    }

    /* set response information */
    set_http_resource(cepid, strlen((char*)http_cep[cepid-1].hdr_buff), http_cep[cepid-1].hdr_buff, 0, http_cep[cepid-1].body_buff);

    return 0;

}

static ER method_check(uint8_t *buf)
{

    int8_t ret = METHOD_ERR;

    if (strncmp((char*)buf, "GET ", 4) == 0)
    {
        ret = METHOD_GET;
    }
    if (strncmp((char*)buf, "HEAD ", 5) == 0)
    {
        ret = METHOD_HEAD;
    }
    return ret;
}


static ER extension_check(uint8_t *buf)
{

    int16_t loop;
    int32_t ret;
    uint8_t tmp_extension[4];

    tmp_extension[0] = (uint8_t)tolower((int32_t) * buf);
    tmp_extension[1] = (uint8_t)tolower((int32_t) * (buf + 1));
    tmp_extension[2] = (uint8_t)tolower((int32_t) * (buf + 2));
    tmp_extension[3] = 0;

    for (loop = 0; loop < MAX_EXTENSION; loop++)
    {
        if (strcmp((const char*)tmp_extension, (const char *)extension_data[loop].extension_type) == 0)
        {
            ret = loop;
            break;
        }
    }
    if (loop == MAX_EXTENSION)
    {
        ret = 0;
    }
    return ret;
}


static ER set_allow(uint8_t *buf)
{
    sprintf((char *)buf, "Allow: GET, HEAD\r\n");
    return(12);
}

static ER set_content_length(uint32_t content_size, uint8_t *buf)
{
    sprintf((char *)buf, "Content-Length: %d\r\n", content_size);

    return strlen((const char *)buf);

}

static ER set_content_type(int16_t extension_no, uint8_t *buf)
{
    if (extension_no < 0)
    {
        return -1;
    }

    sprintf((char *)buf, "Content-Type: %s\r\n", &extension_data[extension_no].content_type_buf[0]);

    return strlen((const char *)buf);

}

static ER set_lastmodified_date(ID cepid, uint8_t *buf, DATE_INFO *date_info)
{
    sprintf((char *)buf, "Last-Modified: %s, %d %s %02d %02d:%02d:%02d GMT\r\n",
            date_info->day_of_the_week,
            date_info->day,
            date_info->month,
            date_info->year,
            date_info->hour,
            date_info->min,
            date_info->sec
           );
    return 0;
}

static ER set_http_resource(ID cepid, uint32_t hdr_size, uint8_t *hdr_buff, uint32_t body_size, uint8_t *body_buff)
{
    http_cep[cepid-1].res_info.res.hdr_size = hdr_size;
    http_cep[cepid-1].res_info.res.hdr = hdr_buff;
    http_cep[cepid-1].res_info.res.body_size = body_size;
    http_cep[cepid-1].res_info.res.body = body_buff;
    return 0;
}

static ER get_extension_no(uint8_t *buf)
{
    ER extension_no;
    uint8_t *ptr;

    ptr = (uint8_t *)strchr((char*)buf, '.');
    extension_no = extension_check(ptr + 1);
    return extension_no;

}

static ER gen_response_header(ID cepid, uint8_t *uri, uint8_t *buf, uint32_t content_length)
{
    uint8_t tmp_buf[128];
    uint8_t crlf[] = "\r\n";
    ER ercd = 0;

    buf[0] = 0;

    sprintf((char *)tmp_buf, "HTTP/1.0 200 OK\r\n");
    ercd += strcat_with_check_len((uint8_t*)buf, tmp_buf, HDR_BUF_SIZE);

    sprintf((char *)tmp_buf, "Date: %s\r\n", get_time());
    ercd += strcat_with_check_len((uint8_t*)buf, tmp_buf, HDR_BUF_SIZE);

    // Server:
    sprintf((char *)tmp_buf, "Server: %s\r\n", HTTPD_VERSION_CODE);
    ercd += strcat_with_check_len((uint8_t*)buf, tmp_buf, HDR_BUF_SIZE);

    // Allow:
    set_allow(tmp_buf);
    ercd += strcat_with_check_len((uint8_t*)buf, tmp_buf, HDR_BUF_SIZE);

    // Last-Modified:
    set_lastmodified_date(cepid, tmp_buf, &http_cep[cepid-1].current_file_date_info);
    ercd += strcat_with_check_len((uint8_t*)buf, tmp_buf, HDR_BUF_SIZE);

    // Content-Type:
    set_content_type(get_extension_no((uint8_t *)uri), tmp_buf);
    ercd += strcat_with_check_len((uint8_t*)buf, tmp_buf, HDR_BUF_SIZE);

    // Content-Length:
    set_content_length(content_length, tmp_buf);
    ercd += strcat_with_check_len((uint8_t*)buf, tmp_buf, HDR_BUF_SIZE);

    // Connection:
    sprintf((char *)tmp_buf, "Connection: Close\r\n");
    ercd += strcat_with_check_len((uint8_t*)buf, tmp_buf, HDR_BUF_SIZE);

    // End of header
    ercd += strcat_with_check_len((uint8_t*)buf, crlf, HDR_BUF_SIZE);

    return 0;
}

static ER gen_response_error_header(uint8_t *buf, int32_t error_code)
{
    uint8_t tmp_buf[128];
    uint8_t crlf[] = "\r\n";
    ER ercd = 0;

    buf[0] = 0;

    switch (error_code)
    {
        case HTTP_ERROR_CODE_404:
            sprintf((char *)tmp_buf, "HTTP/1.0 404 Not Found\r\n");
            ercd += strcat_with_check_len((uint8_t*)buf, tmp_buf, HDR_BUF_SIZE);
            break;

        case HTTP_ERROR_CODE_500:
            sprintf((char *)tmp_buf, "HTTP/1.0 500 Internal Server Error\r\n");
            ercd += strcat_with_check_len((uint8_t*)buf, tmp_buf, HDR_BUF_SIZE);
            break;

        case HTTP_ERROR_CODE_501:
            sprintf((char *)tmp_buf, "HTTP/1.0 501 Not Implemented \r\n");
            ercd += strcat_with_check_len((uint8_t*)buf, tmp_buf, HDR_BUF_SIZE);
            break;
    }

    sprintf((char *)tmp_buf, "Date: %s\r\n", get_time());
    ercd += strcat_with_check_len((uint8_t*)buf, tmp_buf, HDR_BUF_SIZE);

    // Server:
    sprintf((char *)tmp_buf, "Server: %s\r\n", HTTPD_VERSION_CODE);
    ercd += strcat_with_check_len((uint8_t*)buf, tmp_buf, HDR_BUF_SIZE);

    // Connection:
    sprintf((char *)tmp_buf, "Connection: Close\r\n");
    ercd += strcat_with_check_len((uint8_t*)buf, tmp_buf, HDR_BUF_SIZE);

    // End of header
    ercd += strcat_with_check_len((uint8_t*)buf, crlf, HDR_BUF_SIZE);

    return ercd;
}


static ER align_file_name_space(const uint8_t *src, uint8_t *dst, uint8_t **space)
{
    static const uint8_t c_space[SNAME_LEN+1] = "            ";
    int32_t src_len;

    src_len = strlen((const char *)src);
    if (src_len > SNAME_LEN)
    {
        strncpy((char *)dst, (const char *)src, SNAME_LIMIT_LEN);
        dst[SNAME_LIMIT_LEN+1] = '~';
        dst[SNAME_LIMIT_LEN+1] = src[src_len-1];
        *space = (uint8_t *) & c_space[SNAME_LEN-1];
    }
    else
    {
        strcpy((char *)dst, (const char *)src);
        *space = (uint8_t *) & c_space[src_len];
    }

    return 0;
}

static ER gen_response_indexes(ID cepid, uint8_t *dir_name)
{
    uint8_t tmp_buf[512];
    uint8_t parent_dir_name[256];
    uint8_t crlf[] = "\r\n";
    int32_t i = 0;
    uint8_t shortname[13];
    uint8_t *p_space;
    ER ercd = 0;
    uint8_t *user_path;
    int32_t filenum;

    http_cep[cepid-1].hdr_buff[0] = 0;
    http_cep[cepid-1].body_buff[0] = 0;

    // generate body
    filenum = get_file_list_info((uint8_t *)dir_name, http_cep[cepid-1].file_list, HTTP_MAX_FILE_LIST, 0);
    if (filenum < 0)
    {
        return -1;
    }

    sprintf((char *)tmp_buf, "<html>%s<body>%s<pre>%s", lf_code, lf_code, lf_code);
    ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].body_buff, tmp_buf, sizeof(http_cep[cepid-1].body_buff));

    user_path = (uint8_t *)strstr((const char *)dir_name, (const char *)httpd_root_directry);
    user_path += strlen((const char *)httpd_root_directry);
    sprintf((char *)tmp_buf, "Index of %s %s%s", user_path, lf_code, lf_code, lf_code);
    ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].body_buff, tmp_buf, sizeof(http_cep[cepid-1].body_buff));

    sprintf((char *)tmp_buf, "        Name            Last modified       Size%s", lf_code);
    ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].body_buff, tmp_buf, sizeof(http_cep[cepid-1].body_buff));

    sprintf((char *)tmp_buf, "<hr>%s", lf_code);
    ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].body_buff, tmp_buf, sizeof(http_cep[cepid-1].body_buff));

    /* display parent directory */
    if (strlen((const char *)user_path) != 1)
    {
        strcpy((char *)parent_dir_name, (const char *)user_path);
        parent_dir_name[strlen((const char *)parent_dir_name)-1] = 0;
        *(strrchr((const char *)parent_dir_name, '/')) = 0;
        sprintf((char *)tmp_buf, "<a href=%s/>Parent Directory</a> ", parent_dir_name);
        ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].body_buff, tmp_buf, sizeof(http_cep[cepid-1].body_buff));
        /* line feed code */
        sprintf((char *)tmp_buf, "%s", lf_code);
        ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].body_buff, tmp_buf, sizeof(http_cep[cepid-1].body_buff));
    }

    while (i < HTTP_MAX_FILE_LIST)
    {
        if (http_cep[cepid-1].file_list[i].file_name[0] == 0)
        {
            break;
        }
        else
        {
            /* directory name */
            if ((http_cep[cepid-1].file_list[i].file_attr & FILE_ATTR_DIR) == FILE_ATTR_DIR)
            {
                align_file_name_space((const uint8_t *)http_cep[cepid-1].file_list[i].file_name, shortname, &p_space);
                sprintf((char *)tmp_buf, "<a href=%s%s/>%s</a>%s     ", user_path, http_cep[cepid-1].file_list[i].file_name, shortname, p_space);
            }
            /* file name */
            else
            {
                align_file_name_space((const uint8_t *)http_cep[cepid-1].file_list[i].file_name, shortname, &p_space);
                sprintf((char *)tmp_buf, "<a href=%s%s>%s</a>%s     ", user_path, http_cep[cepid-1].file_list[i].file_name, shortname, p_space);

            }
            ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].body_buff, tmp_buf, sizeof(http_cep[cepid-1].body_buff));

            /* Last modified */
            sprintf((char *)tmp_buf, "%2d-%s-%02d %02d:%02d:%02d ",
                    http_cep[cepid-1].file_list[i].date_info.day,
                    http_cep[cepid-1].file_list[i].date_info.month,
                    http_cep[cepid-1].file_list[i].date_info.year,
                    http_cep[cepid-1].file_list[i].date_info.hour,
                    http_cep[cepid-1].file_list[i].date_info.min,
                    http_cep[cepid-1].file_list[i].date_info.sec
                   );
            ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].body_buff, tmp_buf, sizeof(http_cep[cepid-1].body_buff));

            /* directory disp */
            if ((http_cep[cepid-1].file_list[i].file_attr & FILE_ATTR_DIR) == FILE_ATTR_DIR)
            {
                sprintf((char *)tmp_buf, "     (dir)");
            }
            /* file size disp*/
            else
            {
                sprintf((char *)tmp_buf, "%10ld", http_cep[cepid-1].file_list[i].file_size);
            }
            ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].body_buff, tmp_buf, sizeof(http_cep[cepid-1].body_buff));

            /* line feed code */
            sprintf((char *)tmp_buf, "%s", lf_code);
            ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].body_buff, tmp_buf, sizeof(http_cep[cepid-1].body_buff));
        }
        i++;
    }

    sprintf((char *)tmp_buf, "<hr>%s", lf_code);
    ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].body_buff, tmp_buf, sizeof(http_cep[cepid-1].body_buff));

    sprintf((char *)tmp_buf, "%s", HTTPD_VERSION_CODE);
    ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].body_buff, tmp_buf, sizeof(http_cep[cepid-1].body_buff));

    sprintf((char *)tmp_buf, "</pre>%s</body>%s</html>", lf_code, lf_code, lf_code);
    ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].body_buff, tmp_buf, sizeof(http_cep[cepid-1].body_buff));

    // generate header
    sprintf((char *)tmp_buf, "HTTP/1.0 200 OK\r\n");
    ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].hdr_buff, tmp_buf, sizeof(http_cep[cepid-1].hdr_buff));

    sprintf((char *)tmp_buf, "Date: %s\r\n", get_time());
    ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].hdr_buff, tmp_buf, sizeof(http_cep[cepid-1].hdr_buff));

    // Server:
    sprintf((char *)tmp_buf, "Server: %s\r\n", HTTPD_VERSION_CODE);
    ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].hdr_buff, tmp_buf, sizeof(http_cep[cepid-1].hdr_buff));

    // Content-Length:
    set_content_length(strlen((char*)http_cep[cepid-1].body_buff), tmp_buf);
    ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].hdr_buff, tmp_buf, sizeof(http_cep[cepid-1].hdr_buff));

    // Connection:
    sprintf((char *)tmp_buf, "Connection: Close\r\n");
    ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].hdr_buff, tmp_buf, sizeof(http_cep[cepid-1].hdr_buff));

    // End of header
    ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].hdr_buff, crlf, sizeof(http_cep[cepid-1].hdr_buff));

    return ercd;
}

static ER gen_response_cgi(ID cepid, uint8_t *buf)
{
    ER ercd = 0;
    uint8_t tmp_buf[256];
    uint8_t crlf[] = "\r\n";

    buf[0] = 0;

    // generate header
    sprintf((char *)tmp_buf, "HTTP/1.0 200 OK\r\n");
    ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].hdr_buff, tmp_buf, sizeof(http_cep[cepid-1].hdr_buff));

    sprintf((char *)tmp_buf, "Date: %s\r\n", get_time());
    ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].hdr_buff, tmp_buf, sizeof(http_cep[cepid-1].hdr_buff));

    // Server:
    sprintf((char *)tmp_buf, "Server: %s\r\n", HTTPD_VERSION_CODE);
    ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].hdr_buff, tmp_buf, sizeof(http_cep[cepid-1].hdr_buff));

    // Content-Length:
    set_content_length(strlen((char*)http_cep[cepid-1].body_buff), tmp_buf);
    ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].hdr_buff, tmp_buf, sizeof(http_cep[cepid-1].hdr_buff));

    // Connection:
    sprintf((char *)tmp_buf, "Connection: Close\r\n");
    ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].hdr_buff, tmp_buf, sizeof(http_cep[cepid-1].hdr_buff));

    // End of header
    ercd += strcat_with_check_len((uint8_t*)http_cep[cepid-1].hdr_buff, crlf, sizeof(http_cep[cepid-1].hdr_buff));

    return ercd;
}

static ER strcat_with_check_len(uint8_t *src, uint8_t *concat, int32_t size)
{
    if (strlen((const char *)src) + strlen((const char *)concat) >= size)
    {
        return -1;
    }
    strcat((char *)src, (const char *)concat);
    return 0;
}

static ER is_request_cgi(uint8_t *buf)
{
    char *strstr_return;
    ER return_value;

    strstr_return = strstr((const char*)buf, (const char *)".cgi");

    if (strstr_return == 0)
    {
        return_value = -1;
    }
    else
    {
        return_value = 0;
    }
    return return_value;
}

static uint8_t *get_time(void)
{
    static uint8_t date[] = {"Mon, 07 Feb 2011 12:00:00 GMT"};
    SYS_TIME *ptime;

    ptime = get_sys_time();

    if (NULL != ptime)
    {
        sprintf((char *)date, "%s, %02d %s %4d %02d:%02d:%02d GMT",
                conv_day_for_systime(ptime->year, ptime->month, ptime->day),
                ptime->day,
                conv_month_for_systime(ptime->month),
                ptime->year,
                ptime->hour,
                ptime->min,
                ptime->sec
               );
    }

    return date;
}

static uint8_t *conv_month_for_systime(int32_t file_month)
{
    return (uint8_t*)month_data[file_month-1];
}


static uint8_t *conv_day_for_systime(int32_t tmp_year, int32_t tmp_month, int32_t tmp_day)
{
    if ((tmp_month == 1) || (tmp_month == 2))
    {
        tmp_year -= 1;
        tmp_month += 12;
    }
    return (uint8_t*)day_data[((tmp_year + (tmp_year/4) - (tmp_year/100) + (tmp_year/400) + ((13*tmp_month + 8)/5) + tmp_day) % 7)];
}

ER http_callback(ID cepid, FN fncd , VP p_parblk)
{

    ER   parblk = *(ER *)p_parblk;
    ER   ercd;
    ID   cepid_oft;

    if (cepid > HTTP_START_TCP_CEP)
    {
        cepid_oft = cepid - HTTP_START_TCP_CEP;
    }
    else /* if (cepid > HTTP_START_TCP_CEP) */
    {
        while (1);  /* System error !! */
    }

    switch (fncd)
    {
        case TFN_TCP_ACP_CEP:
            http_cep[cepid_oft-1].status = HTTPD_WAIT_REQUEST;
            tcp_rcv_dat(cepid, http_cep[cepid_oft-1].rcv_buff, sizeof(http_cep[cepid_oft-1].rcv_buff), TMO_NBLK);
            break;
        case TFN_TCP_SND_DAT:
            if (parblk < 0)
            {
                /* In case : FORCE DISCONNECT */
                http_cep[cepid_oft-1].now_data = 0;
                http_cep[cepid_oft-1].remain_data = 0;
                tcp_cls_cep(cepid, TMO_NBLK);
                http_cep[cepid_oft-1].status = HTTPD_CLOSING;
            }
            if (http_cep[cepid_oft-1].status == HTTPD_SEND_RESPONSE_HEADER)
            {
                http_cep[cepid_oft-1].now_data += parblk;
                http_cep[cepid_oft-1].remain_data -= parblk;
                if (http_cep[cepid_oft-1].remain_data == 0)
                {
                    /* get file data */
                    if (http_cep[cepid_oft-1].res_info.res.type == RESTYPE_FILE)
                    {
                        if (file_read(http_cep[cepid_oft-1].fileid, http_cep[cepid_oft-1].body_buff, BODY_BUF_SIZE) < 0)
                        {
                            /* no more transmit when error has occured */
                            http_cep[cepid_oft-1].now_data = 0;
                            http_cep[cepid_oft-1].remain_data = 0;
                            tcp_cls_cep(cepid, TMO_NBLK);
                            http_cep[cepid_oft-1].status = HTTPD_CLOSING;
                            break;
                        }
                    }
                    http_cep[cepid_oft-1].status = HTTPD_SEND_RESPONSE_BODY;
                    http_cep[cepid_oft-1].now_data = 0;
                    http_cep[cepid_oft-1].remain_data = http_cep[cepid_oft-1].res_info.res.body_size;
                    if (http_cep[cepid_oft-1].remain_data > BODY_BUF_SIZE)
                    {
                        tcp_snd_dat(cepid, (VP)(http_cep[cepid_oft-1].res_info.res.body), BODY_BUF_SIZE, TMO_NBLK);
                    }
                    else if (http_cep[cepid_oft-1].remain_data > 0)
                    {
                        tcp_snd_dat(cepid, (VP)(http_cep[cepid_oft-1].res_info.res.body), http_cep[cepid_oft-1].res_info.res.body_size, TMO_NBLK);
                    }
                    else
                    {
                        /* no transmit when there is no body data */
                        http_cep[cepid_oft-1].now_data = 0;
                        http_cep[cepid_oft-1].remain_data = 0;
                        tcp_cls_cep(cepid, TMO_NBLK);
                        http_cep[cepid_oft-1].status = HTTPD_CLOSING;
                        break;
                    }
                }
                else
                {
                    if (http_cep[cepid_oft-1].remain_data > BODY_BUF_SIZE)
                    {
                        tcp_snd_dat(cepid, (VP)(http_cep[cepid_oft-1].res_info.res.hdr + http_cep[cepid_oft-1].now_data), BODY_BUF_SIZE, TMO_NBLK);
                    }
                    else
                    {
                        tcp_snd_dat(cepid, (VP)(http_cep[cepid_oft-1].res_info.res.hdr + http_cep[cepid_oft-1].now_data), http_cep[cepid_oft-1].res_info.res.hdr_size, TMO_NBLK);
                    }
                }
            }
            else if (http_cep[cepid_oft-1].status == HTTPD_SEND_RESPONSE_BODY)
            {
                http_cep[cepid_oft-1].now_data += parblk;
                http_cep[cepid_oft-1].remain_data -= parblk;
                if (http_cep[cepid_oft-1].remain_data == 0)
                {
                    http_cep[cepid_oft-1].now_data = 0;
                    http_cep[cepid_oft-1].remain_data = 0;
                    tcp_cls_cep(cepid, TMO_NBLK);
                    http_cep[cepid_oft-1].status = HTTPD_CLOSING;
                    break;
                }
                else
                {
                    if (file_read(http_cep[cepid_oft-1].fileid, http_cep[cepid_oft-1].body_buff, BODY_BUF_SIZE) < 0)
                    {
                        /* no more transmit when error has occured */
                        http_cep[cepid_oft-1].now_data = 0;
                        http_cep[cepid_oft-1].remain_data = 0;
                        tcp_cls_cep(cepid, TMO_NBLK);
                        http_cep[cepid_oft-1].status = HTTPD_CLOSING;
                        break;
                    }
                    if (http_cep[cepid_oft-1].remain_data > BODY_BUF_SIZE)
                    {
                        tcp_snd_dat(cepid, (VP)(http_cep[cepid_oft-1].res_info.res.body), BODY_BUF_SIZE, TMO_NBLK);
                    }
                    else
                    {
                        tcp_snd_dat(cepid, (VP)(http_cep[cepid_oft-1].res_info.res.body), http_cep[cepid_oft-1].remain_data, TMO_NBLK);
                    }
                }
            }
            break;
        case TFN_TCP_RCV_DAT:
            /* receive request, return value -1 == error, 0 == complete, -2 == pend sending, -3 == continue */
            ercd = httpd_rcv_request(cepid_oft, http_cep[cepid_oft-1].rcv_buff, sizeof(http_cep[cepid_oft-1].rcv_buff), &http_cep[cepid_oft-1].res_info, *(ER *)p_parblk);

            /* complete */
            if (ercd == 0)
            {
                /* start transmitting response header */
                tcp_snd_dat(cepid, (VP)http_cep[cepid_oft-1].res_info.res.hdr, http_cep[cepid_oft-1].res_info.res.hdr_size, TMO_NBLK);
                http_cep[cepid_oft-1].status = HTTPD_SEND_RESPONSE_HEADER;
                http_cep[cepid_oft-1].now_data = 0;
                http_cep[cepid_oft-1].remain_data = http_cep[cepid_oft-1].res_info.res.hdr_size;
            }
            /* pend sending */
            else if (ercd == -2)
            {
                http_cep[cepid_oft-1].status = HTTPD_CGI_PENDING;
                break;
            }
            /* continue */
            else if (ercd == -3)
            {
                tcp_rcv_dat(cepid, http_cep[cepid_oft-1].rcv_buff + http_cep[cepid_oft-1].total_rcv_len, sizeof(http_cep[cepid_oft-1].rcv_buff) - http_cep[cepid_oft-1].total_rcv_len, TMO_NBLK);
            }
            /* error */
            else
            {
                tcp_cls_cep(cepid, TMO_NBLK);
                http_cep[cepid_oft-1].status = HTTPD_CLOSING;
                break;
            }
            break;
        case TFN_TCP_SHT_CEP:
            break;
        case TFN_TCP_CLS_CEP:
            http_cep[cepid_oft-1].status = HTTPD_CLOSED;
            file_close(http_cep[cepid_oft-1].fileid);
            http_cep[cepid_oft-1].fileid = -1;
            break;
        default:
            break;
    }
    return(0);
}

/*****************************************************************************
* Function Name: R_T4_HTTP_SERVER_GetVersion
* Description  : Returns the version of this module. The version number is
*                encoded such that the top two bytes are the major version
*                number and the bottom two bytes are the minor version number.
* Arguments    : none
* Return Value : version number
******************************************************************************/
#pragma inline(R_T4_HTTP_SERVER_GetVersion)
uint32_t  R_T4_HTTP_SERVER_GetVersion(void)
{
    uint32_t const version = (T4_HTTP_SERVER_VERSION_MAJOR << 16) | T4_HTTP_SERVER_VERSION_MINOR;

    return version;
}

/* Copyright 2005,2013 RENESAS ELECTRONICS CORPORATION */







