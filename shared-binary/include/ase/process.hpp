#pragma once

#include <string_view>
#include <future>
#include <filesystem>
#include <thread>

#include <boost/process.hpp> // defaults to process v2
#include <boost/utility/string_view.hpp>
#include <boost/regex.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp> 

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
            boost::regex std_out_match_regex,
            boost::regex std_err_match_regex,
            const bool convert_utf8_on_win
        );
        ~process();
        void start(
            std::filesystem::path exe_path, 
            std::initializer_list<boost::string_view> args,
            std::filesystem::path working_dir
        );
        void write_std_in(boost::string_view);
        void close_std_in();
        void send_terminate();
        bool wait_on_completion(std::chrono::milliseconds timeout);
        int32_t wait_for_exit_code();
        private:
        boost::regex m_std_out_regex, m_std_err_regex;
        asio::io_context m_io_context;
        asio::executor_work_guard<asio::io_context::executor_type> m_work_guard;
        event_handler m_event_handler;
        asio::streambuf m_std_out_buf, m_std_err_buf;
        asio::readable_pipe m_std_out, m_std_err;
        asio::cancellation_signal m_signal;
        std::promise<int> m_exit_promise;
        std::shared_future<int> m_exit_future;
        std::thread m_io_run_thread;
        std::function<void(boost::system::error_code ec, size_t transferred)> m_std_out_handler, m_std_err_handler;
    };
}