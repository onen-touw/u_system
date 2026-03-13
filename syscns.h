#pragma once

#include "config.h"
#include "u_sys/cns.h"
#include "u_sys/utils.h" // time
#include "u_sys/thread.h"
#include "sdt.h"

namespace ufo
{
    namespace sysconcole
    {

        void console_task() {
            
        }




#ifdef UFO_WIFI
        void console_wifi(cns::console_t::block_t &block)
        {
            vector_t<string_t> &arg_list = block->get_buf();

            if (!arg_list.empty())
            {
                if (arg_list.size() > 1)
                {
                    cns::opt_t opt(arg_list[1]);
                    if (opt == 'i' || opt == "info")
                    {
                        if (__global_system_data._drv._wifi._ap)
                        {
                            __global_system_data._drv._wifi._ap->log_ipinfo();
                        }
                        else if (__global_system_data._drv._wifi._sta)
                        {
                            __global_system_data._drv._wifi._sta->log_ipinfo();
                        }
                        else
                        {
                            block->write("no info\n");
                        }
                        return;
                    }
                    else if (opt == "ipconfig")
                    {
                        if (opt.arg_count() != 3)
                        {
                            if (opt.arg_count() == 1)
                            {
                                if (opt.get_arg(0) == "dhcp")
                                {
                                    block->write("wf dhcp configuring...\n");
                                    
                                    if (__global_system_data._drv._wifi._ap)
                                    {
                                        __global_system_data._drv._wifi._ap->ip_config(uint32_t(0),uint32_t(0),uint32_t(0));
                                        __global_system_data._drv._wifi._ap->log_ipinfo();
                                    }
                                    else if (__global_system_data._drv._wifi._sta)
                                    {
                                        __global_system_data._drv._wifi._sta->ip_config(uint32_t(0),uint32_t(0),uint32_t(0));
                                        __global_system_data._drv._wifi._sta->log_ipinfo();
                                    }
                                    else
                                    {
                                        block->write("wf want started succesfuly\n");
                                        return;
                                    }
                                    return;
                                }                              
                                block->log_incorrect_arg();
                                return;
                            }
                            else if (opt.arg_count() == 0)
                            {
                                if (arg_list.size() > 2)
                                {
                                    ufo::cns::opt_t help = arg_list[2];

                                    if (help == 'h' || help == "h" || help == "help")
                                    {
                                        block->write("usage:\n\t");
                                        block->write("wf --ipconfig=dhsp or ={ip},{gw},{mask}\n\t");
                                        block->write("wf --ipconfig --help for see this info\n");
                                        return;
                                    }
                                }
                                block->log_incorrect_arg();
                                return;
                            }
                        }

                        ip_t ip(opt.get_arg(0).c_str());
                        ip_t gw(opt.get_arg(1).c_str());
                        ip_t msk(opt.get_arg(2).c_str());
                        if (!ip || !msk || !gw)
                        {
                            block->log_incorrect_arg();
                            return;
                        }

                        block->write("wf configuring...\n");

                        // todo
                        using s_t = types::wifi_ctrl_t::wifi_states_t;

                        
                        if (__global_system_data._wifi_ctl._states.get(s_t::ap_enable))
                        {
                            if (__global_system_data._drv._wifi._ap)
                            {
                                __global_system_data._drv._wifi._ap->ip_config(ip, gw, msk);
                                __global_system_data._drv._wifi._ap->log_ipinfo();
                                return;
                            }
                        }
                        else if (__global_system_data._wifi_ctl._states.get(s_t::sta_enable))
                        {
                            if (__global_system_data._drv._wifi._sta)
                            {
                                __global_system_data._drv._wifi._sta->ip_config(ip, gw, msk);
                                __global_system_data._drv._wifi._sta->log_ipinfo();
                                return;
                            }
                            
                        }
                        else
                        {
                            block->write("wf want started succesfuly\n");
                            return;
                        }
                        return;
                    }
                    else if (opt == 'e' || opt == "enable")
                    {
                        char md = '\0';
                        string_t tp;
                        string_t bs;
                        string_t pass;

                        if (opt.arg_count() > 0)
                        {
                            tp = opt.get_arg(0);
                        }

                        if (tp == "ap")
                        {
                            md = 'a';
                        }
                        else if (tp == "sta")
                        {
                            md = 's';
                        }
                        else
                        {
                            block->write("incorrect mode use -h for info\n");
                            return;
                        }

                        if (opt.arg_count() == 3)
                        {
                            bs = opt.get_arg(1);
                            pass = opt.get_arg(2);
                        }
                        else if (opt.arg_count() == 1)
                        {
#if UFO_WIFI_USE_DEFAULT_CONFIG
                            block->write("use default params\n");
                            if (md == 'a')
                            {
                                bs = UFO_WIFI_DEFAULT_AP_BSSD;
                                pass = UFO_WIFI_DEFAULT_AP_PASS;
                            }
                            else
                            {
                                bs = UFO_WIFI_DEFAULT_STA_BSSD;
                                pass = UFO_WIFI_DEFAULT_STA_PASS;
                            }
#else
                            block->write("default params are not availvable\n");
                            return;
#endif
                        }
                        else
                        {
                            block->log_incorrect_arg();
                            return;
                        }
                        
                        if (bs.empty())
                        {
                            block->write("bad bssd\n");
                            return;
                        }
                        if (pass.empty())
                        {
                            block->write("bad bssd\n");
                            return;
                        }

                        using s_t = types::wifi_ctrl_t::wifi_states_t;
                        
                        if (md == 'a')
                        {
                            if (__global_system_data._drv._wifi._sta)
                            {
                                block->fwrite("shutsown sta and start ap ");
                                __global_system_data._drv._wifi._sta->disable();
                                __global_system_data._drv._wifi._sta.reset();
                            }
                            if (__global_system_data._drv._wifi._ap)
                            {
                                block->fwrite("restart ap ");
                                __global_system_data._drv._wifi._ap->disable();
                            }
                            else
                            {
                                block->fwrite("starting ap ");
                                __global_system_data._drv._wifi._ap = std::make_unique<drv::drv_t::wf_t::ap_t>();
                            }
                            utl::sleep_for(10);                                        
                            block->fwrite("with %s %s\n", bs.c_str(), pass.c_str());
                            __global_system_data._drv._wifi._ap->enable();
                            __global_system_data._drv._wifi._ap->create(bs.c_str(), pass.c_str());
                            __global_system_data._drv._wifi._ap->log_ipinfo();
                            __global_system_data._wifi_ctl._states.set(s_t::ap_enable);
                        }
                        else if (md == 's')
                        {
                            if (__global_system_data._drv._wifi._ap)
                            {
                                block->fwrite("shutsown ap and start sta ");
                                __global_system_data._drv._wifi._ap->disable();
                                __global_system_data._drv._wifi._sta.reset();
                            }
                            if (__global_system_data._drv._wifi._sta)
                            {
                                block->fwrite("restart sta ");
                                __global_system_data._drv._wifi._sta->disable();
                            }
                            else
                            {
                                block->fwrite("starting sta ");
                                __global_system_data._drv._wifi._sta = std::make_unique<drv::drv_t::wf_t::sta_t>();
                            }

                                utl::sleep_for(10);                                        
                            block->fwrite("with %s %s\n", bs.c_str(), pass.c_str());
                            __global_system_data._drv._wifi._sta->enable();
                            __global_system_data._drv._wifi._sta->connect(bs.c_str(), pass.c_str());
                            __global_system_data._drv._wifi._sta->log_ipinfo();
                            __global_system_data._wifi_ctl._states.set(s_t::sta_enable);
                        }
                        return;
                    }
                    else if (opt == 'd' || opt == "disable")
                    {
                        using s_t = types::wifi_ctrl_t::wifi_states_t;

                        if (__global_system_data._drv._wifi._ap)
                        {
                            __global_system_data._drv._wifi._ap->disable();
                            __global_system_data._wifi_ctl._states.unset(s_t::ap_enable);
                        }
                        else if (__global_system_data._drv._wifi._sta)
                        {
                            __global_system_data._drv._wifi._sta->disable();
                            __global_system_data._wifi_ctl._states.unset(s_t::sta_enable);
                        }
                        else
                        {
                            block->write("wf want started succesfuly\n");
                        }
                        return;
                    }
                }
            }
            block->log_incorrect_arg();
        }
#endif

    void console_dev(cns::console_t::block_t& block)
    {
        vector_t<string_t> &arg_list = block->get_buf();

        if (!arg_list.empty())
        {
            if (arg_list.size() > 1)
            {
                cns::opt_t opt(arg_list[1]);
                if (opt == 'l' || opt == "list")
                {
                    ufo::i2c_detecter_console(__global_system_data._drv._i2c.get());
                    return;
                }
            }
        }
        block->log_incorrect_arg();
    }

    } // namespace sysconcole
} // namespace ufo
