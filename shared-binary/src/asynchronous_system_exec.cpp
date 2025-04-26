//          Copyright serenial.io and contributors.
// Distributed under the Boost Software License, Version 1.0.
//    (See https://www.boost.org/LICENSE_1_0.txt)

#ifndef ASE_VERSION
#define ASE_VERSION "X.Y.Z"
#endif

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

    struct LV_ConversionEnum_t{
        uint8_t m_value;
        operator LV_StringHandle_t::multibyte_conversion_t() const;
    };

#include "ase/lv_interop/reset_packing.hpp"
}

extern "C"
{
    ASE_EXPORT LV_MgErr_t ase_start_call(
        LV_ErrorClusterPtr_t error_cluster_ptr,
        LV_StringHandle_t exe_handle,
        LV_ArgsListHandle_t args_handle,
        LV_StringHandle_t working_dir_handle,
        LV_StringHandle_t id_handle,
        LV_Ptr_t<LV_EventRefs_t> user_event_refs_ptr,
        LV_StringHandle_t std_out_regex_handle,
        LV_StringHandle_t std_err_regex_handle,
        LV_ConversionEnum_t conversion_type,
        process* process_ptr
    )
    {
        try
        {
            process_ptr = new process(
                exe_handle, 
                args_handle, 
                working_dir_handle,
                id_handle,
                *user_event_refs_ptr,
                std_out_regex_handle,
                std_err_regex_handle,
                conversion_type
            );
        }
        catch (...)
        {
            error_cluster_ptr.copy_from_exception(std::current_exception(), __func__);
        }
        return LV_ERR_noError;
    }
}

LV_EventRefs_t::operator event_handler::user_event_refs_t() const{
    return event_handler::user_event_refs_t{m_out, m_err, m_exit};
}

LV_ArgsListHandle_t::operator std::vector<boost::string_view>() const{

    std::vector<boost::string_view> args;

    auto n = ((m_handle) && (*m_handle))? (*m_handle)->cnt : 0;

    for(int i=0; i< n; i++){
        args.push_back((*m_handle)->args[i]);
    }

    return args;
}

LV_ConversionEnum_t::operator LV_StringHandle_t::multibyte_conversion_t() const{
    const LV_StringHandle_t::multibyte_conversion_t conversions[] =
    {
        LV_StringHandle_t::multibyte_conversion_t::NO_CONVERSION,
        LV_StringHandle_t::multibyte_conversion_t::UTF8_TO_CP_ACP,
        LV_StringHandle_t::multibyte_conversion_t::UTF8_TO_CP_OEMCP
    };

    if (m_value < std::size(conversions))
            {
                return conversions[m_value];
            }

    throw std::out_of_range("The supplied value for the Multi-Byte Character conversion mode does not map to a valid value.");
}