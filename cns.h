#pragma once 

#include <stdint.h>
#include <bits/unique_ptr.h>
#include <bits/shared_ptr.h>

#include "invoke.h"
#include "list.h"
#include "str.h"
#include "vct.h"
#include "sdt.h"

#include "u_drivers/uart/UFO_Uart.h"

#include "thread.h"
#include "trace.h"
#include "error.h"

// struct port_t
// {
//     void read(ufo::string_t& buf) {
//         char s[127] = {};
//         std::cin.getline(s, 127);
//         buf = ufo::string_t(s);
//     }

//     void write(const char* msg){
//         std::cout << msg << '\n';
//     }
// };

namespace ufo
{
    namespace cns
    {
        using port_t = ufo::drv::UFO_Uart;

        enum class event_t
        {
            none,
            work,
            exit,
            error
        };
        
        class msg_block_t
        {
        public:
            
        private:
            friend class console_t;
            port_t* _port = nullptr;
            event_t _event = event_t::none;
            ufo::vector_t<string_t> _buf;

        public:
            msg_block_t(){}

            msg_block_t(port_t* port) : 
                _port(port), 
                _event(event_t::work)
            {
                if (!_port)
                {
                    throw;
                }
                
            }
            ~msg_block_t(){}

            bool is_read_out_signal(){
                uint16_t t = _port->Available();
                if (t == 1) {
                    char sig = {};
                    _port->Read(&sig, 1);
                    if (sig == 3 || sig == '~')
                    {
                        return true;
                    }
                }
                return false;
            }

            string_t read(){
                uint16_t t = _port->Available();
                if (t)
                {
                    string_t str(t);
                    _port->Read(str.raw());
                    return str;
                }
                return string_t();
            }

            void fwrite(const char* msg, ...) {
                char data [127]= {};
                if (!strlen(msg))
                {
                    return;
                }
                va_list ap;
                va_start(ap, msg);

                // calculate required string size
                va_list arg;
                va_copy(arg, ap);
                uint32_t req = 1+vsnprintf(NULL, 0, msg, arg);
                va_end(arg);

                if (req > 127)
                {
                    req = 126;
                }

                int n = vsnprintf(data, req, msg, ap);
                va_end(ap);

                _port->SendMsg(data, n);

                return;
            }

            void write(const char* buf){
                _port->SendMsg(buf);
            }

            void write(const string_t& str){
                _port->SendMsg(str.c_str(), str.size());
            }

            event_t get_event() const {
                return _event;
            }
            void exit(){
                _event = event_t::exit;
            }

            ufo::vector_t<string_t>& get_buf(){
                return _buf;
            }

        private:
            void set_buf(ufo::vector_t<string_t>&& buf) {
                _buf = std::move(buf);
            }

        };
        
        class blank_t
        {        
        private:
#ifdef ADVANCED_CONSOLE_BLANK
            struct impl_base_t
            {
                virtual ~impl_base_t() = default;
                virtual void run() = 0;
            };

            template <typename Call, typename... Args>
            struct impl_t : impl_base_t
            {
                Call _call;
                std::tuple<typename std::decay<Args>::type...> _tuple;

                impl_t(Call &&call, Args &&...args) : _call(std::forward<Call>(call)), _tuple(std::forward<Args>(args)...)
                {}

                virtual void run()
                {
                    deffered_call();
                }

                template <std::size_t tup_sz = std::tuple_size_v<decltype(_tuple)>>
                void deffered_call()
                {
                    _invoke(std::make_index_sequence<tup_sz>{});
                }

                template<std::size_t... ind>
                void _invoke(std::index_sequence<ind...>)
                {
                    ufo::invoke(_call, std::get<ind>(_tuple)...);
                }
            };
#else
        public:
            using call_t = void(*)(std::shared_ptr<msg_block_t>);
#endif

        private:
            string_t _class;
            string_t _desc;

            #ifdef ADVANCED_CONSOLE_BLANK
            std::unique_ptr<impl_base_t> _deffered;
            #else
            call_t _foo = nullptr;
            #endif

            
        public:
            // blank_t(){}
#ifdef ADVANCED_CONSOLE_BLANK

            template <typename Call, typename... Args>
            blank_t(const char* cl_name, const char* desc, Call call, Args... args) : 
                _class(cl_name),
                _desc(desc),
                _deffered(std::make_unique<impl_t<Call, Args...>>(std::forward<Call>(call), std::forward<Args>(args)...)) {}
            blank_t(blank_t&& other) :
                _class(std::move(other._class)), 
                _desc(std::move(other._desc)), 
                _deffered(std::move(other._deffered))
                {}
            blank_t& operator = (blank_t&& other) 
            {
                if (&other != this)
                {
                    _class = std::move(other._class);
                    _desc = std::move(other._desc);
                    _deffered = std::move(other._deffered);
                }
                return *this;
            }

#else
            blank_t(const char* cl_name, const char* desc, call_t foo) : 
            _class(cl_name),
            _desc(desc),
            _foo(foo){}

            blank_t(blank_t &&other) : _class(std::move(other._class)),
                                       _desc(std::move(other._desc)),
                                       _foo(other._foo)
            {
                other._foo = nullptr;
            }

