#pragma once

#include <string.h>
#include <stdint.h>


namespace ufo
{
    namespace str_base
    {

        template <typename char_t, typename size_t>
        class bsstr_t
        {
        public:
            static constexpr size_t no_pos = -1;

            using size_type = size_t;
        public:
        
            class bsstr_iterator_t
            {
            private:
                char_t *_ptr = nullptr;
                const bsstr_t *_parent = nullptr;

            public:
                bsstr_iterator_t() {}

                bsstr_iterator_t(char_t *raw, const bsstr_t *parent) : _ptr(raw), _parent(parent) {}

                bsstr_iterator_t(const bsstr_iterator_t &other) : _ptr(other._ptr),
                                                                  _parent(other._parent)
                {
                }

                bsstr_iterator_t(bsstr_iterator_t &&other) : _ptr(other._ptr),
                                                             _parent(other._parent)
                {
                    other._parent = nullptr;
                    other._ptr = nullptr;
                }

                bsstr_iterator_t &operator=(const bsstr_iterator_t &other)
                {
                                
                    if (&other != this)
                    {
                        _ptr = other._ptr;
                        _parent = other._parent;
                    }
                    return *this;
                }

                bsstr_iterator_t &operator=(bsstr_iterator_t &&other)
                {
                    if (&other != this)
                    {
                        _ptr = other._ptr;
                        _parent = other._parent;
                        other._parent = nullptr;
                        other._ptr = nullptr;
                    }
                    return *this;
                }

                ~bsstr_iterator_t() {}

                bool operator==(const bsstr_iterator_t &other)
                {
                    return (other._ptr == _ptr) && (other._parent == _parent);
                }

                bool operator!=(const bsstr_iterator_t &other)
                {
                    return !(other._ptr == _ptr) && (other._parent == _parent);
                }

                bool operator==(const char_t &obj)
                {
                    return *_ptr == obj;
                }

                bool operator > (const bsstr_iterator_t &other)
                {
                    return _ptr > other._ptr;
                }

                void out_of_range_check(const char_t *ptr)
                {
                    size_t dist = ptr - _parent->_arr;
                    if (dist > _parent->size())
                    {
                        throw;
                    }
                }

                char_t &operator++()
                {
                    if (!_ptr || !_parent)
                    {
                        throw;
                    }
                    ++_ptr;

                    out_of_range_check(_ptr);

                    return *_ptr;
                }

                char_t &operator--()
                {
                    if (!_ptr || !_parent)
                    {
                        throw;
                    }
                    --_ptr;

                    out_of_range_check(_ptr);

                    return *_ptr;
                }

                operator bool() const
                {
                    return (_ptr && _parent && _ptr != (_parent->_arr + _parent->_size));
                    // return _ptr && _parent;
                }

                char_t *operator->()
                {
                    return _ptr;
                }

                const char &get()
                {
                    return *_ptr;
                }
            };

            using iterator_t = bsstr_iterator_t;

        private:
            char_t *_arr = nullptr;
            size_t _size = 0;

        public:
            bsstr_t() {}

            bsstr_t(size_t sz) : _arr(nullptr), _size(sz)
            {
                if (!_size)
                {
                    return;
                }
                _arr = new char_t[_size + 1](); // +1 for \0
            }

            bsstr_t(const char *str) : _arr(nullptr), _size(strlen(str))
            {
                if (!_size)
                {
                    return;
                }

                _arr = new char_t[_size + 1](); // +1 for \0
                memcpy(_arr, str, _size);
            }

            bsstr_t(const bsstr_t &other) : _arr(nullptr), _size(other._size)
            {
                if (_size)
                {
                    _arr = new char_t[_size + 1]();
                    memcpy(_arr, other._arr, _size);
                }
            }

            bsstr_t(bsstr_t &&other) : _arr(other._arr), _size(other._size)
            {
                other._arr = nullptr;
                other._size = 0;
            }

            ~bsstr_t()
            {
                clear();
            }

            void clear(){
                _size = 0;
                if (_arr)
                {
                    delete[] _arr;
                    _arr = nullptr;
                }
            }

