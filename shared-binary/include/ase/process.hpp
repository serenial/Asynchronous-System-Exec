#pragma once;

#include <string_view>

#include <boost/filesystem.hpp>
#include <boost/process.hpp> // defaults to process v2
#include <boost/utility/string_view.hpp>
#include <boost/regex.hpp>
#include <boost/asio.hpp>
#include <boost/thread/future.hpp>

#include "./event_handler.hpp"

namespace ase
{

    using namespace lv_interop;
    namespace bpv = boost::process::v2;
    namespace asio = boost::asio;

    class process{
        public:
        process() = delete;
        process( 
            boost::filesystem::path exe_path, 
            std::initializer_list<boost::string_view> args,
            boost::filesystem::path working_dir,
            boost::string_view id, 
            event_handler::user_event_refs_t event_refs,
            boost::regex std_out_match_regex,
            boost::regex std_err_match_regex,
            const bool convert_utf8_on_win
        );
        void write_std_in(boost::string_view) const;
        void close_std_in() const;
        void send_terminate() const;
        int32_t wait_on_completion(std::chrono::milliseconds timeout) const;

        private:
        asio::io_context io_context;
        asio::executor_work_guard<asio::io_context::executor_type> work;
        //boost::threads::shared_future await_completion;

    };
}