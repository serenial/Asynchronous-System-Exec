//          Copyright serenial.io and contributors.
// Distributed under the Boost Software License, Version 1.0.
//    (See https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string_view>
#include <future>
#include <filesystem>
#include <thread>
#include <memory>
#include <exception>
#include <vector>

#include <boost/process.hpp> // defaults to process v2
#include <boost/utility/string_view.hpp>
#include <boost/regex.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp> 

#ifdef _WIN32
#include <boost/process/windows/show_window.hpp>
#endif

#include "./event_handler.hpp"

namespace ase
{

    using namespace lv_interop;
    namespace asio = boost::asio;

    class process{
        public:
        process() = delete;
        process(
            boost::string_view id, 
            event_handler::user_event_refs_t event_refs,
            const boost::regex& std_out_match_regex,
            const boost::regex& std_err_match_regex,
            const LV_StringHandle_t::multibyte_conversion_t conversion
        );
        void start_call(
            const std::filesystem::path& exe_path,
            const std::vector<boost::string_view>& exe_args,
            const std::filesystem::path& working_dir
        );
        ~process();
        bool write_std_in(boost::string_view);
        bool close_std_in();
        bool send_terminate();
        bool wait_on_completion(std::chrono::milliseconds timeout);
        int32_t wait_for_exit_code();
        static std::filesystem::path find_executable_by_name(std::filesystem::path exe_name);
        private:
        // define a class with the template methods to get called by the boost-process lifetime hooks
        // this includes both the windows and posix functions which have different signatures
        class custom_initializer{
            private:
            std::promise<int> m_process_start_promise;
            void set_error(const boost::process::error_code & ec){
                try{
                    boost::process::detail::do_throw_error(ec);
                }
                catch(...){
                    m_process_start_promise.set_exception(std::current_exception());
                }
            }
            void set_ok(){
                m_process_start_promise.set_value(0);
            }

            public:
            custom_initializer() = default;
            std::shared_future<int> get_future(){
                return m_process_start_promise.get_future();
            }
            template<typename Launcher>
            void on_error(Launcher & launcher, const std::filesystem::path &executable, const char * const * (&cmd_line), const boost::process::error_code & ec){
                set_error(ec);
            }
            template<typename Launcher>
            void on_error(Launcher & launcher, const std::filesystem::path &executable, std::wstring &cmd_line, const boost::process::error_code & ec){
                set_error(ec);
            }
            template<typename Launcher>
            void on_success(Launcher & launcher, const std::filesystem::path &executable, const char * const * (&cmd_line)){
                set_ok();
            }
            template<typename Launcher>
            void on_success(Launcher & launcher, const std::filesystem::path &executable, std::wstring &cmd_line){
                set_ok();
            }
            template<typename Launcher>
            boost::system::error_code on_setup(Launcher & launcher, const std::filesystem::path &executable, std::wstring &cmd_line){
                #ifdef _WIN32
                return boost::process::windows::show_window_hide.on_setup(launcher, executable, cmd_line);
                #endif

                return boost::system::error_code();
            }
        };
        std::exception_ptr m_last_exception;
        const boost::regex m_std_out_regex, m_std_err_regex;
        asio::io_context m_io_context;
        event_handler m_event_handler;
        std::shared_ptr<asio::streambuf> m_std_out_buf, m_std_err_buf;
        asio::writable_pipe m_std_in;
        asio::readable_pipe m_std_out, m_std_err;
        boost::process::process_stdio m_process_io;
        custom_initializer m_custom_initializer;
        asio::cancellation_signal m_signal;
        std::promise<int> m_exit_promise;
        std::shared_future<int> m_exit_future, m_process_start_future;
        asio::executor_work_guard<asio::io_context::executor_type> m_work_guard;
        std::thread m_io_run_thread;
        std::function<void(boost::system::error_code ec, size_t transferred)> m_std_out_handler, m_std_err_handler;
    };
}