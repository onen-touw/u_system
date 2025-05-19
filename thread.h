#pragma once 

#include "config.h"
#include "invoke.h"
#include "trace.h"
#include "error.h"
// #include <cstring>
#include "string.h" 

#include "list.h"

#include "tctb.h"

// #include <list>

namespace ufo
{
    class thread;

    namespace detail
    {
        struct thread_impl_base
        {
            virtual ~thread_impl_base() = default;
            virtual void Run() = 0;
        };
        using thread_ptr = std::unique_ptr<thread_impl_base>;

        // it is unsave but only parent can set value
        enum class state_t : uint8_t
        {
            // RUN -> TERMINATING -> IDLE
            RUN,
            TERMINATE,
            IDLE,    
            JOINED,   
        };

        static uint32_t u_thread_cnt = 0;

        inline void RTOS_task(void *arg);

        struct thread_context
        {
            TaskHandle_t _parent = nullptr; // no delete    if it is nullptr => task was detached
            uint32_t _id = 0;
            state_t _state = state_t::IDLE;
        };

        struct thread_task_bridge {
            void* _other = nullptr;     
            std::shared_ptr<thread_context> _context;   // no delete (shared_ptr)
            thread_ptr _ptr;                            // no delete (unique_ptr)
        };





    } // namespace detail

    struct token_t
    {
        friend class thread;

    private:
        using state_t = detail::state_t;
        state_t *st = nullptr;

    public:
        token_t(state_t *ptr) : st(ptr)
        {
            // std::cout << "token_t::constructor\n";
        }

        ~token_t()
        {
            // std::cout << "token_t::destructor\n";
        }

        token_t(token_t &&rhs) : st(std::move(rhs.st))
        {
            rhs.st = nullptr;
            // std::cout << "tok_t::move\n";
        }

        token_t &operator=(const token_t &) = delete;
        token_t(const token_t &) = delete;

        void what()
        {
            if (st == nullptr)
            {
                // std::cout << "null tok\n";
                return;
            }

            switch (*st)
            {
            case state_t::RUN:
                Trace_t::log("tok::run\n");
                break;
            case state_t::TERMINATE:
                Trace_t::log("tok::terminate\n");
                break;
            case state_t::IDLE:
                Trace_t::log("tok::idle\n");
                break;
            default:
                Trace_t::log("tok::def\n");
                break;
            }
        }

        // while (token){
        //    do smth;
        // }
        operator bool() const
        {
            if (st == nullptr)
            {
                // std::cout << "critical error:: lose token\n";
                return false;
            }
            return (*st == state_t::RUN);
        }
    };


    struct thread_cfg
    {

        const char* _name = nullptr;
        uint32_t _stackSize = 0;
        uint8_t _prio = 0;
        uint8_t _core = 0;
    };
    

    class thread
    {
    public:
        using Handle_t = TaskHandle_t;
        using state_t = detail::state_t;
        using Context_t = detail::thread_context;
        
    private:
        std::shared_ptr<Context_t> _context;

        ufo::Error_t& _error = ufo::Error_t::GetInstance();

        Handle_t _handle = nullptr;     // no delete

    private:
        // impl = wrapper about { func, token, args... }
        template <typename funk, typename... _Args>
        struct _Imle : detail::thread_impl_base
        {
            funk f;
            std::tuple<typename std::decay<_Args>::type..., token_t> tup;

            _Imle(funk &&f, _Args &&...args, token_t tk)
                : f(std::forward<funk>(f)), tup(std::forward<_Args>(args)..., std::move(tk))
            {}

            void Run() { 
                // std::cout << "Run()-->Call()\n";    
                Call(); 
            }

            template <std::size_t TupSize = std::tuple_size_v<decltype(tup)>>
            void Call()
            {
                _invoke(std::make_index_sequence<TupSize>{});
            }

            template <size_t... ind>
            void _invoke(std::index_sequence<ind...>)
            {
                invoke(f, std::get<ind>(std::move(tup))...);
            }
        };

        class id
        {
        private:
            intptr_t _id = 0;
        public:
            id(TaskHandle_t hand) : _id( reinterpret_cast<intptr_t>(hand)) {}
            uint32_t get() const {
                return static_cast<uint32_t>(_id);
            }
        };
    public:
        thread() {}

        template <typename Call, typename... Args>
        thread(Call && f, Args && ...args)
        {

            thread_cfg cfg;
            cfg._core = 0; 
            cfg._prio = 5; 
            cfg._stackSize = 2048; 
            create(cfg ,std::forward<Call>(f), std::forward<Args>(args)...);
        }

        template <typename Call, typename... Args>
        thread(thread_cfg& cfg, Call && f, Args && ...args)
        {
            create(cfg, std::forward<Call>(f), std::forward<Args>(args)...);
        }

        thread(const thread&) = delete;
        thread(thread&& other) : _context(std::move(other._context)), _handle(other._handle) 
        {
            other._handle = nullptr;
        }

        thread& operator= (const thread&) = delete;
        thread& operator= (thread&& other) {
            if (&other != this)
            {
                if (joinable())
                {
                    terminate();
                    _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::trd_creation, "destruction of an unjoined thread")));
                    return *this;
                }
                
