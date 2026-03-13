#pragma once

#include "config.h"
#include "mutex"
#include "btflg.h"

namespace ufo
{
    namespace types
    {
        /* 
            module      module code     error-codes
            sys:        [0  -  200]
                error{er}:  0 
                    error_codes:        [1 - 99]
                trace{trc}:     100
                    error_codes:        [101- 109]
                mutex{mux}:     110
                    error_codes:        [111- 119]
                utils{utl}:     120 
                    error_codes:        [121- 129]
                thread{trd}:    130
                    error_codes:        [131- 139]
                net{socket, ipa}:140
                    error_codes:        [141- 149]
                
                ...
            driver:     [500 - 700]
                i2c     500             [501...509]
                wf      510             [511...519]
                urt     510             [521...529]
                pwm     530             [523...539]
                ... 
                ...                          ... 700]

            sensors:    [1000 - 1500]
                ina     1000
                ...
            free:       [2000 - 2500]
        */

        enum class sys_code_t {
            er = 0,
            trc = 100,
            mux = 110,
            utl = 120,
            trd = 130,
            // net = 140
        };

        enum class driver_code_t
        {
            i2c0 = 501u, // hard
            i2c1,        // -soft
            spi0,        // int-mem
            spi1,        // hard-1
            spi2,        // ..??
            spi3,        // ..??
            uart0,       // hard uart_0 pins(rx, tx)
            uart1,       // hard
            uart2,       // hard
            upwm,        // hard ufo-pwm
        };

        struct ver_t
        {
            uint8_t _ver_maj = 0;
            uint8_t _ver_min = 0;
            char _dev_name[8] = {};

            ver_t() {}

            ver_t(const char *name, uint8_t vmaj, uint8_t vmin) : _ver_maj(vmaj), _ver_min(vmin)
            {
                memcpy(_dev_name, name, 8);
            }
        };

        struct cns_t
        {
            enum class cns_state_t {
                started,
                bloked,
            };

            ufo::mutex_t _lock;
            ufo::bit_flag_t<uint8_t> _states;
            // bool started = false;

            void run(){
                ufo::lock_guard<mutex_t>_l(_lock);
                _states.set(cns_state_t::started);
                // started = true;
            }

            void stop(){
                ufo::lock_guard<mutex_t>_l(_lock);
                _states.unset(cns_state_t::started);
                // started = false;
            }

            // lock for start
            void block() {
                ufo::lock_guard<mutex_t>_l(_lock);
                _states.set(cns_state_t::bloked);
            }

            void unlock() {
                ufo::lock_guard<mutex_t>_l(_lock);
                _states.unset(cns_state_t::bloked);
            }

            const ufo::bit_flag_t<uint8_t>&  get_state() const {
                return _states;
            }

            bool can_run() {
                ufo::lock_guard<mutex_t>_l(_lock);
                bool b = !(_states.get(cns_state_t::bloked) || _states.get(cns_state_t::started));
                return b;
            }
        };

    } // namespace types

} // namespace ufo



