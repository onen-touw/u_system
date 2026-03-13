#pragma once

#include"../u_platfom.h"
#include "app/appconfig.h"

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"


// #define UFO_ERROR_TRACE_BG_COLORED
//#define UFO_ERROR_USE_TIMESTAMP
//#define UFO_ERROR_USE_TAG



#define UFO_THREAD_NAME 1
#define UFO_THREAD_INFO 1
#define UFO_THREAD_DEPENDENCIES 1

namespace ufo {

	class config
	{
	public:
// #ifdef UFO_WIFI
// #	ifdef UFO_WIFI_DEFAULT_START_AP
// 		static constexpr const char* wifi_ap_ss = "u_esp32";
// 		static constexpr const char* wifi_ap_ps = "12345678";
// #	else 
// #		ifdef UFO_WIFI_DEFAULT_START_STA
// 		static constexpr const char* wifi_sta_ss = "u_esp32";
// 		static constexpr const char* wifi_sta_ps = "12345678";
// #		endif
// #	endif
// #endif

		static constexpr uint8_t Counting_limit = 5;
		static constexpr uint8_t Counting_capacity = 5;
		static constexpr uint8_t Warning_capacity = 3;
		static constexpr uint16_t CountingErrorLiveTime = 30;     // in seconds	//todo:: second_t
		

		static constexpr uint32_t MutexTickWait = UINT16_MAX;

		static constexpr uint8_t ufo_i2c_supported = 1;
		static constexpr uint8_t ufo_spi_supported = 1;
		static constexpr uint8_t ufo_uart_supported = 3;

		static constexpr uint8_t version_maj = 0;
		static constexpr uint8_t version_min = 1;
	};


	enum class dev_status_t : uint8_t
	{
		undef = 0,
		bad_status,
		ok,
		off,
		error,
		warning,
	};
}	// ufo


