#pragma once

#include "config.h"
#include "u_drivers/drv.h"
#include "types.h"

namespace ufo
{
    class sys_data_t
    {
    public:
        types::ver_t _ver = {};
        types::cns_t _cns = {};
        drv::drv_t _drv = {};

            // // !!! read only !!!    // setting in app-task  // reading in a lot of parts 
            // struct drv_t
            // {
            //     dev_status_t _i2c[config::ufo_i2c_supported] =  {};   // 
            //     dev_status_t _spi[config::ufo_spi_supported] =  {};   // spi1 ureachable {spi2, spi3};
            //     dev_status_t _uart[config::ufo_uart_supported] =  {}; // means user initialization of uart (not system)   
            //     dev_status_t _wf = dev_status_t::undef;   
            // };

    private:
        sys_data_t()
        {
            _ver = {"ufomir", config::version_maj, config::version_min};
        }
    public:
        static sys_data_t &get_instanse()
        {
            static sys_data_t d;
            return d;
        }
    };

} // namespace ufo
