#pragma once
#include <stdint.h>

namespace ufo
{

    class ip_t
    {
    private:
        uint32_t _ip = 0;

    public:
        ip_t() {}
        ip_t(uint32_t raw) : _ip(raw) {}

        ip_t(uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4)
        {
            set(a1, a2, a3, a4);
        }
        ip_t(const char *addr)
        {
            set(addr);
        }

        ~ip_t() {}

        void operator=(uint32_t raw) {
            _ip = raw;
        }


        uint8_t operator[](uint8_t ind) const
        {

            if (ind > 3)
            {
                return 0;
            }

            uint32_t ip = _ip >> (8 * (3 - ind));
            return ip & 255;
        }

        void set(uint32_t raw)
        {
            _ip = raw;
        }

        void set(uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4)
        {
            // first a1 in initialiation
            uint32_t addr = a1;
            addr <<= 8;
            addr |= a2;
            addr <<= 8;
            addr |= a3;
            addr <<= 8;
            addr |= a4;
            _ip = lwip_htonl(addr);
        }

        void set(const char *addr)
        {
            if (!__stoa4(addr))
            {
                printf("warning !ip4\n");
                _ip = 0;
            }
        }

        void rst() { _ip = 0; }

        uint32_t get() const
        {
            return _ip;
        }

        uint32_t operator()() const
        {
            return _ip;
        }

        operator bool()
        {
            return _ip > 0;
        }
        void log()
        {
            uint32_t a = _ip;
            Trace_t::flog(/* "%u :: " */ "%u.%u.%u.%u",
                //   _ip,
                  static_cast<uint32_t>(a & 255),
                  static_cast<uint32_t>((a >> 8) & 255),
                  static_cast<uint32_t>((a >> 16) & 255),
                  static_cast<uint32_t>((a >> 24) & 255));
        }

    private:
        // string to address (ipv4)
        bool __stoa4(const char *addr)
        {
            int dot = 0;
            int oct = -1; // oct-max = 255

            uint32_t tmp = 0;
            // std::cout << addr << '\n';
            while (*addr)
            {
                char c = *addr++;

                if (c >= '0' && c <= '9')
                {
                    oct = (oct < 0) ? (c - '0') : oct * 10 + (c - '0');
                    if (oct > 255)
                    {
                        // Value out of [0..255] range
                        return false;
                    }
                }
                else if (c == '.')
                {
                    if (dot == 3)
                    {
                        // Too many dots (there must be 3 dots)
                        return false;
                    }
                    if (oct < 0)
                    {
                        /* No value between dots, e.g. '1..' */
                        return false;
                    }
                    tmp <<= 8;
                    tmp |= oct;
                    ++dot;
                    oct = -1;
                }
                else
                {
                    // Invalid char
                    return false;
                }
            }
            if (dot != 3)
            {
                // Too few dots (there must be 3 dots)
                return false;
            }
            if (oct < 0)
            {
                /* No value between dots, e.g. '1..' */
                return false;
            }
            tmp <<= 8;
            tmp |= oct;
            // _addr = tmp;
            _ip =  lwip_htonl(tmp);

            return true;
        }

        bool __stoa6(const char *addr) { return false; }

    };

} // namespace ufo
