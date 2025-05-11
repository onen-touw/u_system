#pragma once

#include <string.h>
#include <initializer_list>

namespace ufo
{

    template <typename Ty, typename sz_t>
    class allocator_t
    {
    public:
        // placement new
        Ty *allocate(sz_t sz)
        {
            Ty *buf = static_cast<Ty *>(operator new(sz * sizeof(Ty)));
            if (!buf)
            {
                throw;
            }
            return buf;
        }

        void dealocate(Ty *storage, sz_t sz)
        {
            operator delete(storage, sz * sizeof(Ty));
        }

        void destroy(Ty *obj)
        {
            obj->~Ty();
        }

        template <typename... Args>
        void construct(Ty *storage, sz_t offset, Args &&...args)
        {
            ::new (storage + offset) Ty(std::forward<Args>(args)...);
        }

        allocator_t(/* args */) {}
        ~allocator_t() {}
    };

    template <typename Ty, typename sz_t>
    class bsvct_t
    {
    public:
        using alloc_t = allocator_t<Ty, sz_t>;

    private:
        Ty *_arr = nullptr;
        sz_t _sz = 0;
        sz_t _cap = 0;
        alloc_t _alloc; // should be in the end of the parametrs list

    public:
        bsvct_t() {}

        bsvct_t(sz_t size) : _arr(nullptr), _sz(0), _cap(size)
        {
            _arr = _alloc.allocate(_cap);
            for (sz_t i = 0; i < size; ++i)
            {
                _alloc.construct(_arr, _sz, Ty());
                ++_sz;
            }
        }

        bsvct_t(std::initializer_list<Ty> list) : _arr(nullptr), _sz(0), _cap(list.size())
        {
            _arr = _alloc.allocate(_cap);
            for (const Ty *i = list.begin(); i != list.end(); ++i)
            {
                _alloc.construct(_arr, _sz, std::move(*i));
                ++_sz;
            }
        }

        bsvct_t(sz_t sz, Ty obj) : _arr(nullptr), _sz(sz), _cap(sz)
        {
            _arr = _alloc.allocate(_cap);
            for (size_t i = 0; i < _sz; ++i)
            {
                // Ty cobj = obj;
                // _alloc.construct(_arr, i, std::move(cobj));
                _alloc.construct(_arr, i, obj);
            }
        }

        bsvct_t(const bsvct_t &rhs) : _arr(nullptr), _sz(rhs._sz), _cap(rhs._cap)
        {
            _arr = _alloc.allocate(_cap);
            for (sz_t i = 0; i < _sz; ++i)
            {
                _alloc.construct(_arr, i, rhs._arr[i]);
            }
        }

        bsvct_t(bsvct_t &&rhs) : _arr(rhs._arr), _sz(rhs._sz), _cap(rhs._cap)
        {
            rhs._arr = nullptr;
            rhs._sz = 0;
            rhs._cap = 0;
        }

        bsvct_t &operator=(const bsvct_t &rhs)
        {
            if (&rhs != this)
            {
                _clear_mem();

                _sz = rhs._sz;
                _cap = rhs._cap;
                _arr = _alloc.allocate(_cap);

                for (sz_t i = 0; i < _sz; ++i)
                {
                    _alloc.construct(_arr, i, rhs._arr[i]);
                }
            }
            return *this;
        }

        bsvct_t &operator=(bsvct_t &&rhs)
        {
            if (&rhs != this)
            {
                _clear_mem();

                _sz = rhs._sz;
                _cap = rhs._cap;
                _arr = rhs._arr;

                rhs._sz = 0;
                rhs._cap = 0;
                rhs._arr = nullptr;
            }
            return *this;
        }

        ~bsvct_t()
        {
            _clear_mem();
        }

        sz_t size() const noexcept { return _sz; }
        sz_t capacity() const noexcept { return _cap; }
        bool empty() const noexcept { return !_sz; }

        Ty &operator[](sz_t index) { return _arr[index]; }
        const Ty &operator[](sz_t index) const { return _arr[index]; }

        Ty &at(sz_t index)
        {
            if (index > _sz)
            {
                throw;
            }
            return _arr[index];
        }

        void reserve(sz_t newcap)
        {
            if (newcap < _sz)
            {
                throw;
            }
            Ty *t_cap = _alloc.allocate(newcap);

            for (sz_t i = 0; i < _sz; ++i)
            {
                _alloc.construct(t_cap, i, std::move(_arr[i]));
                _alloc.destroy(&_arr[i]);
            }
            _del_mem();
            _arr = t_cap;
            _cap = newcap;
        }

        void push_back(const Ty& obj)
        {
            if (_cap == _sz)
            {
                if (!_cap)
                {
                    _cap = 1;
                }
                reserve(_cap * 2);
            }

            _alloc.construct(_arr, _sz, obj);
            ++_sz;
        }

        void push_back(Ty &&obj)
        {
            emplayce_back(std::move(obj));
        }

        void pop_back()
        {
            _alloc.destroy(&_arr[--_sz]);
        }

        template <typename... Args>
        void emplayce_back(Args &&...args)
        {

            if (_cap == _sz)
            {
                if (!_cap)
                {
                    _cap = 1;
                }
                reserve(_cap * 2);
            }

            _alloc.construct(_arr, _sz, std::forward<Args>(args)...);
            ++_sz;
        }

        void clear()
        {
            _destroy_all();
        }

        void reset()
        {
            _clear_mem();
        }

        Ty *raw()
        {
            return _arr;
        }

    private:
        void _destroy_all()
        {
            if (_sz)
            {
                for (sz_t i = 0; i < _sz; ++i)
                {
                    _alloc.destroy(&_arr[i]);
                }
            }
            _sz = 0;
        }

        void _del_mem()
        {
            if (_cap)
            {
                _alloc.dealocate(_arr, _cap);
            }
            _cap = 0; 
        }

        void _clear_mem()
        {
            _destroy_all();
            _del_mem();
        }
    };

    template <typename Ty>
    using vector_t = bsvct_t<Ty, uint16_t>;

} // namespace ufo
