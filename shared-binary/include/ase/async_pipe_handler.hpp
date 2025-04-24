#pragma once

#include <functional>
#include <boost/asio.hpp>
#include <boost/utility/string_view.hpp>
#include <boost/regex.hpp>

namespace ase{
    // a class which manages asynchronously capturing data in a pipe and copying it into a buffer
    // which then executes a callback when the buffer-data matches the regular expression
    namespace asio = boost::asio;
    class async_pipe_handler{
        public:
        async_pipe_handler() = delete;
        async_pipe_handler(asio::io_context ctx, const boost::regex reg ,std::function<void(boost::system::error_code, boost::string_view)> callback);
        operator asio::readable_pipe() const;
        void on_data(boost::system::error_code, std::size_t);
        private:
        asio::streambuf m_buffer;
        asio::readable_pipe m_pipe;
        const boost::regex m_regex;
    };
}