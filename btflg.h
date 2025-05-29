#pragma once

#include <type_traits>
#include <stdint.h>
#include "mutex.h"
namespace ufo
{
    template<typename Ty, typename lTy = void, typename lTy2 = void>
    class bit_flag_t;

    template <typename Ty>
    class bit_flag_t<Ty>
    {
    private:
        Ty _flags = 0;

    public:
        bit_flag_t() {
            static_assert(std::is_integral_v<Ty>);
        }
        bit_flag_t(Ty flags) : _flags(flags) {}
        ~bit_flag_t() {}

        bit_flag_t(const bit_flag_t& oth) : _flags(oth._flags) {}
        bit_flag_t& operator= (const bit_flag_t& oth) {
            if (&oth != this)
            {
                _flags = oth._flags;
            }
            return *this;            
        }
        
        bool operator== (const bit_flag_t<Ty>& o) const {
            return _flags == o._flags;
        }

        bit_flag_t(bit_flag_t&& oth) : _flags(oth._flags) {
            oth._flags = 0;
        }
        bit_flag_t& operator= (bit_flag_t&& oth) {
            if (&oth != this)
            {
                _flags = oth._flags;
                oth._flags = 0;
            }
            return *this;    
        }

        template<typename Pos, typename... Poses>
        void set(Pos pos, Poses... lst)
        {
            set(pos);
            set(lst...);
        }
        template <typename enum_Ty, std::enable_if_t<std::is_enum_v<enum_Ty>, bool> = true>
        void set(enum_Ty pos)
        {
            set(static_cast<uint8_t>(pos));
        }
        template <typename int_t, std::enable_if_t<std::is_integral_v<int_t>, bool> = true>
        void set(int_t pos)
        {
            if (pos < sizeof(Ty) * 8)
            {
                _flags |= (1 << pos);
            }
        }

        template <typename Pos, typename... Poses>
        void unset(Pos pos, Poses... lst)
        {
            unset(pos);
            unset(lst...);
        }
        template <typename enum_Ty, std::enable_if_t<std::is_enum_v<enum_Ty>, bool> = true>
        void unset(enum_Ty pos)
        {
            unset(static_cast<uint8_t>(pos));
        }
        template <typename int_t, std::enable_if_t<std::is_integral_v<int_t>, bool> = true>
        void unset(int_t pos)
        {
            if (get(pos))
            {
                togle(pos);
            }
        }

        template <typename Pos, typename... Poses>
        void togle(Pos pos, Poses... lst)
        {
            togle(pos);
            togle(lst...);
        }

        template <typename enum_Ty, std::enable_if_t<std::is_enum_v<enum_Ty>, bool> = true>
        void togle(enum_Ty pos)
        {
            togle(static_cast<uint8_t>(pos));
        }

        template <typename int_t, std::enable_if_t<std::is_integral_v<int_t>, bool> = true>
        void togle(int_t pos)
        {
            if (pos < sizeof(Ty) * 8)
            {
                _flags ^= (1 << pos);
            }
        }

        template <typename Pos, typename... Poses>
        bool get(Pos pos, Poses... poses) const
        {
            return get(pos) && get(poses...);
        }
        template <typename enum_Ty, std::enable_if_t<std::is_enum_v<enum_Ty>, bool> = true>
        bool get(enum_Ty pos) const
        {
            return get(static_cast<uint8_t>(pos));
        }
        template <typename int_t, std::enable_if_t<std::is_integral_v<int_t>, bool> = true>
        bool get(int_t pos) const
        {
            if (pos < sizeof(Ty) * 8)
            {
                return static_cast<bool>((_flags >> pos) & 0b1);
            }
            return false;
        }

        void rst() { _flags = 0; }
        Ty get() const { return _flags; }

        void upd(Ty val) { _flags = val; }
    };

    template <typename Ty, typename lock_t>
    class bit_flag_t<Ty, lock_t>
    {
    private:
        lock_t _lock;
        bit_flag_t<Ty> _fl;
    public:
        bit_flag_t() {
            static_assert(std::is_integral_v<Ty>);
        }
        bit_flag_t(Ty flags) : _fl(flags) {}
        ~bit_flag_t() {}

        bit_flag_t(const bit_flag_t<Ty, lock_t>&) = delete;
        bit_flag_t &operator=(const bit_flag_t<Ty, lock_t> &) = delete;

        bit_flag_t(bit_flag_t<Ty, lock_t> &&oth) : _fl(oth._fl) {
            oth._fl = 0;
        }

        bit_flag_t &operator=(bit_flag_t<Ty, lock_t> &&oth)
        {
            lock_guard<lock_t> _l(_lock);
            if (&oth != this)
            {
                _fl = std::move(oth._fl);
            }
            return *this;
        }

        bit_flag_t(const bit_flag_t<Ty>& oth) {
            _fl = oth.get();
        }

        bit_flag_t(bit_flag_t<Ty> &&oth) : _fl(std::move(oth)) {}

        bit_flag_t &operator=(const bit_flag_t<Ty> &oth)
        {
            lock_guard<lock_t>_l(_lock);
            // _fl = oth._flags;
            _fl = oth.get();
            return *this;
        }

        bit_flag_t &operator=(bit_flag_t<Ty> &&oth)
        {
            lock_guard<lock_t>_l(_lock);
            _fl = oth._flags;
            oth._flags = 0;
            return *this;
        }

        template<typename... Poses>
        void set(Poses... ps){
            lock_guard<lock_t>_l(_lock);
            _fl.set(ps...);
        }

        template<typename... Poses>
        void unset(Poses... ps){
            lock_guard<lock_t>_l(_lock);
            _fl.unset(ps...);
        }

        template <typename... Poses>
        void togle(Poses... ps)
        {
            lock_guard<lock_t> _l(_lock);
            _fl.togle(ps...);
        }

        template <typename... Poses>
        bool get(Poses... ps)
        {
            lock_guard<lock_t> _l(_lock);
            bool v = _fl.get(ps...);
            return v;
        }

        void rst() { 
            lock_guard<lock_t> _l(_lock);
            _fl.rst(); 
        }
        Ty get() const {
            lock_guard<lock_t> _l(_lock);
            Ty v = _fl.get();
            return v;
        }

        void upd(Ty val) {
            lock_guard<lock_t> _l(_lock);
            _fl.upd(val);
        }
    };

} // namespace ufo
