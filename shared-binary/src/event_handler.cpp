#include <memory>

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
    // copy data into m_out_string_handle
    m_out_string_handle->copy_from_streambuf(data, bytes, m_convert_utf8);

    LV_EventStdOut_t event_data(*m_out_string_handle, *m_id_string_handle);

    return PostLVUserEvent(m_refs.std_out, &event_data);
}

LV_MgErr_t event_handler::generate_std_err(const boost::asio::streambuf& data, size_t bytes)
{

    // copy data into m_err_string_handle
    m_err_string_handle->copy_from_streambuf(data, bytes, m_convert_utf8);

    LV_EventStdErr_t event_data(*m_err_string_handle, *m_id_string_handle);

    return PostLVUserEvent(m_refs.std_err, &event_data);
}

LV_MgErr_t event_handler::generate_did_exit(int32_t exit_code, const std::string& remaining_out, const std::string& remaining_err)
{

    // create some LV_StringHandle_t for this event
    using unique_lv_string_handle_t = std::unique_ptr<LV_StringHandle_t, decltype(&LV_StringHandle_t::destroy)>;

    unique_lv_string_handle_t remaining_out_handle{LV_StringHandle_t::create(remaining_out.length()), &LV_StringHandle_t::destroy};
    unique_lv_string_handle_t remaining_err_handle{LV_StringHandle_t::create(remaining_err.length()), &LV_StringHandle_t::destroy};

    remaining_out_handle->copy_from_string(remaining_out, m_convert_utf8);
    remaining_err_handle->copy_from_string(remaining_err, m_convert_utf8);

    LV_EventExit_t event_data(exit_code, *remaining_out_handle, *remaining_err_handle, *m_id_string_handle);

    return PostLVUserEvent(m_refs.did_exit, &event_data);
}

event_handler::~event_handler(){
    LV_StringHandle_t::destroy(m_out_string_handle);
    LV_StringHandle_t::destroy(m_err_string_handle);
    LV_StringHandle_t::destroy(m_id_string_handle);
}