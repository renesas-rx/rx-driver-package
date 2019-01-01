MODULE_NAME_LIST = 	r_bsp r_byteq r_cmt_rx r_ether_rx r_flash_rx r_riic_rx r_s12ad_rx r_sci_iic_rx r_sci_rx r_emwin_rx

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
