#include <boost/bind.hpp>

#include "ase/async_reader_pipe.hpp"

using namespace ase;
namespace asio = boost::asio;

async_reader_pipe::async_reader_pipe(asio::io_context ctx, const boost::regex reg, async_reader_pipe::callback_t callback)
    : m_pipe(ctx),
      m_regex(reg),
      m_on_data_callback(callback)
{
    // start the asynchronous reading by calling on_data with no bytes-transferred
    on_data(make_error_code(boost::system::errc::success), 0);
}

void async_reader_pipe::on_data(boost::system::error_code ec, std::size_t transferred)
{
    if (!ec)
    {

        if (transferred > 0)
        {
            // pass buffer and pipe to callback
            m_on_data_callback(m_buffer, transferred, m_pipe);
            // remove the bytes that should have been consumed
            m_buffer.consume(transferred);
        }

        // check that the pipe is still open
        if (!m_pipe.is_open())
        {
            return;
        }

        // requeue this async-read-task
        asio::async_read_until(m_pipe, m_buffer, m_regex, boost::bind(&async_reader_pipe::on_data, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
    }
}