                _context = std::move(other._context);
                _handle = other._handle;
                other._handle = nullptr;
            }
            return *this;
        }

        ~thread()
        {
            // std::cout << "thread::destructor\n";
            if (joinable())
            {
                terminate();
                // CriticalError_t e;
                // e._info = GenerateInfo_Code(error::codes_t::trd_creation, "destruction of an unjoined thread");
                // _error.Push(e);
                _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::trd_creation, "destruction of an unjoined thread")));

            }
        }

        bool joinable() const {
            if (!_context)
            {
                return false;
            }
            return (_context->_state != state_t::JOINED && _context->_parent != nullptr);
        }

    // protected:
    private:
        template <typename Call, typename... Args>
        void create(thread_cfg& cfg, Call && f, Args && ...args)
        {
            if (_context)
            {
                terminate();
                _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::trd_creation, "double-creation")));
                return;
            }
            
            _context = std::make_shared<Context_t>();
           
            ++detail::u_thread_cnt;
            _context->_state = state_t::RUN;
            _context->_parent = xTaskGetCurrentTaskHandle();

            detail::thread_task_bridge* bridge = new detail::thread_task_bridge();

            // throw ptr to void task(void* arg) -freeRTOS-task  /   p->Run();
            bridge->_ptr = std::make_unique<_Imle<Call, Args...>>(std::forward<Call>(f), std::forward<Args>(args)..., token_t(&_context->_state));
            bridge->_context = _context;


            char nname[16] = {'u'};
            if (cfg._name == nullptr)
            {
                itoa(detail::u_thread_cnt, (nname + 1), 10);
                // std::cout << "name" << nname << '\n';
            }
            else {
                // avoid overflow
                memcpy(nname, cfg._name, 16);
            }
            
            xTaskCreatePinnedToCore(detail::RTOS_task, nname, cfg._stackSize, bridge, cfg._prio,  &_handle , cfg._core);
            if (!_handle)
            {
                delete bridge;
                _context.reset();

                _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::trd_creation, "cant create task")));

                return;
            }

            id pid(_handle);
            _context->_id = pid.get();

            // rm
            Trace_t::log("[debug]:: task info:\n");
            Trace_t::log("\tid: ");
            Trace_t::log(_context->_id);
            Trace_t::log("\n");

            detail::u_privite_tskTaskControlBlock* cb = reinterpret_cast<detail::u_privite_tskTaskControlBlock*>(_handle);
            Trace_t::log("\tname: ");
            Trace_t::log(cb->pcTaskName);
            Trace_t::log("\n");

            Trace_t::log("\tprio: ");
            Trace_t::log(cb->uxPriority);
            Trace_t::log("\n");
            // rm

            xTaskNotify(_handle, 0, eNoAction);
        }

    public:
        void join()
        {
            if (!joinable())
            {
                // if it was detached => try to terminate
                if (_context->_parent == nullptr && _context->_state != state_t::JOINED)
                {
                    terminate();
                }
                
                _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::trd_join_unj, "cant join unjoinable task")));

                return;
            }

            // wait child-task while it stop;
            // std::cout << "join::waiting start "<< _context->_id << "\n";
            xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
            // std::cout << "join::waiting end\n";
            
            _context->_state = state_t::JOINED;

            --detail::u_thread_cnt;

        }

        void detach(){
            if (!_context)
            {
                _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::trd_null_opertions, "cant join null-thread")));

                return;
            }

            if (_context->_state != state_t::RUN)
            {
                return;
            }
            if (_context->_parent == nullptr)
            {
                // or warinig
                terminate();
                _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::trd_detach_detached, "task has already been detached")));

                return;
            }
            _context->_parent = nullptr;
        }

        uint32_t get_id() const {
            if (!_context)
            {
                return 0;
            }
            return _context->_id;
        }

        //  terminating of detached threads does not provide a guarantee of termianting task-function
        void terminate()
        {
            if (!_context)
            {
                _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::trd_null_opertions, "cant join null-thread")));

                return;
            }
            
            // std::cout << "terminating::start\n";
            if (_context->_state != state_t::RUN)
            {
                return;
            }
            _context->_state = state_t::TERMINATE;

            // wait();
            // std::cout << "terminating::wait\n";
            if (_context->_parent)
            {
                xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
            }
            // std::cout << "terminating::wait end\n";
            _context->_state = state_t::JOINED;

            --detail::u_thread_cnt;

        }
    };

    class thread_guard
    {
    private:
        thread _t;
    public:
        thread_guard(thread&& t) : _t(std::forward<thread>(t)) {}
        // thread_guard(){}
        thread_guard(thread_guard&) = delete;
        thread_guard& operator=(thread_guard&) = delete;

        thread_guard(thread_guard&& other) : _t(std::move(other._t))
        {}
        thread_guard& operator=(thread_guard&& other) {
            if (&other != this)
            {
                _t = std::move(other._t);
            }
            return *this;
        }


        thread* operator->() { 
            return &_t;
        }

        ~thread_guard() {
            if (_t.joinable())
            {
                _t.join();
            }
        }
    };



    // todo move it into .cpp
    namespace detail
    {
        // freertos-task which will be passed into xTaskCreate...()
        inline void RTOS_task(void *arg)
        {
            utl::sleep_for(5);
            thread_task_bridge* _data = reinterpret_cast<thread_task_bridge*>(arg);

            // wait from parent-task
            xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

            asm volatile ("" : : "r,m"(_data) : "memory");

            _data->_ptr->Run();

            _data->_context->_state = state_t::IDLE;

            TaskHandle_t parent =_data->_context->_parent; 

            delete _data;

            // if parent == nullptr => task was detached
            if (parent != nullptr)
            {
                // intptr_t ptr = reinterpret_cast<intptr_t>(parent);
                
                // notify parent-task when it call .join() or when it is force terminating
                xTaskNotify(parent, 0, eNoAction);
            }

            vTaskDelete(nullptr);
        }
    } // namespace detail

} // namespace ufo
