#pragma once

#include "config.h"

#include "u_sys/thread.h"
#include "u_sys/utils.h" // time
#include "u_sys/cns.h"

#include "u_utils/i2c_dtct.h"	// todo: place i2c dtct into i2c_driver

#include "nvs_flash.h"
#include "sdt.h"
#include "syscns.h"

#include "app/app.h"

namespace ufo
{
	class sys
	{

	public:
		sys() {}
		sys(sys &&) = default;
		~sys()
		{}

		void app_initialize(){}
		void app_start(){}

#ifdef UFO_WIFI
		void wifi_initialize(){
			using s_t =types::wifi_ctrl_t::wifi_states_t;
			auto& s = __global_system_data._wifi_ctl._states;
			if (!s.get(s_t::enable))
			{
				return;
			}
			
			// check stupid error
			if (s.get(s_t::ap_enable, s_t::sta_enable))
			{
				return;
			}
			
			if (s.get(s_t::ap_enable))
			{
				__global_system_data._drv._wifi._ap->create(UFO_WIFI_DEFAULT_AP_BSSD,UFO_WIFI_DEFAULT_AP_PASS);

				__global_system_data._wifi_ctl._pass = UFO_WIFI_DEFAULT_AP_PASS;
				__global_system_data._wifi_ctl._ssid = UFO_WIFI_DEFAULT_AP_BSSD;
				
				if (s.get(s_t::static_enable))
				{
					__global_system_data._drv._wifi._ap->ip_config(
						UFO_WIFI_DEFAULT_AP_IPSTATIC_IP, 
						UFO_WIFI_DEFAULT_AP_IPSTATIC_GATE, 
						UFO_WIFI_DEFAULT_AP_IPSTATIC_MASK
					);

					// __global_system_data._wifi_ctl._ip_config = __global_system_data._drv._wifi._ap->get_ip_config();

				}
			}
			else if (s.get(s_t::sta_enable))
			{
				__global_system_data._drv._wifi._sta->connect(UFO_WIFI_DEFAULT_STA_BSSD,UFO_WIFI_DEFAULT_STA_PASS);
				__global_system_data._wifi_ctl._pass = UFO_WIFI_DEFAULT_STA_PASS;
				__global_system_data._wifi_ctl._ssid = UFO_WIFI_DEFAULT_STA_BSSD;
				
				if (s.get(s_t::static_enable))
				{
					__global_system_data._drv._wifi._sta->ip_config(
						UFO_WIFI_DEFAULT_STA_IPSTATIC_IP, 
						UFO_WIFI_DEFAULT_STA_IPSTATIC_GATE, 
						UFO_WIFI_DEFAULT_STA_IPSTATIC_MASK
					);

					// __global_system_data._wifi_ctl._ip_config = __global_system_data._drv._wifi._sta->get_ip_config();
				}
			}
			else
			{
				s.upd(0);
			}
		}
#endif

		void wrapped_task(ufo::token_t token)
		{
			sys_initialize();

#ifdef UFO_WIFI

			wifi_initialize();
// #	ifdef UFO_WIFI_DEFAULT_START_AP
// 			__global_system_data._drv._wifi._ap->create(config::wifi_ap_ss, config::wifi_ap_ps);
// 			__global_system_data._drv._wifi._ap->ip_config("192.168.0.64", "192.168.0.1","255.255.255.0");
// #	else 
// #		ifdef UFO_WIFI_DEFAULT_START_STA
// 			__global_system_data._drv._wifi._sta->connect(config::wifi_sta_ss, config::wifi_sta_ps);
// 			__global_system_data._drv._wifi._sta->ip_config("192.168.0.68", "192.168.0.1","255.255.255.0");
// #		endif
// #	endif
#endif
			if (__global_error)
			{
				__global_error.Trace();
				return;
			}

			ufo::thread_cfg app_cfg;
			app_cfg._name = "app";
			app_cfg._core = 0;
			app_cfg._prio = 5;
			app_cfg._stackSize = 4096;
			app::app_t app_cb;
			ufo::thread_guard app_task(ufo::thread(app_cfg, &app::app_t::task, &app_cb));

			while (token)
			{
				uint16_t t = __global_system_data._drv._uart0->Available();
                if (t)
                {
					ufo::bit_flag_t<uint8_t> s = __global_system_data._cns.get_state();
					using cns_st_t = ufo::types::cns_t::cns_state_t;
					if (!s.get(cns_st_t::started))
					{
						if (s.get(cns_st_t::bloked))
						{
							__global_system_data._drv._uart0->Flush();
						}
						else
						{
							char s[1] = {};
							__global_system_data._drv._uart0->Read(s, 1);

							if (s[0] == '~')
							{
								__global_system_data._cns.run();	// cns should call .stop in end of ctask

								ufo::cns::console_t cns(__global_system_data._drv._uart0.get());
								
								cns_init(cns);
								app_cb.cns_init(cns);

								ufo::thread_cfg cfg_cns;
								cfg_cns._name = "cns";
								cfg_cns._core = 0;
								cfg_cns._prio = 5;
								cfg_cns._stackSize = 4096;
								ufo::thread tt(cfg_cns, &ufo::cns::console_t::ctask, std::move(cns));
								tt.detach();
							}
						}
					}
                }
				
				if (__global_error)
				{
					__global_error.Trace();
					break;
				}
				ufo::utl::sleep_for(100);
			}

			driver_deinitialize();
		}

