#pragma once

#include "config.h"

#include "u_sys/thread.h"
#include "u_sys/utils.h" // time
#include "u_sys/cns.h"

#include "u_utils/i2c_dtct.h"	// todo: place i2c dtct into i2c_driver

#include "nvs_flash.h"
#include "sdt.h"

#include "app/app.h"

namespace ufo
{
	class sys
	{
		ufo::Error_t &_error = ufo::Error_t::GetInstance();
		ufo::sys_data_t& _sys = ufo::sys_data_t::get_instanse();

		// place app_cb here???

	public:
		sys() {}
		sys(sys &&) = default;
		~sys()
		{
			// printf("sys destructor\n");
		}

		void app_initialize(){}
		void app_start(){}

		void wrapped_task(ufo::token_t token)
		{
			sys_initialize();

#ifdef UFO_WIFI
#	ifdef UFO_WIFI_DEFAULT_START_AP
			_sys._drv._wifi._ap->create(config::wifi_ap_ss, config::wifi_ap_ps);
			_sys._drv._wifi._ap->ip_config("192.168.0.64", "192.168.0.1","255.255.255.0");
#	else 
#		ifdef UFO_WIFI_DEFAULT_START_STA
			_sys._drv._wifi._sta->connect(config::wifi_sta_ss, config::wifi_sta_ps);
			_sys._drv._wifi._sta->ip_config("192.168.0.68", "192.168.0.1","255.255.255.0");
#		endif
#	endif
#endif
			if (_error)
			{
				_error.Trace();
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
				uint16_t t = _sys._drv._uart0->Available();
                if (t)
                {
					if (!_sys._cns.get_state())
					{
						char s[1] = {}; 
						_sys._drv._uart0->Read(s,1);
	
						if (s[0] == '~')
						{
							_sys._cns.run();

							ufo::cns::console_t cns(_sys._drv._uart0.get());
							
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
				
				if (_error)
				{
					_error.Trace();
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
			_error.SetLevel(ufo::WarningLevel_t::W_1);
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
			_sys._drv._i2c = std::make_unique<drv_t::i2c_t>();
			if (!_sys._drv._i2c)
			{
				// err
				v_fail();
				return false;
			}
			v_done();

#ifdef UFO_I2C_SOFT
			Trace_t::log("_i2cS:");
			_sys._drv._i2cSoft = std::make_unique<drv_t::i2c_t>();
			if (!_sys._drv._i2cSoft)
			{
				v_fail();
				return false;
			}
			v_done();
#endif
			utl::sleep_for(1);
#ifdef UFO_SPI

			Trace_t::log("_spi2:");
			_sys._drv._spi2 = std::make_unique<drv_t::spi_t>();
			if (!_sys._drv._spi2)
			{
				v_fail();
				return false;
			}
			v_done();
			utl::sleep_for(1);

#if (UFO_SPI_CNT > 1)
			Trace_t::log("_spi3:");
			_sys._drv._spi3 = std::make_unique<drv_t::spi_t>();
			if (!_sys._drv._spi3)
			{
				v_fail();
				return false;
			}
			v_done();
#endif
#endif
			utl::sleep_for(1);

			Trace_t::log("_uart0:");
			_sys._drv._uart0 = std::make_unique<drv_t::uart_t>();
			if (!_sys._drv._uart0)
			{
				v_fail();
				return false;
			}
			v_done();
#if (UFO_UART_CNT > 1)
			Trace_t::log("_uart1:");
			_sys._drv._uart1 = std::make_unique<drv_t::uart_t>();
			if (!_sys._drv._uart1)
			{
				v_fail();
				return false;
			}
			v_done();
			utl::sleep_for(1);
#endif

#if (UFO_UART_CNT > 2)
			Trace_t::log("_uart2:");
			_sys._drv._uart2 = std::make_unique<drv_t::uart_t>();
			if (!_sys._drv._uart2)
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
			_sys._drv._wifi._ap = std::make_unique<drv_t::wf_t::ap_t>();
			if (!_sys._drv._wifi._ap)
			{
				v_fail();
				return false;
			}
#	else 
#		ifdef UFO_WIFI_DEFAULT_START_STA
			Trace_t::log("_sta:");
			_sys._drv._wifi._sta = std::make_unique<drv_t::wf_t::sta_t>();
			if (!_sys._drv._wifi._sta)
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
			sys_data_t& _sys = sys_data_t::get_instanse();
			using namespace ufo::drv;
			utl::sleep_for(50);

			Trace_t::log("_i2c:");
			_sys._drv._i2c->Init(ufo::drv::UFO_I2C_port::UFO_I2C_HARDWARE, drv_t::i2c_sda, drv_t::i2c_scl);
			// app._drv._i2c[0] = _sys._drv._i2c->get_status();
			v_done();
			utl::sleep_for(50);

#ifdef UFO_I2C_SOFT
			Trace_t::log("_i2cS:");
			_sys._drv._i2cSoft->Init(ufo::drv::UFO_I2C_port::UFO_I2C_SOFTWARE, drv_t::i2c_soft_sda, drv_t::i2c_soft_scl);
			// app._drv._i2c[1] = _sys._drv._i2cSoft->get_status();
			v_done();
			utl::sleep_for(1);
#endif

			Trace_t::log("_spi2:");
			_sys._drv._spi2->init(spi_host_device_t::SPI2_HOST, drv_t::spi2_mosi, drv_t::spi2_miso, drv_t::spi2_clk);
			// app._drv._spi[0] = _drivers._spi2->get_status();
			v_done();
			utl::sleep_for(1);

#if (UFO_SPI_CNT > 1)
			Trace_t::log("_spi3:");
			_sys._drv._spi3->init(spi_host_device_t::SPI3_HOST, drv_t::spi3_mosi, drv_t::spi3_miso, drv_t::spi3_clk);
			// app._drv._spi[1] = _sys._drv._spi3->get_status();
			v_done();
			utl::sleep_for(1);
#endif

			Trace_t::log("_uart0:");
			_sys._drv._uart0->init(drv_t::uart_t::unum_t::UART_NUM_0, drv_t::uart0_rx, drv_t::uart0_tx);
			// _sys._drv._uart[0] = _sys._drv._uart0->get_status();
			v_done();
			
			_sys._drv._uart0->SetBaudRate(drv_t::uart0_br);
			utl::sleep_for(1);

#if (UFO_UART_CNT > 1)
			Trace_t::log("_uart1:");
			_sys._drv._uart1->init(drv_t::uart_t::unum_t::UART_NUM_1, drv_t::uart1_rx, drv_t::uart1_tx);
			// _sys._drv._uart[1] = _sys._drv._uart1->get_status();
			v_done();
			
			_sys._drv._uart1->SetBaudRate(drv_t::uart1_br);
			utl::sleep_for(1);
#endif
#if (UFO_UART_CNT > 2)
			Trace_t::log("_uart2:");
			_sys._drv->_uart2->init(drv_t::uart_t::unum_t::UART_NUM_2, drv_t::uart2_rx, drv_t::uart2_tx);
			_sys._drv._uart[2] = _sys._drv._uart2->get_status();
			v_done();
			
			_sys._drv._uart2->SetBaudRate(drv_t::uart2_br);
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
			_sys._drv._wifi._ap->enable();
			// _sys._drv._wifi._ap->create(config::wifi_ap_ss, config::wifi_ap_ps);
#	else 
#		ifdef UFO_WIFI_DEFAULT_START_STA
			Trace_t::log("_sta:");
			_sys._drv._wifi._sta->enable();
			// _sys._drv._wifi._sta->connect(config::wifi_sta_ss, config::wifi_sta_ps);
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

			_sys._drv._i2c.reset();
#ifdef UFO_I2C_SOFT
			_sys._drv._i2cSoft.reset();
#endif

			_sys._drv._spi2.reset();
#if (UFO_SPI_CNT > 1)
			_sys._drv._spi3.reset();
#endif

			_sys._drv._uart0.reset();
#if (UFO_UART_CNT > 1)
			_sys._drv._uart1.reset();
#endif
#if (UFO_UART_CNT > 2)
			_sys._drv->_uart2.reset();
#endif

#ifdef UFO_WIFI
#	ifdef UFO_WIFI_DEFAULT_START_AP
			_sys._drv._wifi._ap.reset();
#	else 
#		ifdef UFO_WIFI_DEFAULT_START_STA
			_sys._drv._wifi._sta.reset();
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
			cns.mk_blank(
				"exit",
				"",
				[](cns::console_t::block_t block)
				{
					block->exit();
				});

				cns.mk_blank(
					"dev",
					"",
					[](cns::console_t::block_t block)
					{
						vector_t<string_t> &arg_list = block->get_buf();

						if (!arg_list.empty())
						{
							if (arg_list.size() > 1)
							{
								cns::opt_t opt(arg_list[1]);
								if (opt == 'l' || opt == "list"){
									sys_data_t &_sys = sys_data_t::get_instanse(); // change to pointer
									ufo::i2c_detecter_console(_sys._drv._i2c.get());
									return;
								}
							}
						}
						block->log_incorrect_arg();
					});
				
			cns.mk_blank(
				"wf",
				"",
				[](cns::console_t::block_t block)
				{
					vector_t<string_t> &arg_list = block->get_buf();

					if (!arg_list.empty())
					{
						if (arg_list.size() > 1)
						{
							sys_data_t &_sys = sys_data_t::get_instanse(); // change to pointer
							
							cns::opt_t opt(arg_list[1]);
							if (opt == 'i' || opt == "info"){
								if (_sys._drv._wifi._ap)
								{
									_sys._drv._wifi._ap->log_ipinfo();
								}
								else if (_sys._drv._wifi._sta)
								{
									_sys._drv._wifi._sta->log_ipinfo();
								}
								else
								{
									block->write("no info\n");
								}
							}
							else if (opt == 's' || opt == "set-ip")
							{
								if (opt.arg_count() != 3)
								{
									if (opt.arg_count() == 1)
									{
										if (opt.get_arg(0) == "h")
										{
											block->write("set-ip:\n");
											block->write("use -s/--set-ip=ip,gw,msk\n");
											return;
										}
									}
									block->log_incorrect_arg();
									return;
								}
								
								ip_t ip(opt.get_arg(0).c_str());
								ip_t gw(opt.get_arg(1).c_str());
								ip_t msk(opt.get_arg(2).c_str());
								if (!ip || !msk || !gw)
								{
									block->log_incorrect_arg();
									return;
								}

								// todo
								if (_sys._drv._wifi._ap)
								{
									_sys._drv._wifi._ap->ip_config(ip, gw, msk);
								}
								else if (_sys._drv._wifi._sta)
								{
									_sys._drv._wifi._sta->ip_config(ip, gw, msk);
								}
								else
								{
									block->write("no info\n");
									return;
								}
								block->write("wf configuring...\n");

								/// todo =============
								if (_sys._drv._wifi._ap)
								{
									_sys._drv._wifi._ap->log_ipinfo();
								}
								else if (_sys._drv._wifi._sta)
								{
									_sys._drv._wifi._sta->log_ipinfo();
								}
								else
								{
									block->write("no info\n");
								}
								/// ==================
								return;
							}
						}
					}
					block->log_incorrect_arg();
				});
		}
	};
} // ufo