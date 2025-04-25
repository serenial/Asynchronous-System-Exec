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
        LV_UserEventRef_t out, err, exit;
        operator event_handler::user_event_refs_t() const
        {
            event_handler::user_event_refs_t refs;
            refs.did_exit = exit;
            refs.std_err = err;
            refs.std_out = out;
            return refs;
        }
    };
#include "ase/lv_interop/reset_packing.hpp"
}

extern "C"
{
    ASE_EXPORT LV_MgErr_t do_everything(
        LV_ErrorClusterPtr_t error_cluster_ptr,
        LV_StringHandle_t exe_path_handle,
        LV_Ptr_t<LV_EventRefs_t> refs_ptr)
    {
        try
        {
            process p(
                "_id_",
                *refs_ptr,
                boost::regex{"Approximate"},
                boost::regex{"\n"},
                true);

            p.start(exe_path_handle, {"8.8.8.8"}, std::filesystem::path{});

                auto x = p.wait_for_exit_code();
        }
        catch (...)
        {
            error_cluster_ptr.copy_from_exception(std::current_exception(), __func__);
        }
        return LV_ERR_noError;
    }
}