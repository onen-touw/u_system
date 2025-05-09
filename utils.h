#pragma once

#include "config.h"
#include "esp_timer.h"
namespace ufo
{
	namespace utl
	{
		static constexpr uint8_t SetBit(uint8_t n)
		{
			uint8_t i = 1 << n;
			return i;
		}

		inline void sleep_for(uint32_t ms)
		{
			vTaskDelay(ms / portTICK_PERIOD_MS);
		}

		uint64_t get_time_micros(){
			return esp_timer_get_time(); 
		}
		uint32_t get_time_millis(){
			return esp_timer_get_time()/1000; 
		}

		template <typename Ty>
		Ty map(Ty _val, Ty _inmin, Ty _inmax, Ty _outmin, Ty _outmax)
		{

			Ty run = _inmax - _inmin;
			if (run == 0)
			{
				// log_e("map(): Invalid input range, min == max");
				return -1; // AVR returns -1, SAM returns 0
			}
			Ty rise = _outmax - _outmin;
			Ty delta = _val - _inmin;
			return (delta * rise) / run + _outmin;
		}

		template <typename Ty>
		Ty constrain(Ty _in, Ty _min, Ty _max)
		{
			return _in > _max ? _max : _in < _min ? _min: _in;
		}

	} // utl

} // ufo