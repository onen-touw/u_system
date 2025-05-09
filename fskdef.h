#pragma once

#include "config.h"
#include "bits/unique_ptr.h"
#include "mutex.h"
#include "error.h"
#include <stdarg.h>

// todo constexpr
#define UFO_SOCKET_DEFAULT_PORT 6464
#define UFO_SOCKET_DEFAULT_RCV_TIMEOUT_MS 5
#define UFO_SOCKET_BUFFER_SIZE 128

namespace ufo
{
    namespace net
    {
        enum class uSocketReturn_t : uint8_t {
            ok,
            owerflow,
            size0,
        };

        enum class uSocketType_t : uint8_t
        {
            UFO_SOCK_NO,
            UFO_SOCK_SERVER,
            UFO_SOCK_CLIENT,
        };

        struct uSocketDataPacket_t
        {
            char _payload[UFO_SOCKET_BUFFER_SIZE];
            uint8_t _len = 0;
            uint8_t _errorCount = 0; // timeout count
            int32_t _lastCallTick = 0;
            bool _ready = false;
        };

        class uSocketControlBlock_t
        {
            friend class fast_sock;

        private:
            uSocketDataPacket_t _data;
            mutex_t _lock;

        public:
            uSocketDataPacket_t &GetDataBlock()
            {
                return _data;
            }

            mutex_t& get_lock(){
                return _lock;
            }

            uSocketReturn_t Msg(uint8_t offset, const char *payload, int16_t size)
            {
                if (!size)
                {
                    return uSocketReturn_t::size0;
                }

                if (size > UFO_SOCKET_BUFFER_SIZE - offset - 1)
                {
                    return uSocketReturn_t::owerflow;
                }
                
                lock_guard<mutex_t> _l(_lock);

                memcpy((_data._payload + offset), payload, size);
                _data._len = size + offset;
                _data._ready = true;
                return uSocketReturn_t::ok;
            }

            uSocketReturn_t fMsg(uint8_t offset, const char* msg, ...) {
                uSocketReturn_t ret = uSocketReturn_t::ok;
                if (!strlen(msg))
                {
                    ret = uSocketReturn_t::size0;
                    return ret;
                }
                
                va_list ap;
                va_start(ap, msg);

                // calculate required string size
                va_list arg;
                va_copy(arg, ap);
                uint32_t req = 1+vsnprintf(NULL, 0, msg, arg);
                va_end(arg);

                lock_guard<mutex_t> _l(_lock);

                if (req + offset > UFO_SOCKET_BUFFER_SIZE - 1)
                {
                    req = UFO_SOCKET_BUFFER_SIZE - 1;
                    ret = uSocketReturn_t::owerflow;
                }

                int n = vsnprintf(_data._payload + offset, req, msg, ap);
                va_end(ap);
                _data._len = n;
                _data._ready = true;
                return ret;
            }

            

            uSocketReturn_t fMsg(const char* msg, ...) {
                uSocketReturn_t ret = uSocketReturn_t::ok;
                if (!strlen(msg))
                {
                    ret = uSocketReturn_t::size0;
                    return ret;
                }
                
                va_list ap;
                va_start(ap, msg);

                // calculate required string size
                va_list arg;
                va_copy(arg, ap);
                uint32_t req = 1+vsnprintf(NULL, 0, msg, arg);
                va_end(arg);

                lock_guard<mutex_t> _l(_lock);

                if (req > UFO_SOCKET_BUFFER_SIZE - 1)
                {
                    req = UFO_SOCKET_BUFFER_SIZE - 1;
                    ret = uSocketReturn_t::owerflow;
                }

                int n = vsnprintf(_data._payload, req, msg, ap);
                va_end(ap);
                _data._len = n;
                _data._ready = true;
                return ret;
            }

            uSocketReturn_t Msg(const char *payload, int16_t size)
            {
                if (!size)
                {
                    return uSocketReturn_t::size0;
                }
                
                if (size > UFO_SOCKET_BUFFER_SIZE - 1)
                {
                    return uSocketReturn_t::owerflow; 
                }

                lock_guard<mutex_t> _l(_lock);

                memcpy(_data._payload, payload, size);
                _data._len = size;
                _data._ready = true;

                return uSocketReturn_t::ok;
            }

            uSocketReturn_t Msg(const char *payload)
            {
                int16_t l = strlen(payload);

                if (!l)
                {
                    return uSocketReturn_t::size0;
                }

                if (l > UFO_SOCKET_BUFFER_SIZE - 1)
                {
                    return uSocketReturn_t::owerflow; // false if overflow or empty string
                }

                lock_guard<mutex_t> _l(_lock);

                memcpy(_data._payload, payload, l);
                _data._len = l;
                _data._ready = true;
                return uSocketReturn_t::ok;
            }

            
            uSocketReturn_t Msg(uint8_t* payload, uint8_t sz)
            {
                if (!sz)
                {
                    return uSocketReturn_t::size0;
                }

                if (sz > UFO_SOCKET_BUFFER_SIZE - 1)
                {
                    return uSocketReturn_t::owerflow; // false if overflow or empty string
                }

                lock_guard<mutex_t> _l(_lock);

                memcpy(_data._payload, payload, sz);
                _data._len = sz;
                _data._ready = true;
                return uSocketReturn_t::ok;
            }
            
        };

    } // namespace net
} // namespace ufo
