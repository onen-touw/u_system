#pragma once

namespace ufo
{
    // first defenitions of Error_t
    class Error_t;

    namespace error
    {

        enum class codes_t{
            unknown = 0,
            internal_generic,       // for both errors and warnings
            app_data_error,
            
            mux_create = 111,
            
            trd_destruction = 131,  // thread
            trd_id = 132,
            trd_creation = 133,
            trd_null_opertions = 134,
            trd_join_detached = 135,
            trd_detach_detached = 136,
            trd_join_unj = 137,

            net_bad_ip = 141,
            net_bad_sock = 142,             // lwip_socket return negative descriptor
            net_snd_err = 143,             // lwip_socket return negative descriptor

            i2c_null_operations = 501,      // i2c-driver not inited but try to do smth
            i2c_incr_pins,      // i2c-driver not inited but try to do smth
            i2c_conf_fail,            // i2c-driver 
            i2c_instl_fail,           // i2c-driver 
            i2c_tcfg_fail,           // i2c-driver 
            i2c_write,                // i2c-driver write-operation return bad-status
                                            //  it code can use like warning or counting
            i2c_write_read,           // i2c-driver write-read-operation return bad-status
                                            //  it code can use like warning or counting
            i2c_driver_del,           // i2c-driver delete-operation return bad-status
            i2c_not_supported,           // 
            i2c,           // 
            

            wf_drv_init = 511,
            wf_drv,
            wf_disable,
            wf_create,
            wf_clear,
            wf_ip,

            urt_drv_install    = 521,
            urt_drv_cfg,
            urt_drv_pin,
            urt_drv_tmout,

            pwm_ledc_cfg = 531,
            pwm_ledc_timer,
            pwm_gpio_cfg,

            spi_drv_init = 541,

        };      
    } // namespace error
    
    
} // namespace ufo
