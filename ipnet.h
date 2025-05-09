#pragma once

#include "ipa.h"
#include "lwip/sockets.h"
#include "fskdef.h"

namespace ufo
{
    
    namespace net
    {
        // todo ipv6
        // use for fast socket-creation 
        // it collect ip, port, address-family, prot-ip 
        // 
        class ipnet : public ipa
        {
        protected:
            uint8_t _protoIP = 0;

        public:
            ipnet() : _protoIP(IPPROTO_IP) 
            {
                _addr.sin_port = UFO_SOCKET_DEFAULT_PORT;
                _addr.sin_family = AF_INET;
            }

            ipnet(uint32_t raw,  uint16_t port = UFO_SOCKET_DEFAULT_PORT) 
                : ipa(raw) , _protoIP(IPPROTO_IP)
            {
                _addr.sin_port = htons(port);;
                _addr.sin_family = AF_INET;
            }


            // todo (if addr != ipv4 => ipv6)
            ipnet (const char* addr, uint16_t port = UFO_SOCKET_DEFAULT_PORT) 
                : ipa(addr), _protoIP(IPPROTO_IP) 
            {
                _addr.sin_port = htons(port);
                _addr.sin_family = AF_INET;
            }

            ipnet(uint8_t o1,uint8_t o2,uint8_t o3,uint8_t o4, uint16_t port = UFO_SOCKET_DEFAULT_PORT) 
                : ipa(o1,o2,o3,o4), _protoIP(IPPROTO_IP)
            {
                _addr.sin_port = htons(port);;
                _addr.sin_family = AF_INET;
            }

            // uint16_t port() const {
            //     return _addr.sin_port;
            // }

            // uint8_t fam() const {
            //     return _addr.sin_family;
            // }
            // uint8_t proto() const {
            //     return _protoIP;
            // }

            void log (){
                ipa::log();
                Trace_t::log("\nport:\n\t");
                Trace_t::log(_addr.sin_port);
                Trace_t::log("\n");
            }

            ~ipnet() {}
        };

 


    } // namespace net
    


} // namespace ufo


