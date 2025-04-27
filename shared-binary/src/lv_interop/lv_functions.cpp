#include <algorithm>
#include <vector>
#include <iterator>
#include <stdexcept>
#include <string>

#include "ase/lv_interop/lv_functions.hpp"
#include "ase/lv_interop/lv_error.hpp"
#include "ase_export.h"

using namespace ase;
using namespace lv_interop;

namespace
{
    static std::string lv_runtime_path_windows;
    // static function pointers
    static LV_DSDisposeHandleFnPtr_t DSDisposeHandleImp = nullptr;
    static LV_DSGetHandleSizePtr_t DSGetHandleSizeImp = nullptr;
    static LV_NumericArrayResizePtr_t NumericArrayResizeImp = nullptr;
    static LV_PostLVUserEventPtr_t PostLVUserEventImp = nullptr;
}

// call this function when the .so is loaded on linux
#if defined(__GNUC__) && !defined(_WIN32)
__attribute__((constructor))
#endif
void ase::on_shared_library_load()
{
    // Import functions from LabVIEW IDE or LabVIEW Runtime
#ifdef _WIN32

            HMODULE module = nullptr;

            if(lv_runtime_path_windows.empty()){
                // try loading from the normal runtimes
                static const char * const runtimes [] = {"LabVIEW.exe","lvffrt.dll","lvrt.dll"};

                // work through the list of runtimes
                for(const auto & runtime : runtimes){
                    module = GetModuleHandle(runtime);
                    if(module) break;
                }
    
                if (!module)
                {
                    return;
                }
            }
            else{
                module = GetModuleHandle(lv_runtime_path_windows.c_str());
                if(!module){
                    // clear the path
                    lv_runtime_path_windows = "";
                    return;
                }
            }

            DSDisposeHandleImp = reinterpret_cast<LV_DSDisposeHandleFnPtr_t>(GetProcAddress(module, "DSDisposeHandle"));
            DSGetHandleSizeImp = reinterpret_cast<LV_DSGetHandleSizePtr_t>(GetProcAddress(module, "DSGetHandleSize"));
            NumericArrayResizeImp = reinterpret_cast<LV_NumericArrayResizePtr_t>(GetProcAddress(module, "NumericArrayResize"));
            PostLVUserEventImp = reinterpret_cast<LV_PostLVUserEventPtr_t>(GetProcAddress(module, "PostLVUserEvent"));
#else
            auto module = dlopen(nullptr, RTLD_LAZY);

            if (!module)
            {
                return;
            }

            DSDisposeHandleImp = reinterpret_cast<LV_DSDisposeHandleFnPtr_t>(dlsym(module, "DSDisposeHandle"));
            DSGetHandleSizeImp = reinterpret_cast<LV_DSGetHandleSizePtr_t>(dlsym(module, "DSGetHandleSize"));
            NumericArrayResizeImp = reinterpret_cast<LV_NumericArrayResizePtr_t>(dlsym(module, "NumericArrayResize"));
            PostLVUserEventImp = reinterpret_cast<LV_PostLVUserEventPtr_t>(dlsym(module, "PostLVUserEvent"));

#endif
}

#if defined(__GNUC__) && !defined(_WIN32)
__attribute__((destructor))
#endif
void ase::on_shared_library_unload()
{
    // nothing to cleanup
}

#if defined(_WIN32)
#if defined(_MSC_VER)
#pragma warning(push, 3)
#endif
#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        on_shared_library_load();
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        on_shared_library_unload();
        break;
    }
    return true;
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif

LV_MgErr_t lv_interop::DSDisposeHandle(LV_UHandle_t hndl)
{
    return DSDisposeHandleImp? DSDisposeHandleImp(hndl): LV_ERR_bogusError;
}

size_t lv_interop::DSGetHandleSize(LV_UHandle_t hndl)
{
    return DSGetHandleSizeImp? DSGetHandleSizeImp(hndl): LV_ERR_bogusError;
}

LV_MgErr_t lv_interop::NumericArrayResize(int32_t type_code, int32_t n_dims, LV_UHandlePtr_t ptr, size_t size){
    return NumericArrayResizeImp? NumericArrayResizeImp(type_code, n_dims, ptr, size) : LV_ERR_bogusError;
}

LV_MgErr_t lv_interop::PostLVUserEvent(LV_UserEventRef_t ref, void* data){
    return PostLVUserEventImp ? PostLVUserEventImp(ref, data) : LV_ERR_bogusError;
}

#ifdef _WIN32
extern "C"
{
    ASE_EXPORT LV_MgErr_t ase_specify_lv_runtime_windows(const char * const path)
    {
        // set the path
        lv_runtime_path_windows = path;
        // try and load the library
        on_shared_library_load();
        // this will clear the lv_runtime_path_windows string if it fails
        return lv_runtime_path_windows.empty()? LV_ERR_bogusError : LV_ERR_noError;
    }
}
#endif