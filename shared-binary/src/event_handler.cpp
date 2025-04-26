#include <memory>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "ase/event_handler.hpp"
#include "ase/lv_interop/lv_functions.hpp"

using namespace ase;
using namespace lv_interop;

event_handler::event_handler(boost::string_view id, user_event_refs_t ue_refs, LV_StringHandle_t::multibyte_conversion_t conversion): 
    m_refs(ue_refs),
    m_conversion(conversion),
    m_id_string_handle(LV_StringHandle_t::create(0)),
    m_out_string_handle(LV_StringHandle_t::create(4096)),
    m_err_string_handle(LV_StringHandle_t::create(4096))
{
    *m_id_string_handle = id;
}

LV_MgErr_t event_handler::generate_std_out(std::shared_ptr<boost::asio::streambuf> data, size_t bytes)
{
    // copy data into m_out_string_handle
    m_out_string_handle->consume_from_streambuf(data, bytes, m_conversion);

    LV_EventStdOut_t event_data(*m_out_string_handle, *m_id_string_handle);

    return PostLVUserEvent(m_refs.std_out, &event_data);
}

LV_MgErr_t event_handler::generate_std_err(std::shared_ptr<boost::asio::streambuf> data, size_t bytes)
{

    // copy data into m_err_string_handle
    m_err_string_handle->consume_from_streambuf(data, bytes, m_conversion);

    LV_EventStdErr_t event_data(*m_err_string_handle, *m_id_string_handle);

    return PostLVUserEvent(m_refs.std_err, &event_data);
}

LV_MgErr_t event_handler::generate_did_exit(int32_t exit_code, std::shared_ptr<boost::asio::streambuf>std_out_buf, size_t std_out_size, std::shared_ptr<boost::asio::streambuf>std_err_buf, size_t std_err_size)
{
    // create some LV_StringHandle_t for this event
    using unique_lv_string_handle_t = std::unique_ptr<LV_StringHandle_t, decltype(&LV_StringHandle_t::destroy)>;

    unique_lv_string_handle_t remaining_out_handle{LV_StringHandle_t::create(std_out_size), &LV_StringHandle_t::destroy};
    unique_lv_string_handle_t remaining_err_handle{LV_StringHandle_t::create(std_err_size), &LV_StringHandle_t::destroy};

    remaining_out_handle->consume_from_streambuf(std_out_buf, std_out_size, m_conversion);
    remaining_err_handle->consume_from_streambuf(std_err_buf, std_err_size, m_conversion);

    LV_EventExit_t event_data(exit_code, *remaining_out_handle, *remaining_err_handle, *m_id_string_handle);

    return PostLVUserEvent(m_refs.did_exit, &event_data);
}

event_handler::~event_handler(){
    LV_StringHandle_t::destroy(m_out_string_handle);
    LV_StringHandle_t::destroy(m_err_string_handle);
    LV_StringHandle_t::destroy(m_id_string_handle);
}