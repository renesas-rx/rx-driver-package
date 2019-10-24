#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "platform.h"
#include "r_sci_rx_if.h"
#include "r_byteq_if.h"
#include "r_wifi_esp32_if.h"
#include "r_wifi_esp32_private.h"

#define ESP32_DATA_RECEIVE         "+IPD,%d,%d:"
#define ESP32_READ_MAC             "+CIPSTAMAC"
#define ESP32_READ_IP              "+CIPSTA"
#define ESP32_READ_DNS             "+CIPDOMAIN"
#define ESP32_READ_APLIST          "+CWLAP"

#if(1 == WIFI_CFG_USE_CALLBACK_FUNCTION)
void WIFI_CFG_CALLBACK_FUNCTION_NAME(void *p_args);
void (* const p_wifi_callback)(void *p_args) = WIFI_CFG_CALLBACK_FUNCTION_NAME;
#else
void (* const p_wifi_callback)(void *p_args) = NULL;
#endif


uint32_t g_wifi_response_recv_count;
uint32_t tmp_recvcnt;
uint32_t g_wifi_response_recv_status;

char g_wifi_response_last_string[15];
uint8_t g_wifi_response_last_string_recv_count;

uint8_t g_wifi_macaddress[6];
wifi_ip_configuration_t g_wifi_ipconfig;
uint32_t g_wifi_dnsaddress;

wifi_scan_result_t *gp_wifi_ap_results;
uint32_t g_wifi_aplistmax;
uint32_t g_wifi_aplist_stored_num;
uint32_t g_wifi_aplist_count;

wifi_socket_t g_wifi_socket[WIFI_CFG_CREATABLE_SOCKETS];



wifi_at_execute_queue_t g_wifi_at_execute_queue[10];
uint8_t g_wifi_set_queue_index;
uint8_t g_wifi_get_queue_index;

static void wifi_recv_task( void * pvParameters );
TaskHandle_t g_wifi_recv_task_handle;


int32_t wifi_start_recv_task( void )
{
	int32_t ret = -1;
	BaseType_t xReturned;

    /* Create wifi driver at response tasks. */
	xReturned = xTaskCreate( wifi_recv_task,                               /* The function that implements the task. */
                 "esp32_recv",                                     /* Just a text name for the task to aid debugging. */
                 1024,
                 ( void * ) 0,                                    /* The task parameter, not used in this case. */
				 tskIDLE_PRIORITY + 6,
				 &g_wifi_recv_task_handle );                                          /* The task handle is not used. */
	if(pdPASS == xReturned)
	{
		ret = 0;
	}
	return ret;
}

void wifi_delete_recv_task( void )
{
    /* Delete wifi driver at response tasks. */
    if( NULL != g_wifi_recv_task_handle )
    {
    	vTaskDelete(g_wifi_recv_task_handle);
    }
}

