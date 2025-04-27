//          Copyright serenial.io and contributors.
// Distributed under the Boost Software License, Version 1.0.
//    (See https://www.boost.org/LICENSE_1_0.txt)

#include <boost/process.hpp>
#include <boost/system/result.hpp>

#include "ase/process.hpp"

using namespace ase;

process::process(
    boost::string_view id, 
    event_handler::user_event_refs_t event_refs,
    const boost::regex& std_out_match_regex,
    const boost::regex& std_err_match_regex,
    const LV_StringHandle_t::multibyte_conversion_t conversion
):
    m_std_out_regex(std_out_match_regex),
    m_std_err_regex(std_err_match_regex),
    m_event_handler(id, event_refs, conversion),
    m_std_out_buf(std::make_shared<asio::streambuf>()),
    m_std_err_buf(std::make_shared<asio::streambuf>()),
    m_std_in(m_io_context),
    m_std_out(m_io_context),
    m_std_err(m_io_context),
    m_process_io({m_std_in, m_std_out, m_std_err}),
    m_exit_future(m_exit_promise.get_future()),
    m_process_start_future(m_custom_initializer.get_future()),
    m_io_run_thread([=](){ m_io_context.run(); }),
    m_last_exception(nullptr),
    m_std_out_handler([&](boost::system::error_code ec, size_t transferred)
                    {
                        try{
                            if (!ec)
                            {
                                m_event_handler.generate_std_out(m_std_out_buf, transferred);
                                boost::asio::async_read_until(m_std_out, *m_std_out_buf, m_std_out_regex, m_std_out_handler);
                            }
                        }
                        catch(...){
                            m_last_exception = std::current_exception();
                        } 
                    }),
    m_std_err_handler([&](boost::system::error_code ec, size_t transferred) // copy regex with capture by-val
                        {
                            try{
                                if (!ec)
                                {
                                    m_event_handler.generate_std_err(m_std_err_buf, transferred);
                                    boost::asio::async_read_until(m_std_err, *m_std_err_buf, m_std_err_regex, m_std_err_handler);
                                } 
                            }
                            catch(...){
                                m_last_exception = std::current_exception();
                            }
                        })
{
    // add the async_pipe readers
    boost::asio::async_read_until(m_std_out, *m_std_out_buf, m_std_out_regex, m_std_out_handler);
    boost::asio::async_read_until(m_std_err, *m_std_err_buf, m_std_err_regex, m_std_err_handler);

    // make the process launch a different method call as catching and forwarding errors in the constructor is challenging
}

void process::start_call(
    const std::filesystem::path& exe_path,
    const std::vector<boost::string_view>& exe_args,
    const std::filesystem::path& working_dir
){
    // create a lambda which can pass variable args to the boost::process::process call
    auto execute_with_args = [=](auto &&...args)
    {
        boost::process::async_execute(
            boost::process::process(
                m_io_context,
                exe_path,
                exe_args,
                m_process_io,
                m_custom_initializer,
                std::forward<decltype(args)>(args)...),
            asio::bind_cancellation_slot(m_signal.slot(),
                                         [&](boost::system::error_code ec, int exit_code)
                                         {
                                            try{
                                                if (!ec)
                                                {
                                                    // cancel any pending async_read operations
                                                    m_std_out.cancel();
                                                    m_std_err.cancel();
                                                    // generate did_exit with any contents remaining in the buffers
                                                    m_event_handler.generate_did_exit(exit_code, m_std_out_buf, m_std_out_buf->size(), m_std_err_buf, m_std_err_buf->size());
                                                }
                                            }
                                            catch(...){
                                                // store this exception in the promise
                                                m_exit_promise.set_exception(std::current_exception());
                                            }
                                            try{
                                                m_exit_promise.set_value(exit_code);
                                            }
                                            catch(...){
                                                m_last_exception = std::current_exception();
                                            }
                                         }));
    };

    if (working_dir.empty())
    {
        execute_with_args();
    }
    else
    {
        execute_with_args(boost::process::process_start_dir(working_dir));
    }

    // wait on either launch or error
    // if there was an error the exception will be thrown here
    m_process_start_future.get();
}

void process::write_std_in(boost::string_view data)
{
    m_std_in.write_some(asio::buffer(data, data.length()));
}

bool process::close_std_in()
{
    // check if already closed
    if(!m_std_in.is_open()){
        return true;
    }

    m_std_in.close();

    return false;
}

bool process::send_terminate()
{
    // check if already terminated
    if(m_exit_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready){
        return true;
    }

    m_signal.emit(asio::cancellation_type::terminal);

    return false;
}

bool process::wait_on_completion(std::chrono::milliseconds timeout)
{
    return m_exit_future.wait_for(timeout) == std::future_status::ready;
}

int32_t process::wait_for_exit_code()
{
    return m_exit_future.get();

    if(m_last_exception != nullptr){
        std::rethrow_exception(m_last_exception);
    }
}

process::~process()
{
    send_terminate();
    m_std_out.close();
    m_std_err.close();
    m_std_in.close();

    m_io_run_thread.join();
}

std::filesystem::path process::find_executable_by_name(std::filesystem::path exe_name){
    return boost::process::environment::find_executable(exe_name);
}