//          Copyright serenial.io and contributors.
// Distributed under the Boost Software License, Version 1.0.
//    (See https://www.boost.org/LICENSE_1_0.txt)

#ifndef ASE_VERSION
#define ASE_VERSION "X.Y.Z"
#endif

#include <system_error>

#include "ase/process.hpp"
#include "ase/lv_interop/lv_error.hpp"
#include "ase/lv_interop/lv_str.hpp"
#include "ase_export.h"

using namespace ase;
using namespace lv_interop;

namespace
{
#include "ase/lv_interop/set_packing.hpp"
    struct LV_EventRefs_t
    {
        LV_UserEventRef_t m_out, m_err, m_exit;
        operator event_handler::user_event_refs_t() const;
    };

    class LV_ArgsListHandle_t
    {
    private:
        struct LV_ArgList_t
        {
            int32_t cnt;
            LV_StringHandle_t args[1];
        };

        LV_Handle_t<LV_ArgList_t> m_handle;

    public:
        LV_ArgsListHandle_t() = delete;
        operator std::vector<boost::string_view>() const;
    };

    struct LV_ConversionEnum_t
    {
        uint8_t m_value;
        operator LV_StringHandle_t::multibyte_conversion_t() const;
    };

#include "ase/lv_interop/reset_packing.hpp"
}

extern "C"
{
    ASE_EXPORT LV_MgErr_t ase_version(
        LV_ErrorClusterPtr_t error_cluster_ptr,
        LV_StringHandle_t version_handle)
    {
        try
        {
            version_handle = std::string{ASE_VERSION};
        }
        catch (...)
        {
            error_cluster_ptr.copy_from_exception(std::current_exception(), __func__);
        }
        return LV_ERR_noError;
    }

    ASE_EXPORT LV_MgErr_t ase_start_call(
        LV_ErrorClusterPtr_t error_cluster_ptr,
        LV_StringHandle_t exe_handle,
        LV_ArgsListHandle_t args_handle,
        LV_StringHandle_t working_dir_handle,
        LV_BooleanPtr_t use_working_dir,
        LV_StringHandle_t id_handle,
        LV_Ptr_t<LV_EventRefs_t> user_event_refs_ptr,
        LV_StringHandle_t std_out_regex_handle,
        LV_StringHandle_t std_err_regex_handle,
        LV_ConversionEnum_t conversion_type,
        process **process_handle)
    {
        try
        {
            *process_handle = new process(
                id_handle,
                *user_event_refs_ptr,
                std_out_regex_handle,
                std_err_regex_handle,
                conversion_type);

                (*process_handle)->start_call(
                exe_handle,
                args_handle,
                *use_working_dir ? working_dir_handle : std::filesystem::path{}
            );
        }
        catch (...)
        {
            error_cluster_ptr.copy_from_exception(std::current_exception(), __func__);
        }
        return LV_ERR_noError;
    }

    ASE_EXPORT LV_MgErr_t ase_destroy(
        LV_ErrorClusterPtr_t error_cluster_ptr,
        process *process_ptr,
        int32_t *exit_code)
    {
        try
        {
            if (process_ptr == nullptr)
            {
                throw std::invalid_argument("Process pointer is invalid.");
            }

            *exit_code = process_ptr->wait_for_exit_code();
        }
        catch (...)
        {
            error_cluster_ptr.copy_from_exception(std::current_exception(), __func__);
        }

        delete (process_ptr);

        return LV_ERR_noError;
    }

    ASE_EXPORT LV_MgErr_t ase_write_std_in(
        LV_ErrorClusterPtr_t error_cluster_ptr,
        process *process_ptr,
        LV_StringHandle_t std_in_handle,
        LV_BooleanPtr_t already_closed)
    {
        try
        {
            if (process_ptr == nullptr)
            {
                throw std::invalid_argument("Process pointer is invalid.");
            }

            try
            {
                *already_closed = process_ptr->write_std_in(std_in_handle);
            }
            catch (...)
            {
                std::rethrow_exception(std::current_exception());
            }
        }
        catch (...)
        {
            error_cluster_ptr.copy_from_exception(std::current_exception(), __func__);
        }
        return LV_ERR_noError;
    }

    ASE_EXPORT LV_MgErr_t ase_close_std_in(
        LV_ErrorClusterPtr_t error_cluster_ptr,
        process *process_ptr,
        LV_BooleanPtr_t already_closed
    )
    {
        try
        {
            if (process_ptr == nullptr)
            {
                throw std::invalid_argument("Process pointer is invalid.");
            }
            *already_closed = process_ptr->close_std_in();
        }
        catch (...)
        {
            error_cluster_ptr.copy_from_exception(std::current_exception(), __func__);
        }
        return LV_ERR_noError;
    }

    ASE_EXPORT LV_MgErr_t ase_wait_on_call(
        LV_ErrorClusterPtr_t error_cluster_ptr,
        process *process_ptr,
        int32_t timeout_ms,
        LV_Ptr_t<LV_UserEventRef_t> result_ref)
    {
        try
        {
            if (process_ptr == nullptr)
            {
                throw std::invalid_argument("Process pointer is invalid.");
            }

            // we don't want to consume all of LabVIEW's threads waiting on these
            // so we will run this asynchronously in our own thread and signal
            // with a user event when done
            std::thread t{[=]()
                          {
                              try
                              {
                                  if (timeout_ms < 0)
                                  {
                                      process_ptr->wait_for_exit_code();
                                      throw std::system_error(0, std::generic_category());
                                  }

                                  throw std::system_error(process_ptr->wait_on_completion(std::chrono::milliseconds(timeout_ms)) ? 0 : 56, std::generic_category());
                              }
                              catch (...)
                              {
                                  auto e = std::unique_ptr<LV_ErrorClusterPtr_t>(LV_ErrorClusterPtr_t::create(std::current_exception(), "wait on call (asynchronous thread)"));

                                  PostLVUserEvent(*result_ref, *e);
                              } }};
            t.detach();
        }
        catch (...)
        {
            error_cluster_ptr.copy_from_exception(std::current_exception(), __func__);
        }
        return LV_ERR_noError;
    }

    ASE_EXPORT LV_MgErr_t ase_send_terminate(
        LV_ErrorClusterPtr_t error_cluster_ptr,
        process *process_ptr,
        LV_BooleanPtr_t already_terminated)
    {
        try
        {
            if (process_ptr == nullptr)
            {
                throw std::invalid_argument("Process pointer is invalid.");
            }
            *already_terminated = process_ptr->send_terminate();
        }
        catch (...)
        {
            error_cluster_ptr.copy_from_exception(std::current_exception(), __func__);
        }
        return LV_ERR_noError;
    }

    ASE_EXPORT LV_MgErr_t ase_find_executable_path(
        LV_ErrorClusterPtr_t error_cluster_ptr,
        LV_StringHandle_t exe_name,
        LV_ConversionEnum_t conversion,
        LV_BooleanPtr_t found)
    {
        try
        {
            auto result = process::find_executable_by_name(exe_name);

            if (result.empty())
            {
                return LV_ERR_noError;
            }

            *found = true;
            exe_name.copy_from_char_ptr(result.c_str(), conversion);
        }
        catch (...)
        {
            error_cluster_ptr.copy_from_exception(std::current_exception(), __func__);
        }
        return LV_ERR_noError;
    }
}

