#pragma once

namespace ufo
{
    namespace detail
    {
        template <typename _Ty>
        bool list_default_compare(const _Ty &a, const _Ty &b)
        {
            return a == b;
        }

        template <typename _Ty>
        struct l_item_t
        {
            _Ty _data;
            l_item_t<_Ty> *_prev = nullptr;
            l_item_t<_Ty> *_next = nullptr;

            
            // l_item_t() : _data(), _prev(nullptr), _next(nullptr) 
            // {}
        public:
            l_item_t(const _Ty &data) : _data(data), _prev(nullptr), _next(nullptr) {}

            template<typename... Args> 
            l_item_t(Args&&... args) : 
                _data(std::forward<Args>(args)...), 
                _prev(nullptr), 
                _next(nullptr) 
            {}
            ~l_item_t(){}
        };

        template <typename _Ty>
        class simple_list_iterator_t
        {
            using item_t = l_item_t<_Ty>;

        private:
            item_t *_obj = nullptr;

        public:
            simple_list_iterator_t() {}
            simple_list_iterator_t(l_item_t<_Ty> *obj) : _obj(obj) {}

            simple_list_iterator_t(const simple_list_iterator_t &other) : _obj(other._obj) {}
            simple_list_iterator_t &operator=(const simple_list_iterator_t &other)
            {
                if (other != *this)
                {
                    _obj = other._obj;
                }
                return *this;
            }

            ~simple_list_iterator_t() {}

            // simple_list_iterator_t(simple_list_iterator_t &&) = delete;
            // simple_list_iterator_t &operator=(simple_list_iterator_t &&) = delete;

            bool operator == (const item_t* item) {
                return (_obj == item);
            }

            _Ty &operator++()
            {
                if (!_obj)
                {
                    throw;
                }
                if (!_obj->_next)
                {
                    item_t *o = _obj;
                    _obj = nullptr;
                    return o->_data;
                }
                _obj = _obj->_next;
                return _obj->_data;
            }

            _Ty &operator--()
            {
                if (!_obj)
                {
                    throw;
                }
                if (!_obj->_prev)
                {
                    item_t *o = _obj;
                    _obj = nullptr;
                    return o->_data;
                }
                _obj = _obj->_prev;
                return _obj->_data;
            }

            operator bool() const
            {
                return _obj;
            }

            _Ty &operator()()
            {
                return _obj->_data;
            }

            _Ty *operator->()
            {
                return &_obj->_data;
            }
        };

    } // namespace detail

    // there are no insert_after, delete after,
    //  no iterators and operations with them
    template <typename _Ty>
    class list_t
    {
    public:
        using item_t = detail::l_item_t<_Ty>;
        using simple_iterator_t = detail::simple_list_iterator_t<_Ty>;
        using comparator_t = bool (*)(const _Ty &, const _Ty &);

    private:
        item_t *_head = nullptr;
        item_t *_tail = nullptr;

        size_t _sz = 0;

    public:
        list_t()
        {
        }
        ~list_t()
        {
            if (_tail)
            {
                while (_head)
                {
                    pop_back();
                }
            }
        }

        list_t(const list_t &other)
        {
            if (other.empty())
            {
                return;
            }

            item_t *other_it = other._head;

            while (other_it)
            {
                insert_back(other_it->_data);
                other_it = other_it->_next;
            }
        }

        // just copy and null ptrs
        list_t(list_t &&other)
            : _head(other._head), _tail(other._tail), _sz(other._sz)
        {
            other._head = nullptr;
            other._tail = nullptr;
            other._sz = 0;
        }

        const list_t &operator=(list_t &&other)
        {
            if (&other != this)
            {
                _head = other._head;
                _tail = other._tail;
                _sz = other._sz;

                other._head = nullptr;
                other._tail = nullptr;
                other._sz = 0;
            }
            return *this;            
        }

        const list_t &operator=(const list_t &other)
        {
            if (&other != this)
            {
                if (!other.empty())
                {
                    item_t *other_it = other._head;
                    while (other_it)
                    {
                        insert_back(other_it->_data);
                        other_it = other_it->_next;
                    }
                }
            }
            return *this;
        }

        void insert_back(const _Ty& data) {
            if (!_head)
            {
                _head = new item_t(data);
                _tail = _head;
                ++_sz;
                return;
            }
            item_t *obj = new item_t(data);
            obj->_prev = _tail;
            _tail->_next = obj;
            _tail = obj;

            // _tail = _tail->_next;
            ++_sz;
        }

        void insert_back(_Ty&& data)
        {
            emplace_back(std::move(data));
            return;
        }

        template<typename... Args>
        void emplace_back(Args&&... args)
        {
            if (!_head)
            {
                _head = new item_t(std::forward<Args>(args)...);
                _tail = _head;
                ++_sz;
                return;
            }
            item_t *obj = new item_t(std::forward<Args>(args)...);
            obj->_prev = _tail;
            _tail->_next = obj;
            _tail = obj;
            ++_sz;
            return;
        }

        void pop_back()
        {
            if (!_head)
            {
                return;
            }

            if (_head == _tail)
            {
                delete _head;
                _head = nullptr;
                _tail = nullptr;
                --_sz;
                return;
            }

            _tail = _tail->_prev;
            delete _tail->_next;
            _tail->_next = nullptr;
            --_sz;
        }

