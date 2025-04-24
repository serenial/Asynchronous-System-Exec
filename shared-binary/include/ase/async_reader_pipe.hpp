#pragma once

#include <functional>
#include <boost/asio.hpp>
#include <boost/utility/string_view.hpp>
#include <boost/regex.hpp>

namespace ase{
    // a class which manages asynchronously capturing data in a pipe and copying it into a buffer
    // which then executes a callback when the buffer-data matches the regular expression
    namespace asio = boost::asio;
    class async_reader_pipe{
        public:
        using callback_t = std::function<void(const asio::streambuf&, size_t bytes ,asio::readable_pipe&)>;
        async_reader_pipe() = delete;
        async_reader_pipe(asio::io_context ctx, const boost::regex reg , callback_t callback);
        operator asio::readable_pipe() const;
        void on_data(boost::system::error_code, std::size_t);
        private:
        asio::streambuf m_buffer;
        asio::readable_pipe m_pipe;
        const boost::regex m_regex;
        callback_t m_on_data_callback;
    };
}