	private:
		void sys_initialize()
		{
			utl::sleep_for(1);
			printf("system starting\n");
			__global_error.SetLevel(ufo::WarningLevel_t::W_1);
			bool e = driver_initialize();
			if (!e)
			{
				return;
			}
			driver_start();
		}

		bool driver_initialize()
		{
			Trace_t::log("driver-initialize::start\n");
			using namespace ufo::drv;
			Trace_t::log("_i2c:");
			__global_system_data._drv._i2c = std::make_unique<drv_t::i2c_t>();
			if (!__global_system_data._drv._i2c)
			{
				// err
				v_fail();
				return false;
			}
			v_done();

#ifdef UFO_I2C_SOFT
			Trace_t::log("_i2cS:");
			__global_system_data._drv._i2cSoft = std::make_unique<drv_t::i2c_t>();
			if (!__global_system_data._drv._i2cSoft)
			{
				v_fail();
				return false;
			}
			v_done();
#endif
			utl::sleep_for(1);
#ifdef UFO_SPI

			Trace_t::log("_spi2:");
			__global_system_data._drv._spi2 = std::make_unique<drv_t::spi_t>();
			if (!__global_system_data._drv._spi2)
			{
				v_fail();
				return false;
			}
			v_done();
			utl::sleep_for(1);

#if (UFO_SPI_CNT > 1)
			Trace_t::log("_spi3:");
			__global_system_data._drv._spi3 = std::make_unique<drv_t::spi_t>();
			if (!__global_system_data._drv._spi3)
			{
				v_fail();
				return false;
			}
			v_done();
#endif
#endif
			utl::sleep_for(1);

			Trace_t::log("_uart0:");
			__global_system_data._drv._uart0 = std::make_unique<drv_t::uart_t>();
			if (!__global_system_data._drv._uart0)
			{
				v_fail();
				return false;
			}
			v_done();
#if (UFO_UART_CNT > 1)
			Trace_t::log("_uart1:");
			__global_system_data._drv._uart1 = std::make_unique<drv_t::uart_t>();
			if (!__global_system_data._drv._uart1)
			{
				v_fail();
				return false;
			}
			v_done();
			utl::sleep_for(1);
#endif

#if (UFO_UART_CNT > 2)
			Trace_t::log("_uart2:");
			__global_system_data._drv._uart2 = std::make_unique<drv_t::uart_t>();
			if (!__global_system_data._drv._uart2)
			{
				v_fail();
				return false;
			}
			v_done();
			utl::sleep_for(1);
#endif

#ifdef UFO_WIFI
Trace_t::log("_wifi.");
#	ifdef UFO_WIFI_DEFAULT_START_AP
			Trace_t::log("_ap:");
			__global_system_data._drv._wifi._ap = std::make_unique<drv_t::wf_t::ap_t>();
			if (!__global_system_data._drv._wifi._ap)
			{
				v_fail();
				return false;
			}
#	else 
#		ifdef UFO_WIFI_DEFAULT_START_STA
			Trace_t::log("_sta:");
			__global_system_data._drv._wifi._sta = std::make_unique<drv_t::wf_t::sta_t>();
			if (!__global_system_data._drv._wifi._sta)
			{
				v_fail();
				return false;
			}
#		endif
#	endif
#endif
			v_done();
			Trace_t::log("driver-initialize::finish\n");
			
			utl::sleep_for(10);
			return true;
		}

