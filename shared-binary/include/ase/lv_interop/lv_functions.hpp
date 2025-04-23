#pragma once

#ifdef _WIN32
// include windows.h
#include <windows.h>
// include windows media foundation headers whilst we are here
#include <mfapi.h>
#else
#include <dlfcn.h>
#endif

#include "./lv_types.hpp"

namespace ase
{
    namespace lv_interop
    {
        LV_MgErr_t DSDisposeHandle(LV_UHandle_t);
        LV_MgErr_t DSCheckHandle(LV_UHandle_t);
        LV_UHandle_t DSNewHClr(size_t);
        LV_MgErr_t DSSetHSzClr(LV_UHandle_t, size_t);
        size_t DSGetHandleSize(LV_UHandle_t);
        LV_MgErr_t PostLVUserEvent(LV_UserEventRef_t, void*);

        // https://www.ni.com/docs/en-US/bundle/labview-api-ref/page/properties-and-methods/lv-manager/dsdisposehandle.html
        // MgErr DSDisposeHandle(h)
        using LV_DSDisposeHandleFnPtr_t = std::add_pointer_t<LV_MgErr_t(LV_UHandle_t)>;

        // https://www.ni.com/docs/en-US/bundle/labview-api-ref/page/properties-and-methods/lv-manager/dscheckhandle.html
        // MgErr DSCheckHandle(h);
        using LV_DSCheckHandlePtr_t = std::add_pointer_t<LV_MgErr_t(LV_UHandle_t)>;

        // https://www.ni.com/docs/en-US/bundle/labview-api-ref/page/properties-and-methods/lv-manager/DSNewHClr.html
        // UHandle DSNewHClr(size);
        using LV_DSNewHClrPtr_t = std::add_pointer_t<LV_UHandle_t(size_t)>;

        // https://www.ni.com/docs/en-US/bundle/labview-api-ref/page/properties-and-methods/lv-manager/DSSetHSzClr.html
        // MgErr DSSetHSzClr(h, size);
        using LV_DSSetHSzClrPtr_t = std::add_pointer_t<LV_MgErr_t(LV_UHandle_t, size_t)>;

        // https://www.ni.com/docs/en-US/bundle/labview-api-ref/page/properties-and-methods/lv-manager/DSGetHandleSize.html
        // size_t DSGetHandleSize(h);
        using LV_DSGetHandleSizePtr_t = std::add_pointer_t<size_t(LV_UHandle_t)>;

        // https://www.ni.com/docs/en-US/bundle/labview-api-ref/page/properties-and-methods/lv-manager/postlvuserevent.html
        // MgErr PostLVUserEvent(LVUserEventRef ref, void *data);
        using LV_PostLVUserEventPtr_t = std::add_pointer_t<LV_MgErr_t(LV_UserEventRef_t, void *)>;

        // custom exceptions
        struct LV_MemoryManagerException : public std::exception
        {
            const LV_MgErr_t err;
            LV_MemoryManagerException(LV_MgErr_t err) : err(err){};
            const char *what() const throw()
            {
                switch (err)
                {
                case LV_ERR_mFullErr:
                    return "An \"mFullErr\" LabVIEW Memory Manager error has occurred.";
                default:
                    return "An unspecified LabVIEW Memory Manager error has occurred.";
                }
            };
        };
    }

    void on_shared_library_load();
    void on_shared_library_unload();
}