static void wifi_recv_task( void * pvParameters )
{
	uint8_t data;
	uint8_t *presponse_buff;
	uint8_t *pwkstr1;
	sci_err_t sci_ercd;
	int sscanf_ret;
	int i;
	uint32_t socket_no;
	uint32_t len;
	byteq_err_t byteq_ercd;
	uint32_t tmp_recvcnt;
	uint32_t psw_value;
	wifi_at_execute_queue_t * pqueue;
	wifi_err_event_t event;
	g_wifi_response_recv_count = 0;
	g_wifi_response_last_string_recv_count = 0;
	memset(g_wifi_response_last_string, 0,sizeof(g_wifi_response_last_string));

	presponse_buff = g_wifi_uart[WIFI_UART_COMMAND_PORT].response_buff;

    for( ; ; )
    {
    	sci_ercd = R_SCI_Receive(g_wifi_uart[WIFI_UART_COMMAND_PORT].wifi_uart_sci_handle, &data, 1);
    	if(SCI_ERR_INSUFFICIENT_DATA == sci_ercd)
    	{
    		/* Pause for a short while to ensure the network is not too
         * congested. */
    		vTaskDelay( 1 );

    	}
    	else if(SCI_SUCCESS == sci_ercd)
    	{
    		if(0x2002 != g_wifi_response_recv_status)
    		{
				if(g_wifi_response_recv_count >= g_wifi_uart[WIFI_UART_COMMAND_PORT].response_buff_size -1 )
				{
					event.event = WIFI_EVENT_RCV_TASK_RXB_OVF_ERR;
					if(NULL != p_wifi_callback)
					{
						p_wifi_callback(&event);
					}
				}
				else
				{
					presponse_buff[g_wifi_response_recv_count] = data;
				}
				g_wifi_response_recv_count++;
    		}
			if(g_wifi_response_last_string_recv_count >= (sizeof(g_wifi_response_last_string) -1))
			{
				memmove(&g_wifi_response_last_string[0], &g_wifi_response_last_string[1], (sizeof(g_wifi_response_last_string) -1));
				g_wifi_response_last_string[sizeof(g_wifi_response_last_string) -2] = data;
				g_wifi_response_last_string[sizeof(g_wifi_response_last_string) -1] = '\0';
			}
			else
			{
				g_wifi_response_last_string[g_wifi_response_last_string_recv_count] = data;
				g_wifi_response_last_string[g_wifi_response_last_string_recv_count+1] = '\0';
				g_wifi_response_last_string_recv_count++;

			}
#if DEBUGLOG == 2
			R_BSP_CpuInterruptLevelWrite (WIFI_CFG_SCI_INTERRUPT_LEVEL-1);
    		putchar(data);
			R_BSP_CpuInterruptLevelWrite (0);
#endif
    		switch(g_wifi_response_recv_status)
    		{
	    		case 0x0000:
		    		switch(data)
		    		{
		    			case '\r':
		    				g_wifi_response_recv_status = 0x1001;
		    			break;
		    			case '+':
		    				g_wifi_response_recv_status = 0x2001;
		    			break;
		    			case '>':
//		    			    internal_callback_socketsend();
		    				pqueue = wifi_get_current_running_queue();
		    				if(ESP32_SET_SOCKET_SEND_START == pqueue->at_command_id)
		    				{
		    					wifi_set_result_to_current_running_queue( WIFI_RETURN_ENUM_OK_GO_SEND );
#if DEBUGLOG >=3
								R_BSP_CpuInterruptLevelWrite (WIFI_CFG_SCI_INTERRUPT_LEVEL-1);
								printf("ID:1 %s",presponse_buff);
								R_BSP_CpuInterruptLevelWrite (0);
#endif
								memset(presponse_buff, 0, g_wifi_uart[WIFI_UART_COMMAND_PORT].response_buff_size);
								g_wifi_response_recv_count = 0;
								g_wifi_response_last_string_recv_count = 0;
								g_wifi_response_recv_status = 0x0000;
		    				}
		    				else
		    				{
			    				g_wifi_response_recv_status = 0xF001;
		    				}
			    			break;
		    			default:
		    				g_wifi_response_recv_status = 0xF001;
		    			break;
		    		}
		    		break;
	    		case 0x1001:
		    		switch(data)
		    		{
		    			case '\n':
//		    				g_wifi_response_recv_status = 0x1002;
							memset(presponse_buff, 0, g_wifi_uart[WIFI_UART_COMMAND_PORT].response_buff_size);
							g_wifi_response_recv_count = 0;
							g_wifi_response_last_string_recv_count = 0;
		    				g_wifi_response_recv_status = 0x0000;
		    			break;
		    			default:
		    				g_wifi_response_recv_status = 0xF001;
		    			break;
		    		}
		    		break;
	    		case 0x2001:	/* + */
		    		switch(data)
		    		{
		    			case ':':
		    				pwkstr1 = strchr((const char *)presponse_buff,'+');
		    				sscanf_ret = sscanf((const char *)pwkstr1,ESP32_DATA_RECEIVE,&socket_no,&len);
							if(sscanf_ret == 2)
							{
								g_wifi_socket[socket_no].receive_num = len;
								g_wifi_socket[socket_no].receive_count = 0;
								g_wifi_socket[socket_no].put_error_count = 0;
			    				g_wifi_response_recv_status = 0x2002;
		    				}
		    				else
		    				{
			    				g_wifi_response_recv_status = 0x2102;
		    				}
		    			break;
		    			case '\r':
		    				g_wifi_response_recv_status = 0xF00F;
		    			break;
		    			default:
		    			break;
		    		}
		    		break;
	    		case 0x2002:	/* +IPD,xx,xx: socket receive */
	    			psw_value = R_BSP_CpuInterruptLevelRead();
    				R_BSP_CpuInterruptLevelWrite (WIFI_CFG_SCI_INTERRUPT_LEVEL-1);
	    			byteq_ercd = R_BYTEQ_Put(g_wifi_socket[socket_no].socket_byteq_hdl, data);
					R_BSP_CpuInterruptLevelWrite (psw_value);
	    			if(byteq_ercd != BYTEQ_SUCCESS)
	    			{
	    				g_wifi_socket[socket_no].put_error_count++;
	    				event.event = WIFI_EVENT_SOCKET_RXQ_OVF_ERR;
	    				event.socket_no = socket_no;
						if(NULL != p_wifi_callback)
						{
							p_wifi_callback(&event);
						}
	    			}
	    			g_wifi_socket[socket_no].receive_count++;
	    			if(g_wifi_socket[socket_no].receive_count >= g_wifi_socket[socket_no].receive_num)
	    			{
#if DEBUGLOG >=3
	    				R_BSP_CpuInterruptLevelWrite (WIFI_CFG_SCI_INTERRUPT_LEVEL-1);
	    				printf("ID:D recvcount = %d\r\n", g_wifi_socket[socket_no].receive_count);
						R_BSP_CpuInterruptLevelWrite (0);
#endif
						memset(presponse_buff, 0, g_wifi_uart[WIFI_UART_COMMAND_PORT].response_buff_size);
						g_wifi_response_recv_count = 0;
						g_wifi_response_last_string_recv_count = 0;
	    				g_wifi_response_recv_status = 0x0000;
	    			}
	    			break;
	    		case 0x2102:	/* +????: response option string */
		    		switch(data)
		    		{
		    			case '\r':
		    				g_wifi_response_recv_status = 0x2103;
		    			break;
		    			default:
		    			break;
		    		}
		    		break;
	    		case 0x2103:	/* +????:xxxx\r response option string */
		    		switch(data)
		    		{
		    			case '\n':
				    		presponse_buff[g_wifi_response_recv_count] = '\0';
				    		pwkstr1 = strchr((const char *)presponse_buff,':');
#if DEBUGLOG >=3
				    		R_BSP_CpuInterruptLevelWrite (WIFI_CFG_SCI_INTERRUPT_LEVEL-1);
	    					printf("ID:4 %s",presponse_buff);
	    					R_BSP_CpuInterruptLevelWrite (0);
#endif
				    		*pwkstr1 = '\0'; // ':'->'\0'

				    		pqueue = wifi_get_current_running_queue();
				    		tmp_recvcnt = strlen((const char *)presponse_buff) + 1;
				    		if (0 == strcmp((const char *)presponse_buff,ESP32_READ_MAC))
	    					{
			    				if(ESP32_GET_MACADDRESS == pqueue->at_command_id)
								{
									uint32_t temp[6];
									if(6 == sscanf(&presponse_buff[tmp_recvcnt],"\"%02x:%02x:%02x:%02x:%02x:%02x\"\r\n", &temp[0],&temp[1],&temp[2],&temp[3],&temp[4],&temp[5]))
									{
										g_wifi_macaddress[0] = (uint8_t)temp[0];
										g_wifi_macaddress[1] = (uint8_t)temp[1];
										g_wifi_macaddress[2] = (uint8_t)temp[2];
										g_wifi_macaddress[3] = (uint8_t)temp[3];
										g_wifi_macaddress[4] = (uint8_t)temp[4];
										g_wifi_macaddress[5] = (uint8_t)temp[5];
									}
								}
	    					}
				    		else if (0 == strcmp((const char *)presponse_buff,ESP32_READ_IP))
				    		{
			    				if(ESP32_GET_IPADDRESS == pqueue->at_command_id)
								{
									char *pstr;
									char *pstr2;
									pstr = (char *)&presponse_buff[tmp_recvcnt];
									uint32_t temp[4];
									pstr2 = strchr((const char *)pstr, ':');
									if(NULL != pstr2)
									{
										*pstr2 = '\0';
										if(0 == strcmp((const char *)pstr,"ip"))
										{
											pstr = pstr2 + 1;
											if(4 == sscanf((const char *)pstr,"\"%d.%d.%d.%d\"\r\n", &temp[0],&temp[1],&temp[2],&temp[3]))
											{
												g_wifi_ipconfig.ipaddress = WIFI_IPV4BYTE_TO_ULONG(temp[0], temp[1], temp[2], temp[3]);
											}
										}
										else if(0 == strcmp((const char *)pstr,"gateway"))
										{
											pstr = pstr2 + 1;
											if(4 == sscanf((const char *)pstr,"\"%d.%d.%d.%d\"\r\n", &temp[0],&temp[1],&temp[2],&temp[3]))
											{
												g_wifi_ipconfig.gateway = WIFI_IPV4BYTE_TO_ULONG(temp[0], temp[1], temp[2], temp[3]);
											}
										}
										else if(0 == strcmp((const char *)pstr,"netmask"))
										{
											pstr = pstr2 + 1;
											if(4 == sscanf((const char *)pstr,"\"%d.%d.%d.%d\"\r\n", &temp[0],&temp[1],&temp[2],&temp[3]))
											{
												g_wifi_ipconfig.subnetmask = WIFI_IPV4BYTE_TO_ULONG(temp[0], temp[1], temp[2], temp[3]);
											}
										}
									}
								}
				    		}
				    		else if (0 == strcmp((const char *)presponse_buff,ESP32_READ_DNS))
				    		{
			    				if(ESP32_SET_DNSQUERY == pqueue->at_command_id)
								{
									uint32_t temp_ip[4];
									if(4 == sscanf((const char *)&presponse_buff[tmp_recvcnt],"%d.%d.%d.%d\r\n",&temp_ip[0],&temp_ip[1],&temp_ip[2],&temp_ip[3]))
									{
										g_wifi_dnsaddress = WIFI_IPV4BYTE_TO_ULONG(temp_ip[0], temp_ip[1], temp_ip[2], temp_ip[3]);
									}
								}
				    		}
				    		else if (0 == strcmp((const char *)presponse_buff,ESP32_READ_APLIST))
				    		{
			    				if(ESP32_GET_APLIST == pqueue->at_command_id)
								{
									uint32_t ecn;
									uint8_t *pssid;
									int32_t rssi;
									int32_t mac[6];
									uint32_t ch;
									uint32_t ret;
									char *pstr;
									char *pstr2;
									pstr = (char *)&presponse_buff[tmp_recvcnt];
									ret = 0;
									if(*pstr == '(')
									{
										pstr++;
									}
									pstr2 = strchr((const char *)pstr, ',');
									if(NULL != pstr2)
									{
										*pstr2 = '\0';
										sscanf((const char *)pstr,"%d",&ecn);
										pstr = pstr2 + 1;
										ret++;
									}
									if(*pstr == '\"')
									{
										pstr++;
									}
									pstr2 = strchr((const char *)pstr,'\"');
									if(NULL != pstr2)
									{
										*pstr2 = '\0';
										pssid = pstr;
										pstr = pstr2 + 1;
										ret++;
									}
									if(*pstr == ',')
									{
										pstr++;
									}
									pstr2 = strchr((const char *)pstr, ',');
									if(NULL != pstr2)
									{
										*pstr2 = '\0';
										sscanf((const char *)pstr,"%d",&rssi);
										pstr = pstr2 + 1;
										ret++;
									}
									if(*pstr == '\"')
									{
										pstr++;
									}
									pstr2 = strchr((const char *)pstr,'\"');
									if(NULL != pstr2)
									{
										*pstr2 = '\0';
										sscanf((const char *)pstr,"%2x:%2x:%2x:%2x:%2x:%2x",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
										pstr = pstr2 + 1;
										ret++;
									}
									if(*pstr == ',')
									{
										pstr++;
									}
									pstr2 = strchr((const char *)pstr,')');
									if(NULL != pstr2)
									{
										*pstr2 = '\0';
										sscanf((const char *)pstr,"%d", &ch);
										pstr = pstr2 + 1;
										ret++;
									}
									if(ret == 5)
									{
										g_wifi_aplist_count++;
										if(g_wifi_aplist_stored_num < g_wifi_aplistmax)
										{
											switch(ecn)
											{
											case 0:
												gp_wifi_ap_results->security = WIFI_SECURITY_OPEN;
												break;
											case 1:
												gp_wifi_ap_results->security = WIFI_SECURITY_WEP;
												break;
											case 2:
												gp_wifi_ap_results->security = WIFI_SECURITY_WPA;
												break;
											case 3:
												gp_wifi_ap_results->security = WIFI_SECURITY_WPA2;
												break;
											case 4:
												gp_wifi_ap_results->security = WIFI_SECURITY_WPA2;
												break;
											case 5:
												gp_wifi_ap_results->security = WIFI_SECURITY_UNDEFINED;
												break;
											default:
												gp_wifi_ap_results->security = WIFI_SECURITY_UNDEFINED;
												break;
											}
											strcpy((char *)gp_wifi_ap_results->ssid,(const char *)pssid);
											gp_wifi_ap_results->bssid[0] = mac[0];
											gp_wifi_ap_results->bssid[1] = mac[1];
											gp_wifi_ap_results->bssid[2] = mac[2];
											gp_wifi_ap_results->bssid[3] = mac[3];
											gp_wifi_ap_results->bssid[4] = mac[4];
											gp_wifi_ap_results->bssid[5] = mac[5];
											gp_wifi_ap_results->rssi = rssi;
											gp_wifi_ap_results->channel = ch;
											gp_wifi_ap_results->hidden = 0;
											gp_wifi_ap_results++;
											g_wifi_aplist_stored_num++;
										}
									}
								}
								else
								{

								}
		    				}
							memset(presponse_buff, 0, g_wifi_uart[WIFI_UART_COMMAND_PORT].response_buff_size);
							g_wifi_response_recv_count = 0;
							g_wifi_response_last_string_recv_count = 0;
		    				g_wifi_response_recv_status = 0x0000;
		    			break;
		    			default:
		    				g_wifi_response_recv_status = 0xF001;
		    			break;
		    		}
		    		break;

	    		case 0xF001:	/* other string */
		    		switch(data)
		    		{
		    			case '\r':
		    				g_wifi_response_recv_status = 0xF00F;
		    			break;
		    			default:
		    			break;
		    		}
		    		break;
	    		case 0xF00F:	/* other string */
		    		switch(data)
		    		{
		    			case '\n':
		    				pqueue = wifi_get_current_running_queue();

		    				if(NULL != strstr((const char *)presponse_buff,"WIFI DISCONNECT\r\n"))
		    				{
		    					if(pqueue->at_command_id != ESP32_SET_WIFI_DISCONNECT)
		    					{
									event.event = WIFI_EVENT_WIFI_DISCONNECT;

									memset(&g_wifi_ipconfig, 0, sizeof(g_wifi_ipconfig));
									if(NULL != p_wifi_callback)
									{
										p_wifi_callback(&event);
									}
		    					}
		    				}
							if(NULL != strstr((const char *)presponse_buff,WIFI_RETURN_TEXT_READY))
							{
								if(ESP32_SET_REBOOT == pqueue->at_command_id)
								{
									wifi_set_result_to_current_running_queue( WIFI_RETURN_ENUM_READY );
								}
								else
								{
									event.event = WIFI_EVENT_WIFI_REBOOT;
									if(NULL != p_wifi_callback)
									{
										p_wifi_callback(&event);
									}
								}
							}
							else if(NULL != strstr((const char *)presponse_buff,WIFI_RETURN_TEXT_ERROR))
							{
								wifi_set_result_to_current_running_queue( WIFI_RETURN_ENUM_ERROR );
							}
							else if(NULL != strstr((const char *)presponse_buff,WIFI_RETURN_TEXT_SEND_OK))
                            {
								wifi_set_result_to_current_running_queue( WIFI_RETURN_ENUM_SEND_OK );
							}
							else if(NULL != strstr((const char *)presponse_buff,WIFI_RETURN_TEXT_SEND_FAIL))
							{
								wifi_set_result_to_current_running_queue( WIFI_RETURN_ENUM_SEND_FAIL );
							}
							else if(NULL != strstr((const char *)presponse_buff,WIFI_RETURN_TEXT_READY))
							{
								wifi_set_result_to_current_running_queue( WIFI_RETURN_ENUM_READY );
							}
//							else if(0 == strncmp((const char *)&presponse_buff[2],WIFI_RETURN_TEXT_BUSY,11))
//							{
//								wifi_set_result_to_current_running_queue( WIFI_RETURN_ENUM_BUSY );
//							}
							else if(NULL != strstr((const char *)presponse_buff,WIFI_RETURN_TEXT_OK))
							{
								if(ESP32_SET_SOCKET_SEND_START == pqueue->at_command_id)
								{
			    					/* AT+CIPSENDのOKパターン */
   					    			tmp_recvcnt = g_wifi_response_recv_count;
								}
								else
								{
									/* 通常のOK　*/
									wifi_set_result_to_current_running_queue( WIFI_RETURN_ENUM_OK );
								}
							}
							else if(NULL != strstr((const char *)presponse_buff,",CLOSED\r\n"))
							{
								if(1 == sscanf((const char *)presponse_buff,"%d,CLOSED\r\n",&socket_no))
								{
									if((ESP32_SET_SOCKET_CLOSE != pqueue->at_command_id) || (socket_no != pqueue->socket_no))
									{
										memset(&g_wifi_ipconfig, 0, sizeof(g_wifi_ipconfig));
										event.event = WIFI_EVENT_SOCKET_CLOSED;
										event.socket_no = socket_no;
										if(NULL != p_wifi_callback)
										{
											p_wifi_callback(&event);
										}
									}
								}
		    				}

							else
							{
							}
#if DEBUGLOG >=3
							R_BSP_CpuInterruptLevelWrite (WIFI_CFG_SCI_INTERRUPT_LEVEL-1);
		    				printf("ID:F %s",presponse_buff);
		    				R_BSP_CpuInterruptLevelWrite (0);
#endif
							memset(presponse_buff, 0, g_wifi_uart[WIFI_UART_COMMAND_PORT].response_buff_size);
							g_wifi_response_recv_count = 0;
							g_wifi_response_last_string_recv_count = 0;
		    				g_wifi_response_recv_status = 0x0000;
		    			break;
		    			case '\r':
		    			break;
		    			default:
		    				g_wifi_response_recv_status = 0xF001;
		    			break;
		    		}
		    		break;
			}
    	}
    	else
    	{
    	}
        if(0 != g_wifi_sci_err_flag)
        {
        	switch(g_wifi_sci_err_flag)
        	{
    			case 1:
    				event.event = WIFI_EVENT_SERIAL_RXQ_OVF_ERR;
    				break;
    			case 2:
    				event.event = WIFI_EVENT_SERIAL_OVF_ERR;
    				break;
    			case 3:
    				event.event = WIFI_EVENT_SERIAL_FLM_ERR;
    				break;
        	}
        	g_wifi_sci_err_flag = 0;
    		if(NULL != p_wifi_callback)
    		{
    			p_wifi_callback(&event);
    		}
        }

    }
}
