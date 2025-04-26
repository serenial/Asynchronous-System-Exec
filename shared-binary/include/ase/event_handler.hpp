#pragma once

#include <mutex>
#include <memory>
#include <string>
#include <utility>

#include <boost/asio.hpp>

#include "./lv_interop/lv_str.hpp"

namespace ase
{

    using namespace lv_interop;

    class event_handler
    {
    public:
        struct user_event_refs_t
        {
            LV_UserEventRef_t std_out, std_err, did_exit;
        };

        event_handler() = delete;
        ~event_handler();
        event_handler(boost::string_view id, user_event_refs_t ue_refs, LV_StringHandle_t::multibyte_conversion_t conversion);
        LV_MgErr_t generate_std_out(std::shared_ptr<boost::asio::streambuf> data, size_t bytes);
        LV_MgErr_t generate_std_err(std::shared_ptr<boost::asio::streambuf> data, size_t bytes);
        LV_MgErr_t generate_did_exit(int32_t exit_code, std::shared_ptr<boost::asio::streambuf> std_out_buf, size_t std_out_size, std::shared_ptr<boost::asio::streambuf> std_err_buf, size_t std_err_size);

    private:
        std::string m_out_str, m_err_str;
        LV_StringHandle_t *m_id_string_handle, *m_out_string_handle, *m_err_string_handle;
        const LV_StringHandle_t::multibyte_conversion_t m_conversion;
        const user_event_refs_t m_refs;

#include "./lv_interop/set_packing.hpp"

        struct LV_EventConsoleOut_t
        {
            LV_EventConsoleOut_t(const LV_StringHandle_t data, const LV_StringHandle_t id) : data(data), id(id) {}
            const LV_StringHandle_t data, id;
        };

        struct LV_EventExit_t
        {
            LV_EventExit_t(const int32_t exit_code, const LV_StringHandle_t remaining_out, const LV_StringHandle_t remaining_err, const LV_StringHandle_t id) : exit_code(exit_code), remaining_out(remaining_out), remaining_err(remaining_err), id(id) {}
            const int32_t exit_code;
            const LV_StringHandle_t id, remaining_out, remaining_err;
        };

        using LV_EventStdOut_t = LV_EventConsoleOut_t;
        using LV_EventStdErr_t = LV_EventConsoleOut_t;

#include "./lv_interop/reset_packing.hpp"
    };
}