		void driver_start()
		{
			Trace_t::log("driver-start::start\n");
			using namespace ufo::drv;
			utl::sleep_for(50);

			Trace_t::log("_i2c:");
			__global_system_data._drv._i2c->Init(ufo::drv::UFO_I2C_port::UFO_I2C_HARDWARE, drv_t::i2c_sda, drv_t::i2c_scl);
			// app._drv._i2c[0] = __global_system_data._drv._i2c->get_status();
			v_done();
			utl::sleep_for(50);

#ifdef UFO_I2C_SOFT
			Trace_t::log("_i2cS:");
			__global_system_data._drv._i2cSoft->Init(ufo::drv::UFO_I2C_port::UFO_I2C_SOFTWARE, drv_t::i2c_soft_sda, drv_t::i2c_soft_scl);
			// app._drv._i2c[1] = __global_system_data._drv._i2cSoft->get_status();
			v_done();
			utl::sleep_for(1);
#endif

#ifdef UFO_SPI


			Trace_t::log("_spi2:");
			__global_system_data._drv._spi2->init(spi_host_device_t::SPI2_HOST, drv_t::spi2_mosi, drv_t::spi2_miso, drv_t::spi2_clk);
			// app._drv._spi[0] = _drivers._spi2->get_status();
			v_done();
			utl::sleep_for(1);

#if (UFO_SPI_CNT > 1)
			Trace_t::log("_spi3:");
			__global_system_data._drv._spi3->init(spi_host_device_t::SPI3_HOST, drv_t::spi3_mosi, drv_t::spi3_miso, drv_t::spi3_clk);
			// app._drv._spi[1] = __global_system_data._drv._spi3->get_status();
			v_done();
			utl::sleep_for(1);
#endif
#endif

			Trace_t::log("_uart0:");
			__global_system_data._drv._uart0->init(drv_t::uart_t::unum_t::UART_NUM_0, drv_t::uart0_rx, drv_t::uart0_tx);
			// __global_system_data._drv._uart[0] = __global_system_data._drv._uart0->get_status();
			v_done();
			
			__global_system_data._drv._uart0->SetBaudRate(drv_t::uart0_br);
			utl::sleep_for(1);

#if (UFO_UART_CNT > 1)
			Trace_t::log("_uart1:");
			__global_system_data._drv._uart1->init(drv_t::uart_t::unum_t::UART_NUM_2, drv_t::uart1_rx, drv_t::uart1_tx);
			// __global_system_data._drv._uart[1] = __global_system_data._drv._uart1->get_status();
			v_done();
			
			__global_system_data._drv._uart1->SetBaudRate(drv_t::uart1_br);
			utl::sleep_for(1);
#endif
#if (UFO_UART_CNT > 2)
			Trace_t::log("_uart2:");
			__global_system_data._drv->_uart2->init(drv_t::uart_t::unum_t::UART_NUM_2, drv_t::uart2_rx, drv_t::uart2_tx);
			__global_system_data._drv._uart[2] = __global_system_data._drv._uart2->get_status();
			v_done();
			
			__global_system_data._drv._uart2->SetBaudRate(drv_t::uart2_br);
			utl::sleep_for(1);
#endif


#ifdef UFO_WIFI
			utl::sleep_for(1);
			esp_err_t ret = nvs_flash_init();
			if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
			{
				ESP_ERROR_CHECK(nvs_flash_erase());
				ret = nvs_flash_init();
				if (ret != ESP_OK)
				{
					//todo
				}
			}
			utl::sleep_for(1);
			Trace_t::log("wifi.");
#	ifdef UFO_WIFI_DEFAULT_START_AP
			Trace_t::log("_ap:");
			__global_system_data._drv._wifi._ap->enable();
			// __global_system_data._drv._wifi._ap->create(config::wifi_ap_ss, config::wifi_ap_ps);
#	else 
#		ifdef UFO_WIFI_DEFAULT_START_STA
			Trace_t::log("_sta:");
			__global_system_data._drv._wifi._sta->enable();
			// __global_system_data._drv._wifi._sta->connect(config::wifi_sta_ss, config::wifi_sta_ps);
#		endif
#	endif
			v_done();
#endif
			utl::sleep_for(1);

			Trace_t::log("driver-start::finish\n");
		}

		void driver_deinitialize()
		{
			printf("drivers-deini\n");

			__global_system_data._drv._i2c.reset();
#ifdef UFO_I2C_SOFT
			__global_system_data._drv._i2cSoft.reset();
#endif

			__global_system_data._drv._spi2.reset();
#if (UFO_SPI_CNT > 1)
			__global_system_data._drv._spi3.reset();
#endif

			__global_system_data._drv._uart0.reset();
#if (UFO_UART_CNT > 1)
			__global_system_data._drv._uart1.reset();
#endif
#if (UFO_UART_CNT > 2)
			__global_system_data._drv->_uart2.reset();
#endif

#ifdef UFO_WIFI
#	ifdef UFO_WIFI_DEFAULT_START_AP
			__global_system_data._drv._wifi._ap->disable();
			__global_system_data._drv._wifi._ap.reset();
#	else 
#		ifdef UFO_WIFI_DEFAULT_START_STA
			__global_system_data._drv._wifi._sta->disable();
			__global_system_data._drv._wifi._sta.reset();
#		endif
#	endif
			nvs_flash_deinit();
#endif
		}

		void v_done() const {
			Trace_t::log("done\n");
		}
		void v_fail() const {
			Trace_t::log("fail\n");
		}

	

		// system console blanks
		// !there are cant be appdata!
		// todo: sys (-i, -m ...)
		void cns_init(ufo::cns::console_t & cns){

			using namespace ufo::sysconcole;

			cns.mk_blank("exit", "", 
				[](cns::console_t::block_t& block)
				{
					block->exit();
				});

			cns.mk_blank("cdev", "print devices info", console_dev);

#ifdef UFO_WIFI
			cns.mk_blank("wf", "wifi configurator", console_wifi);
#endif
		}
	};
} // ufo