////////////////////////////////////////////////////////////////////////////////////////
//                                                                                    //
////////////////////////////////////////////////////////////////////////////////////////

LV_EventRefs_t::operator event_handler::user_event_refs_t() const
{
    return event_handler::user_event_refs_t{m_out, m_err, m_exit};
}

LV_ArgsListHandle_t::operator std::vector<boost::string_view>() const
{

    std::vector<boost::string_view> args;

    auto n = ((m_handle) && (*m_handle)) ? (*m_handle)->cnt : 0;

    for (int i = 0; i < n; i++)
    {
        args.push_back((*m_handle)->args[i]);
    }

    return args;
}

LV_ConversionEnum_t::operator LV_StringHandle_t::multibyte_conversion_t() const
{
    const LV_StringHandle_t::multibyte_conversion_t conversions[] =
        {
            LV_StringHandle_t::multibyte_conversion_t::NO_CONVERSION,
            LV_StringHandle_t::multibyte_conversion_t::UTF8_TO_CP_ACP,
            LV_StringHandle_t::multibyte_conversion_t::UTF8_TO_CP_OEMCP};

    if (m_value < std::size(conversions))
    {
        return conversions[m_value];
    }

    throw std::out_of_range("The supplied value for the Multi-Byte Character conversion mode does not map to a valid value.");
}