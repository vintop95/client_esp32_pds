deps_config := \
	/home/vince/esp/esp-idf/components/app_trace/Kconfig \
	/home/vince/esp/esp-idf/components/aws_iot/Kconfig \
	/home/vince/esp/esp-idf/components/bt/Kconfig \
	/home/vince/esp/esp-idf/components/driver/Kconfig \
	/home/vince/esp/esp-idf/components/esp32/Kconfig \
	/home/vince/esp/esp-idf/components/esp_adc_cal/Kconfig \
	/home/vince/esp/esp-idf/components/esp_http_client/Kconfig \
	/home/vince/esp/esp-idf/components/ethernet/Kconfig \
	/home/vince/esp/esp-idf/components/fatfs/Kconfig \
	/home/vince/esp/esp-idf/components/freertos/Kconfig \
	/home/vince/esp/esp-idf/components/heap/Kconfig \
	/home/vince/esp/esp-idf/components/http_server/Kconfig \
	/home/vince/esp/esp-idf/components/libsodium/Kconfig \
	/home/vince/esp/esp-idf/components/log/Kconfig \
	/home/vince/esp/esp-idf/components/lwip/Kconfig \
	/home/vince/esp/esp-idf/components/mbedtls/Kconfig \
	/home/vince/esp/esp-idf/components/mdns/Kconfig \
	/home/vince/esp/esp-idf/components/openssl/Kconfig \
	/home/vince/esp/esp-idf/components/pthread/Kconfig \
	/home/vince/esp/esp-idf/components/spi_flash/Kconfig \
	/home/vince/esp/esp-idf/components/spiffs/Kconfig \
	/home/vince/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/home/vince/esp/esp-idf/components/vfs/Kconfig \
	/home/vince/esp/esp-idf/components/wear_levelling/Kconfig \
	/home/vince/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/vince/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/vince/esp/esp-idf/examples/wifi/getting_started/softAP/main/Kconfig.projbuild \
	/home/vince/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/vince/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)

ifneq "$(IDF_CMAKE)" "n"
include/config/auto.conf: FORCE
endif

$(deps_config): ;
