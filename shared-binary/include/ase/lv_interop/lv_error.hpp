#pragma once

#include <exception>
#include <stdexcept>
#include "./lv_types.hpp"
#include "./lv_str.hpp"
#include "./lv_functions.hpp"

using namespace ase;
using namespace lv_interop;

namespace ase
{
    namespace lv_interop
    {
        // LabVIEW Error Cluster type
        class LV_ErrorClusterPtr_t
        {
        public:
            void copy_from_exception(std::exception_ptr ex, const char *caller_name);
            static LV_ErrorClusterPtr_t* create(std::exception_ptr ex, const char *caller_name);
            static void destroy(LV_ErrorClusterPtr_t* e);
            operator void *() const;
        private:
        LV_ErrorClusterPtr_t() = default;
#include "./set_packing.hpp"
            struct LV_Error_t
            {
                LV_Boolean_t status;
                LV_MgErr_t code;
                LV_StringHandle_t source;
            };
#include "./reset_packing.hpp"
            LV_Ptr_t<LV_Error_t> m_err;
        };
    }
}