        void pop_front()
        {
            if (!_head)
            {
                return;
            }

            if (_head == _tail)
            {
                delete _head;
                _head = nullptr;
                _tail = nullptr;
                --_sz;
                return;
            }

            _head = _head->_next;
            delete _head->_prev;
            _head->_prev = nullptr;
            --_sz;
        }

        void pop(size_t ind)
        {
            if (ind > _sz - 1)
            {
                return;
            }
            if (!ind)
            {
                pop_front();
                return;
            }
            if (ind == _sz - 1)
            {
                pop_back();
                return;
            }

            size_t cnt = 0;
            item_t *it = nullptr;
            if (ind > (_sz + 1) / 2)
            {
                cnt = _sz;
                it = _tail;
                while (cnt > _sz / 2 - 1 && ind != cnt - 1)
                {
                    --cnt;
                    it = _tail->_prev;
                }
                it->_prev->_next = it->_next;
                it->_next->_prev = it->_prev;
                --_sz;
                delete it;
                return;
            }

            it = _head;
            while (cnt < _sz / 2 + 1 && ind != cnt)
            {
                ++cnt;
                it = it->_next;
            }

            it->_prev->_next = it->_next;
            it->_next->_prev = it->_prev;
            --_sz;
            delete it;
            return;
        }

        void pop(simple_iterator_t &iter)
        {

            if (iter == _tail)
            {
                pop_back();
                iter = simple_iterator_t(nullptr);
                --_sz;
                return;
            }

            if (iter == _head)
            {
                pop_front();
                iter = simple_iterator_t(nullptr);
                --_sz;
                return;
            }

            item_t *it = _tail->_prev;

            while (it != _head)
            {
                if (iter == it)
                {
                    it->_prev->_next = it->_next;
                    it->_next->_prev = it->_prev;
                    delete it;
                    --_sz;
                    iter = simple_iterator_t(nullptr);
                    return;
                }
                it = it->_prev;
            }
            return;
        }

        _Ty &get(size_t ind)
        {
            if (ind > _sz - 1)
            {
                throw;
            }
            if (!ind)
            {
                return _head->_data;
            }
            if (ind == _sz - 1)
            {
                return _tail->_data;
            }

            size_t cnt = 0;
            item_t *it = nullptr;

            if (ind > (_sz + 1) / 2)
            {
                cnt = _sz;
                it = _tail;

                while (cnt > _sz / 2 - 1 && ind != cnt - 1)
                {
                    --cnt;
                    it = _tail->_prev;
                }
                return it->_data;
            }

            it = _head;

            while (cnt < _sz / 2 + 1 && ind != cnt)
            {
                ++cnt;
                it = it->_next;
            }
            return it->_data;
        }

        void pop_if(const _Ty &cond, comparator_t compare = detail::list_default_compare)
        {
            if (empty())
            {
                return;
            }
            
            if (compare(_tail->_data, cond))
            {
                pop_back();
                return;
            }
            if (compare(_head->_data, cond))
            {
                pop_front();
                return;
            }
            item_t *it = _tail->_prev;

            while (it != _head)
            {
                if (compare(it->_data, cond))
                {
                    it->_prev->_next = it->_next;
                    it->_next->_prev = it->_prev;
                    delete it;
                    --_sz;
                    return;
                }
                it = it->_prev;
            }
        }


        template <typename cond_type, typename comparator = bool (*)(const _Ty&, const cond_type&)>
        void pop_if(const cond_type &cond, comparator compare)
        {
            if (empty())
            {
                return;
            }
            
            if (compare(_tail->_data, cond))
            {
                pop_back();
                return;
            }
            if (compare(_head->_data, cond))
            {
                pop_front();
                return;
            }
            item_t *it = _tail->_prev;

            while (it != _head)
            {
                if (compare(it->_data, cond))
                {
                    it->_prev->_next = it->_next;
                    it->_next->_prev = it->_prev;
                    delete it;
                    --_sz;
                    return;
                }
                it = it->_prev;
            }
        }

        // template <typename cond_type, typename comparator = bool (*)(const _Ty&, const cond_type&)>
        // _Ty& get_if(const cond_type &cond, comparator compare){
        //     item_t *it = _tail;
        //     while (it)
        //     {
        //         if (compare(it->_data, cond))
        //         {
        //             return it->_data;
        //         }
        //         it = it->_prev;
        //     }
        //     return _head->_data;
        // }

        // _Ty &get_if(const _Ty &cond, comparator_t compare = detail::list_default_compare)
        // {
        //     item_t *it = _tail;
        //     while (it)
        //     {
        //         if (compare(it->_data, cond))
        //         {
        //             return it->_data;
        //         }
        //         it = it->_prev;
        //     }
        //     return _head->_data;
        // }

        _Ty &front()
        {
            return _head->_data;
        }

        _Ty back()
        {
            return _tail->_data;
        }

        bool empty() const
        {
            return !_sz;
        }

        operator bool() const
        {
            return _sz;
        }

        size_t size() const
        {
            return _sz;
        }

        simple_iterator_t begin() const
        {
            return simple_iterator_t(_head);
        }
        simple_iterator_t end()
        {
            return simple_iterator_t(_tail);
        }
    };

} // namespace ufo
