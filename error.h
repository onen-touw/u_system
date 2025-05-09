#pragma once

#include "error_types.h"
#include "error_helper.h"
#include "trace.h"
#include "mutex.h"

#define UFO_ERR_VEBOSE

namespace ufo {

    class Error_t
    {
        using Crit_t = CriticalError_t;
        using Cnt_t = CountingError_t;
        using Warn_t = Warning_t;
        using lvl_t = WarningLevel_t;
        using cfg = config;

    private:

        static Error_t _instance;

        Cnt_t* _counting = nullptr;     // todo:: create List buff
        Warn_t* _warning = nullptr;     //todo:: List buffer
        CriticalError_t* _critical = nullptr;
        mutex_t _lock; 

        // capacity is constant and is equaled N_want, N_counting
        size_t _sz_cnt = 0;
        size_t _sz_wrn = 0;
        lvl_t _lvl = lvl_t::W_ALL;        // default level
        // std::function<void(void)> _terminator;

    public:
        Error_t() {
            // allocate memory for both _counting errors buf and warnings buf
            _counting = static_cast<Cnt_t*>(operator new((cfg::Counting_capacity) * sizeof(Cnt_t)));    //todo:: List buffer
            _warning = static_cast<Warn_t*>(operator new((cfg::Warning_capacity) * sizeof(Warn_t)));    //todo:: List buffer
        }

        Error_t(const Error_t&) = delete;
        Error_t& operator =(const Error_t&) = delete;

        static Error_t& GetInstance() {
           return _instance;
        }

        ~Error_t() {
            operator delete(_counting);
            operator delete(_warning);
            if (_critical != nullptr)
            {
                delete _critical;
            }
        }

        // void SetTerminator(std::function<void(void)> terminator) {
        //     _terminator = terminator;
        // }

        void SetLevel(WarningLevel_t lvl) {
            _lvl = lvl;
        }

        void Push(CountingError_t&& e) {
            lock_guard<mutex_t>_l(_lock);
            
            if (_HasCritical()) { return ;}

            if (_sz_cnt != 0)
            {
                for (int32_t i = _sz_cnt - 1; i > -1; --i)   // from end to start
                {
                    if (e._info._code == _counting[i]._info._code)
                    {
                        ++_counting[i]._cnt;
                        if (_counting[i]._cnt > cfg::Counting_limit - 1)
                        {
                            if (_lvl != lvl_t::W_2)     // overflowed counting error is error
                            {
                                Crit_t ec;
                                ec._info = GenerateCoreInfo("Counting overflow generate error");
                                _Push(ec);
                                return;
                            }
                            Warn_t ec;
                            ec._info = GenerateCoreInfo("Counting overflow generate warning");
                            _Push(ec);
                            --_sz_cnt;  // do not deallocate buffer when throw out error
                            // when new counting error (cnt = 1) arrives, 
                            // it just will be rewrited on this place //todo:: it is wrong
                        }
                        return;
                    }
                }

                if (_sz_cnt > cfg::Counting_capacity - 1u)
                {
                    Crit_t ec;
                    ec._info = GenerateCoreInfo("Counting buf reach limit");
                    _Push(ec);
                    return;
                }
            }

            Cnt_t* ptr = new (_counting + _sz_cnt) Cnt_t();
            if (!ptr)
            {
                Crit_t ec;
                ec._info = GenerateCoreInfo("Buf alloc error");
                _Push(ec);
                return;
            }
            ++_sz_cnt;

            ptr->_cnt = 1;          // first cnt
            ptr->_info = e._info;   // copy from generated e;
        }

        // void Push(CriticalError_t& e) {
        //     lock_guard<mutex_t>_l(_lock);
            
        //     if (_HasCritical()) { return ;}

        //     _Push(e);
        // }
        void Push(CriticalError_t&& e) {
            lock_guard<mutex_t>_l(_lock);
            
            if (_HasCritical()) { return ;}

            _Push(e);
        }

        void Push(Warning_t&& e) {
            lock_guard<mutex_t>_l(_lock);
            Warning_t ww = e;
#ifdef UFO_ERR_VEBOSE
            Trace_t::log("-v: ");
            Trace_t::log(ww);
#endif
            if (_HasCritical()) { return ;}
            
            _Push(ww);
        }

        void Trace() {
            lock_guard<mutex_t>_l(_lock);

            //
            const char* l = (_lvl == lvl_t::W_ALL) ? "WALL" : (_lvl == lvl_t::W_2) ? "W2" : "W1";
            Trace_t::log("\n+Tracing: ");
            Trace_t::log(l);
            Trace_t::log('\n'); 
            
            if (_sz_wrn > 0)
            {
                Trace_t::log("\t++warnings were found ");
                Trace_t::log(_sz_wrn);
                Trace_t::log("\n");
                for (size_t i = 0; i < _sz_wrn; ++i)
                {
                    Trace_t::log(_warning[i]);
                    Trace_t::log('\n');
                }
            }
             if (_sz_cnt > 0)
            {
                Trace_t::log("\t++countings were found ");
                Trace_t::log(_sz_cnt);
                Trace_t::log("\n");
                for (size_t i = 0; i < _sz_cnt; ++i)
                {
                    Trace_t::log(_counting[i]);
                    Trace_t::log('\n');
                }
            }
            if (_HasCritical())
            {
                Trace_t::log("\t++critical\n");
                Trace_t::log(*_critical);
                Trace_t::log("-Tracing\n");
            }
            
        }


        // no thread-safety
        operator bool() const {
            return _HasCritical();
        }

        bool Citical() {
            lock_guard<mutex_t>_l(_lock);
            return _critical == nullptr;
        }

        void ClearCounting() {
            lock_guard<mutex_t>_l(_lock);

            if (_HasCritical()) { return ;}
            
            _sz_cnt = 0;
        }

        void ClearWarnings() {
            lock_guard<mutex_t>_l(_lock);

            if (_HasCritical()) { return ;}

            _sz_wrn = 0;
        }
        void ForceClearCritical(){
            lock_guard<mutex_t>_l(_lock);

            if (_critical)
            {
                delete _critical;
                _critical = nullptr;
            }
            
        }

        void ClearAll() {
            ClearCounting();
            ClearWarnings();
        }

    private:

        bool _HasCritical() const { return _critical != nullptr; }

        void _Push(Warning_t& e) {
            if (_lvl == lvl_t::W_ALL)
            {
                Crit_t ec;
                ec._info = e._info;
                _Push(ec);
                return;
            }

            if (_sz_wrn != 0)
            {
                if (_sz_wrn > cfg::Warning_capacity - 1u)
                {
                    //shift left
                    // todo LIST for warnings
                    for (size_t i = 1; i < cfg::Warning_capacity; ++i)
                    {
                        _warning[i - 1] = _warning[i];
                    }
                    _warning[cfg::Warning_capacity - 1u] = e;
                    return;
                }
            }
            Warn_t* ptr = new (_warning + _sz_wrn) Warn_t();
            if (!ptr)
            {
                Crit_t ec;
                ec._info = GenerateCoreInfo("Buf alloc error");
            
                _Push(ec);
                return;
            }
            ++_sz_wrn;
            ptr->_info = e._info;   // copy data from generated e;
        }

        void _Push(CriticalError_t& e){
            _critical = new CriticalError_t();
            _critical->_info = e._info;
        }

    };
    Error_t Error_t::_instance;

}   // ufo