            bsstr_t &operator=(const char_t* str)
            {
                if (_arr)
                {
                    delete[] _arr;
                }
                _size = strlen(str);
                if (_size)
                {
                    _arr = new char_t[_size + 1]();
                    memcpy(_arr, str, _size);
                }
                return *this;
            }

            bsstr_t &operator=(const bsstr_t &other)
            {
                if (&other != this)
                {
                    if (_arr)
                    {
                        delete[] _arr;
                    }
                    _size = other._size;
                    if (_size)
                    {
                        _arr = new char_t[_size + 1]();
                        memcpy(_arr, other._arr, _size);
                    }
                }
                return *this;
            }

            bsstr_t &operator=(bsstr_t &&other)
            {
                if (&other != this)
                {
                    if (_arr)
                    {
                        delete[] _arr;
                    }
                    _arr = other._arr;
                    _size = other._size;
                    other._arr = nullptr;
                    other._size = 0;
                }
                return *this;
            }

            bool operator==(const bsstr_t &other)
            {
                if (_size != other._size)
                {
                    return false;
                }

                if (strcmp(_arr, other._arr) != 0)
                {
                    return false;
                }
                return true;
            }

            char& operator[](size_t ind) { return _arr[ind]; }

            bsstr_t substr(size_t pos, size_t size) const
            {
                if (!_size)
                {
                    return bsstr_t();
                }

                if (pos > _size - 1)
                {
                    return bsstr_t();
                }

                if (pos + size > _size)
                {
                    return bsstr_t();
                }

                bsstr_t res;
                res._arr = new char_t[size + 1]();
                res._size = size;
                memcpy(res._arr, _arr + pos, size);
                return res;
            }

            bool equal(const char_t *str) const
            {
                size_t len = strlen(str);
                if (len != _size)
                {
                    return false;
                }
                
                if (strcmp(_arr, str) != 0)
                {
                    return false;
                }
                return true;
            }

            bool equal(const char_t *str, size_t n) const
            {
                if (n > _size)
                {
                    return false;
                }
                
                if (strncmp(_arr, str, n) != 0)
                {
                    return false;
                }
                return true;
            }

            bool equal(const bsstr_t &str) const
            {
                return equal(str.c_str(), str.size());
            }

            bool equal(const bsstr_t &str, size_t n) const
            {
                if (n > str.size())
                {
                    return false;
                }
                
                return equal(str.c_str(), n);
            }

            size_t find(const char_t c) const
            {
                if (!_size)
                {
                    return no_pos;
                }
                
                for (size_t i = 0; i < _size; ++i)
                {
                    if (_arr[i] == c)
                    {
                        return i;
                    }
                }
                return no_pos;
            }

            const char_t *c_str() const 
            {
                return _arr;
            }

            char_t *raw() 
            {
                return _arr;
            }

            size_t size() const { return _size; }

            bool empty() const { return !_size; }

            iterator_t begin() const
            {
                iterator_t i(_arr, this);
                return i;
            }

            iterator_t end() const
            {
                iterator_t i(_arr + _size, this); // this is \0 element in the string
                return i;
            }

            iterator_t front() const
            {
                iterator_t i(_arr, this);
                return i;
            }

            iterator_t back() const
            {
                iterator_t i(_arr + _size - 1, this); // this is the last element in the string
                return i;
            }

            // good way to add enable_if<is_conteiner_t::value>
            template <template <typename> typename Conteiner>
            Conteiner<bsstr_t<char_t, size_t>> split(char_t separator)
            {
                Conteiner<bsstr_t<char_t, size_t>> conteiner;

                size_t pos = 0;
                size_t iter = 0;

                while (iter < _size)
                {
                    if (_arr[iter] == separator)
                    {
                        conteiner.emplayce_back(substr(pos, iter - pos));
                        pos = ++iter;
                    }
                    ++iter;
                }
                conteiner.emplayce_back(substr(pos, iter - pos));

                return conteiner;
            }
        };

    } // namespace str_base

    using string_t = str_base::bsstr_t<char, uint16_t>;
    
}   // ufo