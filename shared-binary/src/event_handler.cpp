#include "ase/event_handler.hpp"

using namespace ase;

event_handler::event_handler(std::string_view id, user_event_refs_t ue_refs) : m_refs(ue_refs),
                                                                               m_id_string_handle(LV_StringHandle_t::create(id.length())),
                                                                               m_out_string_handle(LV_StringHandle_t::create(4096)),
                                                                               m_err_string_handle(LV_StringHandle_t::create(4096))
{
    m_id_string_handle->copy_from(id);
}

LV_MgErr_t event_handler::generate_std_out(std::string_view data)
{
    const std::lock_guard<std::mutex> lock(m_out_mtx);

    // copy data into m_out_string_handle

    m_out_string_handle->copy_from(data);

    LV_EventStdOut_t event_data(*m_out_string_handle, *m_id_string_handle);

    return PostLVUserEvent(m_refs.std_out, &event_data);
}

LV_MgErr_t event_handler::generate_std_err(std::string_view data)
{
    const std::lock_guard<std::mutex> lock(m_err_mtx);

    // copy data into m_err_string_handle

    m_err_string_handle->copy_from(data);

    LV_EventStdErr_t event_data(*m_err_string_handle, *m_id_string_handle);

    return PostLVUserEvent(m_refs.std_err, &event_data);
}

LV_MgErr_t event_handler::generate_did_exit(int32_t exit_code, std::string_view remaining_out, std::string_view remaining_err)
{
    const std::lock_guard<std::mutex> lock_err(m_err_mtx), lock_out(m_out_mtx);

    m_out_string_handle->copy_from(remaining_out);
    m_err_string_handle->copy_from(remaining_err);

    LV_EventExit_t event_data(exit_code, *m_out_string_handle, *m_err_string_handle, *m_id_string_handle);

    return PostLVUserEvent(m_refs.std_err, &event_data);
}