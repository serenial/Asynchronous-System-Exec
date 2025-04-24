#include <boost/bind.hpp>

#include "ase/async_pipe_handler.hpp"

using namespace ase;
namespace asio = boost::asio;

async_pipe_handler::async_pipe_handler(
    asio::io_context ctx, const boost::regex reg,
    std::function<void(boost::system::error_code,
                       boost::string_view)>
        callback)
    : m_pipe(ctx),
      m_regex(reg)
{
    on_data(make_error_code(boost::system::errc::success), 0);
}

void async_pipe_handler::on_data(boost::system::error_code ec, std::size_t transferred)
{
    asio::async_read_until(m_pipe, m_buffer, m_regex, boost::bind(&async_pipe_handler::on_data, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
}