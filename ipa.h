#pragma once 

#include "config.h"
#include "lwip/sockets.h"
#include "error.h"
#include "u_sys/utils.h"
#include "u_sys/ipt.h"

namespace ufo
{
    
    namespace net
    {
        // ip
        class ipa
        {
        protected:
            sockaddr_in _addr;
        public:
            // ipv4
            ipa(){}

            ipa(uint32_t raw)
            {
                _addr.sin_addr.s_addr = lwip_htonl(raw);
            }

            // ipv4
            ipa(uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4)
            {
                ip_t ip(a1,a2,a3,a4);
                _addr.sin_addr.s_addr = ip.get();
            }

            ipa(const char *addr) 
            {
                ip_t ip(addr);
                _addr.sin_addr.s_addr = ip.get();
            }

            ~ipa() {}
            
            void log(){
                uint32_t a = _addr.sin_addr.s_addr;
                ip_t ip(a);
                ip.log();
            }
            
            sockaddr_in& get_native(){
                return _addr;
            }

            uint32_t operator ()() const {
                return _addr.sin_addr.s_addr;
            }

            operator bool(){
                return _addr.sin_addr.s_addr > 0;
            }
        };

    } // namespace net
    

} // namespace ufo



