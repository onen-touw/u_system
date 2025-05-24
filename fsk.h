#pragma once

#include "fskdef.h"
#include "ipa.h"
#include "ipnet.h"

namespace ufo
{
    namespace net
    {

        class fsk : public fsk_base, public ipnet
        {
        public:
            using sockt_t = net::uSocketType_t;
        private:
            sockt_t _type = sockt_t::null;
            int32_t _isock = -1; // socket descriptor
            ipa _source;
        public:
            // fsk() {}

            fsk(const char *addr, sockt_t type, callback_t cb)
                : fsk_base(cb), ipnet(addr), _type(type)
            {
                ufo::Error_t &_error = ufo::Error_t::GetInstance();

                _isock = lwip_socket(_addr.sin_family, SOCK_DGRAM, _protoIP);
                if (_isock < 0)
                {
                    _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::net_bad_sock, "cant create socket")));
                    return;
                }

                {
                    // Set timeout
                    struct timeval timeout;
                    timeout.tv_sec = 0;                                         // blocking lwip_recvfrom time
                    timeout.tv_usec = UFO_SOCKET_DEFAULT_RCV_TIMEOUT_MS * 1000; // 5 ms
                    lwip_setsockopt(_isock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
                }

                if (_type == sockt_t::server)
                {
                    sockaddr_in ai;
                    ai.sin_addr.s_addr = lwip_htonl(INADDR_ANY);
                    ai.sin_family = _addr.sin_family;
                    ai.sin_port = _addr.sin_port;

                    int e = bind(_isock, (struct sockaddr *)&ai, sizeof(ai));
                    if (e < 0)
                    {
                        lwip_shutdown(_isock, 0);
                        lwip_close(_isock);
                        _isock = -1;
                        _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::net_bad_sock, "cant create socket")));
                        return;
                    }
                }
            }
            
            virtual ~fsk() override {
                if (_isock > 0)
                {
                    lwip_shutdown(_isock, 0);
                    lwip_close(_isock);
                }
            }

            void set_source(const char *ip)
            {
                _source = ipa(ip);
            }

            virtual void rcv() override
            {
                socklen_t l = sizeof(sockaddr_in);
                int len = lwip_recvfrom(_isock, _rcv->_payload, UFO_SOCKET_BUFFER_SIZE - 1, 0, (struct sockaddr *)&_source.get_native(), &l);
                if (len > 0)
                {
                    // Serial.println("rsv->funk");
                    _callback(_rcv.get());
                    if (_rcv->_errorCount > 0)
                    {
                        --_rcv->_errorCount;
                    }
                    _rcv->_len = len;
                }
                else
                {
                    uint32_t ms = utl::get_time_millis();
                    if (ms - _rcv->_lastCallTick > 500)
                    {
                        ++_rcv->_errorCount;
                    }
                    _rcv->_lastCallTick = ms;
                }
                // if (_sourceAddr.ss_family == PF_INET)
                // {
                //  ip -> str
                //     inet_ntoa_r(((struct sockaddr_in *)&_sourceAddr)->sin_addr, _addrStr, sizeof(_addrStr) - 1);
                // }
            }

            virtual void snd() override
            {
                lock_guard<mutex_t> _l(_snd->_lock);
                if (_snd->_data._ready)
                {
                    int e = 0;
                    if (_type == sockt_t::server)
                    {
                        e = lwip_sendto(_isock, _snd->_data._payload, _snd->_data._len, 0, (struct sockaddr *)&_source.get_native(), sizeof(sockaddr_in));
                    }
                    else
                    {
                        e = lwip_sendto(_isock, _snd->_data._payload, _snd->_data._len, 0, (struct sockaddr *)&_addr, sizeof(_addr));
                    }
                    if (e < 0)
                    {
                        // add counter
                        // ufo::Error_t &_error = ufo::Error_t::GetInstance();
                        // _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::net_snd_err, "send error")));
                        
                    }

                    if (_snd->_data._errorCount > 0)
                    {
                        --_snd->_data._errorCount;
                    }
                    _snd->_data._ready = false;
                    _snd->_data._len = 0;
                    _snd->_data._lastCallTick = utl::get_time_millis(); // millis for a while
                }
                else
                {
                    if (utl::get_time_millis() - _snd->_data._lastCallTick > 200) // millis, 200 - for a while
                    {
                        ++_snd->_data._errorCount;
                    }
                }
            }

            virtual void ch_snd() override
            {
                lock_guard<mutex_t> _l(_snd->_lock);
                if (_snd->_data._errorCount > 10) // 10 for a while
                {
                    // _alarm.doSmth
                    // func*   onSendError();
                    // Serial.println("Alarm1!");
                }
            }
            virtual void ch_rcv() override
            {
                if (_rcv->_errorCount > 4) // 4 for a while
                {
                    // Serial.println("Alarm2!");
                    // func*   onRecvError();
                }
            }
        };

        class fast_sock : public ipnet
        {
        public:
            using rcv_t = net::uSocketDataPacket_t;
            using snd_t = ufo::net::uSocketControlBlock_t;
            using sockt_t = net::uSocketType_t;
            using callback_t = void (*)(rcv_t *);

            // dest addr is ipnet::ipa::_addr
            // source addr is

        private:
            sockt_t _type = sockt_t::null;
            int32_t _isock = -1; // socket descriptor

            std::shared_ptr<snd_t> _snd;
            std::unique_ptr<rcv_t> _rcv;
            callback_t _callback = nullptr;

            ipa _source;

        public:
            fast_sock()
            {
            }

            fast_sock(const char *addr, sockt_t type, callback_t cb)
                : ipnet(addr), _type(type), _callback(cb)
            {
                ufo::Error_t &_error = ufo::Error_t::GetInstance();

                _isock = lwip_socket(_addr.sin_family, SOCK_DGRAM, _protoIP);
                if (_isock < 0)
                {
                    _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::net_bad_sock, "cant create socket")));
                    return;
                }

                {
                    // Set timeout
                    struct timeval timeout;
                    timeout.tv_sec = 0;                                         // blocking lwip_recvfrom time
                    timeout.tv_usec = UFO_SOCKET_DEFAULT_RCV_TIMEOUT_MS * 1000; // 5 ms
                    lwip_setsockopt(_isock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
                }

                if (_type == sockt_t::server)
                {
                    sockaddr_in ai;
                    ai.sin_addr.s_addr = lwip_htonl(INADDR_ANY);
                    ai.sin_family = _addr.sin_family;
                    ai.sin_port = _addr.sin_port;

                    int e = bind(_isock, (struct sockaddr *)&ai, sizeof(ai));
                    if (e < 0)
                    {
                        lwip_shutdown(_isock, 0);
                        lwip_close(_isock);
                        _isock = -1;
                        _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::net_bad_sock, "cant create socket")));
                        return;
                    }
                }

                // _snd = std::make_unique<snd_t>();
                _snd = std::make_shared<snd_t>();
                _rcv = std::make_unique<rcv_t>();
            }
            fast_sock(fast_sock &) = delete;
            fast_sock &operator=(fast_sock &) = delete;

            const std::shared_ptr<snd_t> get_block() const
            {
                return _snd;
            }

            void set_source(const char* ip){
                _source = ipa(ip);
            }

            void rcv()
            {
                socklen_t l = sizeof(sockaddr_in);
                int len = lwip_recvfrom(_isock, _rcv->_payload, UFO_SOCKET_BUFFER_SIZE - 1, 0, (struct sockaddr *)&_source.get_native(), &l);
                if (len > 0)
                {
                    // Serial.println("rsv->funk");
                    _callback(_rcv.get());
                    if (_rcv->_errorCount > 0)
                    {
                        --_rcv->_errorCount;
                    }
                    _rcv->_len = len;
                }
                else
                {
                    uint32_t ms = utl::get_time_millis();
                    if (ms - _rcv->_lastCallTick > 500)
                    {
                        ++_rcv->_errorCount;
                    }
                    _rcv->_lastCallTick = ms;
                }
                // if (_sourceAddr.ss_family == PF_INET)
                // {
                //  ip -> str
                //     inet_ntoa_r(((struct sockaddr_in *)&_sourceAddr)->sin_addr, _addrStr, sizeof(_addrStr) - 1);
                // }
            }

            void snd()
            {
                lock_guard<mutex_t> _l(_snd->_lock);
                if (_snd->_data._ready)
                {
                    int e = 0;
                    if (_type == sockt_t::server)
                    {
                        e = lwip_sendto(_isock, _snd->_data._payload, _snd->_data._len, 0, (struct sockaddr *)&_source.get_native(), sizeof(sockaddr_in));
                    }
                    else
                    {
                        e = lwip_sendto(_isock, _snd->_data._payload, _snd->_data._len, 0, (struct sockaddr *)&_addr, sizeof(_addr));
                    }
                    if (e < 0)
                    {
                        // add counter
                        // ufo::Error_t &_error = ufo::Error_t::GetInstance();
                        // _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::net_snd_err, "send error")));
                        
                    }

                    if (_snd->_data._errorCount > 0)
                    {
                        --_snd->_data._errorCount;
                    }
                    _snd->_data._ready = false;
                    _snd->_data._len = 0;
                    _snd->_data._lastCallTick = utl::get_time_millis(); // millis for a while
                }
                else
                {
                    if (utl::get_time_millis() - _snd->_data._lastCallTick > 200) // millis, 200 - for a while
                    {
                        ++_snd->_data._errorCount;
                    }
                }
            }

            // todo
            void chk_snd()
            {
                lock_guard<mutex_t> _l(_snd->_lock);
                if (_snd->_data._errorCount > 10) // 10 for a while
                {
                    // _alarm.doSmth
                    // func*   onSendError();
                    // Serial.println("Alarm1!");
                }
            }

            // todo
            void chk_rcv()
            {

                if (_rcv->_errorCount > 4) // 4 for a while
                {
                    // Serial.println("Alarm2!");
                    // func*   onRecvError();
                }
            }

            ~fast_sock()
            {
                if (_isock > 0)
                {
                    lwip_shutdown(_isock, 0);
                    lwip_close(_isock);
                }
            }
        };

    } // namespace net

} // namespace ufo
