MODULE_NAME_LIST = 	r_bsp r_byteq r_cmt_rx r_ether_rx r_flash_rx r_riic_rx r_s12ad_rx r_sci_iic_rx\
					r_sci_rx r_emwin_rx r_drw2d_rx r_simple_glcdc_config_rx r_simple_graphic_rx r_t4_http_server_rx r_t4_dns_client_rx\
					r_t4_ftp_server_rx r_t4_file_driver_rx r_t4_rx r_t4_driver_rx r_t4_sntp_client_rx r_wifi_rx

all:
	for i in $(MODULE_NAME_LIST); do \
		cd ./source/$$i/; \
		make; \
		cd ../../; \
	done
		
clean:
	for i in $(MODULE_NAME_LIST); do \
		cd ./source/$$i/; \
		make clean; \
		cd ../../; \
	done
