#ifdef _WIN32
#include <Windows.h>
#endif

#include "ase/event_handler.hpp"
#include "ase/lv_interop/lv_functions.hpp"

using namespace ase;
using namespace lv_interop;

event_handler::event_handler(boost::string_view id, user_event_refs_t ue_refs, bool convert_utf): 
    m_refs(ue_refs),
    m_convert_utf8(convert_utf),
    m_id_string_handle(LV_StringHandle_t::create(0)),
    m_out_string_handle(LV_StringHandle_t::create(4096)),
    m_err_string_handle(LV_StringHandle_t::create(4096))
{
    *m_id_string_handle = id;
}

LV_MgErr_t event_handler::generate_std_out(const boost::asio::streambuf& data, size_t bytes)
{
    const std::lock_guard<std::mutex> lock(m_out_mtx);

    // copy data into m_out_string_handle
    m_out_string_handle->copy_from_streambuf(data, bytes, m_convert_utf8);

    LV_EventStdOut_t event_data(*m_out_string_handle, *m_id_string_handle);

    return PostLVUserEvent(m_refs.std_out, &event_data);
}

LV_MgErr_t event_handler::generate_std_err(const boost::asio::streambuf& data, size_t bytes)
{
    const std::lock_guard<std::mutex> lock(m_err_mtx);

    // copy data into m_err_string_handle
    m_err_string_handle->copy_from_streambuf(data, bytes, m_convert_utf8);

    LV_EventStdErr_t event_data(*m_err_string_handle, *m_id_string_handle);

    return PostLVUserEvent(m_refs.std_err, &event_data);
}

LV_MgErr_t event_handler::generate_did_exit(int32_t exit_code, const boost::asio::streambuf& remaining_out, size_t remaining_out_bytes, const boost::asio::streambuf& remaining_err, size_t remaining_err_bytes)
{
    const std::lock_guard<std::mutex> lock_err(m_err_mtx), lock_out(m_out_mtx);

    m_out_string_handle->copy_from_streambuf(remaining_out, remaining_out_bytes, m_convert_utf8);
    m_err_string_handle->copy_from_streambuf(remaining_err, remaining_err_bytes, m_convert_utf8);

    LV_EventExit_t event_data(exit_code, *m_out_string_handle, *m_err_string_handle, *m_id_string_handle);

    return PostLVUserEvent(m_refs.std_err, &event_data);
}

event_handler::~event_handler(){
    LV_StringHandle_t::destroy(m_out_string_handle);
    LV_StringHandle_t::destroy(m_err_string_handle);
    LV_StringHandle_t::destroy(m_id_string_handle);
}