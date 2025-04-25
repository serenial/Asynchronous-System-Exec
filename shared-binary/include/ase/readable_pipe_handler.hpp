#pragma once

#include <functional>
#include <string>
#include <boost/asio.hpp>
#include <boost/utility/string_view.hpp>
#include <boost/regex.hpp>

namespace ase{
    // a class which manages asynchronously capturing data in a pipe and copying it into a buffer
    // which then executes a callback when the buffer-data matches the regular expression
    namespace asio = boost::asio;
    class readable_pipe_handler{
        public:
        using callback_t = std::function<void(const asio::streambuf& buf, size_t bytes ,asio::readable_pipe& pipe)>;
        readable_pipe_handler() = delete;
        readable_pipe_handler(asio::any_io_executor ex, const boost::regex reg , callback_t callback);
        std::string drain_and_close();
        ~readable_pipe_handler();
        std::reference_wrapper<asio::readable_pipe> pipe();
        void allow_completion();
        void on_data(boost::system::error_code, std::size_t);
        private:
        asio::streambuf m_buffer;
        asio::readable_pipe m_pipe;
        const boost::regex m_regex;
        callback_t m_on_data_callback;
        asio::executor_work_guard<asio::any_io_executor> m_work_guard;
    };
}