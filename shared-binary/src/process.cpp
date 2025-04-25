#include <boost/process.hpp>
#include <boost/system/result.hpp>
#include "ase/process.hpp"

using namespace ase;

process::process(
    boost::string_view id,
    event_handler::user_event_refs_t event_refs,
    boost::regex std_out_match_regex,
    boost::regex std_err_match_regex,
    const bool convert_utf8_on_win)
    : m_std_out_regex(std_out_match_regex),
      m_std_err_regex(std_err_match_regex),
      m_io_context(),
      m_work_guard(m_io_context.get_executor()),
      m_io_run_thread([=](){ m_io_context.run();}),
      m_event_handler(id, event_refs, convert_utf8_on_win),
      m_std_out(m_io_context),
      m_std_err(m_io_context),
      m_process_io({nullptr, m_std_out, m_std_err}),
      m_exit_future(m_exit_promise.get_future()),
      m_std_out_handler([&](boost::system::error_code ec, size_t transferred)
                        {
          if (!ec)
          {
              m_event_handler.generate_std_out(m_std_out_buf, transferred);
              boost::asio::async_read_until(m_std_out, m_std_out_buf, m_std_out_regex, m_std_out_handler);
          } }),
      m_std_err_handler([&](boost::system::error_code ec, size_t transferred) // copy regex with capture by-val
                        {
          if (!ec)
          {
              m_event_handler.generate_std_out(m_std_err_buf, transferred);
              boost::asio::async_read_until(m_std_err, m_std_err_buf, m_std_err_regex, m_std_err_handler);
          } })
{
    boost::asio::async_read_until(m_std_out, m_std_out_buf, m_std_out_regex, m_std_out_handler);
    boost::asio::async_read_until(m_std_err, m_std_err_buf, m_std_err_regex, m_std_err_handler);
}

void process::start(
    std::filesystem::path exe_path,
    std::initializer_list<boost::string_view> exe_args,
    std::filesystem::path working_dir)
{
    auto execute_with_args = [=](auto &&...args)
    {
        boost::process::async_execute(
            boost::process::process(
                m_io_context,
                exe_path,
                exe_args,
                m_process_io,
                std::forward<decltype(args)>(args)...),
            asio::bind_cancellation_slot(m_signal.slot(),
                                         [&](boost::system::error_code ec, int exit_code)
                                         {
                                             if (!ec)
                                             {
                                                m_std_out.cancel();
                                                m_std_err.cancel();

                                                m_event_handler.generate_did_exit(exit_code, m_std_out_buf, m_std_out_buf.size(), m_std_err_buf, m_std_err_buf.size());
                                             }

                                             m_exit_promise.set_value(exit_code);
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

    m_work_guard.reset();
}

void process::write_std_in(boost::string_view)
{
}
void process::close_std_in()
{
}
void process::send_terminate()
{
    m_signal.emit(asio::cancellation_type::terminal);
}
bool process::wait_on_completion(std::chrono::milliseconds timeout)
{
    return m_exit_future.wait_for(timeout) == std::future_status::ready;
}

int32_t process::wait_for_exit_code()
{
    return m_exit_future.get();
}

process::~process()
{
    m_work_guard.reset();
    //send_terminate();
    m_std_out.close();
    m_std_err.close();

    m_io_run_thread.join();
}