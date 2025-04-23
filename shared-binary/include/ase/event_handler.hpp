#pragma once

#include <string_view>
#include <mutex>
#include <memory>

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
        }

        event_handler() = delete;
        event_handler(std::string_view id, user_event_refs_t ue_refs);
        LV_MgErr_t generate_std_out(std::string_view data);
        LV_MgErr_t generate_std_err(std::string_view data);
        LV_MgErr_t generate_did_exit(int32_t exit_code, std::string_view remaining_out, std::string_view remaining_err);

    private:
        std::mutex m_out_mtx, m_err_mtx;
        std::unique_ptr<LV_StringHandle_t, void (*)(LV_StringHandle_t *)> m_id_string_handle, m_out_string_handle, m_err_string_handle;
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