            blank_t& operator = (blank_t&& other) 
            {
                if (&other != this)
                {
                    _class = std::move(other._class);
                    _desc = std::move(other._desc);
                    _foo = other._foo;
                    other._foo = nullptr;
                }
                return *this;
            }
#endif

            blank_t(const blank_t&) = delete;
            blank_t& operator= (const blank_t&) = delete;

            ~blank_t() {}

            const string_t& get_name() const {
                return _class;
            } 

            const string_t& get_desc() const {
                return _desc;
            }
            
#ifdef ADVANCED_CONSOLE_BLANK
            void run() {
                _deffered->run();
            }
#else
            void run(std::shared_ptr<msg_block_t>& b){
                if (!_foo)
                {
                    throw;
                }
                _foo(b);
            }
#endif
        };

        enum class obj_t
        {
            flag,
            arg,
            null,
        };
        obj_t get_obj_type(string_t &s)
        {
            if (s.size() > 0)
            {
                if (s[0] == '-')
                {
                    return obj_t::flag;
                }
                return obj_t::arg;
            }
            return obj_t::null;
        }

        void log_incorrect_arg() {
            Trace_t::log("incorrect arg\n");
        }

        char get_flag(string_t &s) {
            return s[1];
        }

        template <typename Ty>
        Ty get_arg(ufo::string_t &s);

        template <>
        void get_arg(ufo::string_t &s);

        template <>
        float get_arg(ufo::string_t &s)
        {
            float f = static_cast<float>(std::atof(s.c_str()));
            return f;
        }

        template <>
        int get_arg(ufo::string_t &s)
        {
            int i = static_cast<float>(std::atoi(s.c_str()));
            return i;
        }

        class console_t
        {
        public:
            using block_t = std::shared_ptr<msg_block_t>;
            using bl_list_t = list_t<blank_t>;
        public:
            static constexpr uint8_t max_blanks = 10;
        private:
            bl_list_t _blanks;
            block_t _block;

        public:
        console_t(){}
            console_t(port_t* port) {
                if (!port)
                {
                    // error                    
                    printf("cns:: !port error\n");
                    throw;
                }
                _block = std::make_shared<msg_block_t>(port);
            }

            console_t(console_t&&) =default;
            console_t& operator= (console_t&&) =default;

            console_t(const console_t&) =delete;
            console_t& operator= (const console_t&) =delete;

            ~console_t() {}

#ifdef ADVANCED_CONSOLE_BLANK
            template <typename Call, typename... Args>
            void mk_blank(const char* cl_name, const char* desc, Call&& call, Args&&... args){
                if (_blanks.size() > max_blanks)
                {
                    // error
                    printf("cns:: max blank\n");
                    return;
                }
                _blanks.emplace_back(cl_name, desc, std::forward<Call>(call), std::forward<Args>(args)..., _block);
            }
#else
            void mk_blank(const char* cl_name, const char* desc, blank_t::call_t foo){
                if (_blanks.size() > max_blanks)
                {
                    // error
                    printf("cns:: max blank\n");
                    return;
                }
                _blanks.emplace_back(cl_name, desc, foo);
            }
#endif

            void rm_blank(const char* cl_name){
                if (_blanks.empty())
                {
                    return;
                }
                
                _blanks.pop_if(cl_name, [](const blank_t& bl, const char* s){
                    if (bl.get_name().equal(s))
                    {
                        return true;
                    }
                    return false;
                });
            }

            void ctask(token_t token){
                sys_data_t& _sys = sys_data_t::get_instanse();

                if (!_sys._cns.get_state())
                {
                    _block->write("bad state\n");
                }
                
                print_class_list();
                while (token && _block->get_event() != event_t::exit)
                {
                    string_t s = _block->read();
                    if (s.size())
                    {
                        parse(s);
                    }
                    utl::sleep_for(100);
                }
                _block->write("<console out\n");
                _sys._cns.stop();
            }

            void print_basic_info() const
            {
                _block->write("type exit for exit from console\n");
            }

            void print_class_list()
            {
                utl::sleep_for(100);
                _block->write("class list:\n");
                if (_blanks.empty())
                {
                    _block->write("\tempty\n");
                    print_basic_info();
                    return;
                }

                for (bl_list_t::simple_iterator_t it = _blanks.begin(); it; ++it)
                {
                    _block->write("\t-");
                    _block->write(it->get_name());
                    _block->write("\n");
                }
                print_basic_info();
            }

            

        private:
            void parse(string_t& s)
            {
                if (_blanks.empty())
                {
                    _block->write("no such class\n");
                    return;
                }
                
                ufo::vector_t<string_t>sep = s.split<ufo::vector_t>(' ');
                if (!sep[0].size())
                {
                    return;
                }

                for (bl_list_t::simple_iterator_t it = _blanks.begin(); it; ++it)
                {
                    if (sep[0] == it->get_name())
                    {                        
                        _block->set_buf(std::move(sep));
#ifdef ADVANCED_CONSOLE_BLANK
                        it->run();
#else
                        it->run(_block);
#endif
                        return;
                    }
                }
                _block->write("no such class\n");
                // for (bl_list_t::simple_iterator_t it = _blanks.begin(); it; ++it)
                // {
                //     if (name == it->get_name())
                //     {
                //         it->run();
                //     }
                // }
            }
        };        




    } // namespace cns
} // namespace ufo
