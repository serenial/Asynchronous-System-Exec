//          Copyright serenial.io and contributors.
// Distributed under the Boost Software License, Version 1.0.
//    (See https://www.boost.org/LICENSE_1_0.txt)

#include <cstring>
#include "./lv-interop.hpp"

// define function pointer types
using NumericArrayResizePtr = std::add_pointer<MgErr(int32_t, int32_t, void *handle, size_t size)>::type;
using DSDisposeHandlePtr = std::add_pointer<MgErr(void* handle)>::type;
using PostLVUserEventPtr = std::add_pointer<MgErr (LVUserEventRef, void*)>::type;

// create static function pointers to store function implementations
static NumericArrayResizePtr NumericArrayResizeImp = nullptr;
static DSDisposeHandlePtr DSDisposeHandleImp = nullptr;
static PostLVUserEventPtr PostLVUserEventImp = nullptr;

#ifdef _WIN32

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void load_lv_functions()
{
    if (NumericArrayResizeImp != nullptr)
    {
        return;
    }

    auto lvModule = GetModuleHandle("LabVIEW.exe");
    if (lvModule == nullptr)
    {
        lvModule = GetModuleHandle("lvffrt.dll");
    }
    if (lvModule == nullptr)
    {
        lvModule = GetModuleHandle("lvrt.dll");
    }
    NumericArrayResizeImp = (NumericArrayResizePtr)GetProcAddress(lvModule, "NumericArrayResize");
    DSDisposeHandleImp = (DSDisposeHandlePtr)GetProcAddress(lvModule, "DSDisposeHandle");
    PostLVUserEventImp = (PostLVUserEventPtr)GetProcAddress(lvModule,"PostLVUserEvent");
}

#else

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void load_lv_functions()
{
    if (NumericArrayResizeImp != nullptr)
    {
        return;
    }

    auto lvModule = dlopen(nullptr, RTLD_LAZY);
    if (lvModule != nullptr)
    {
        NumericArrayResizeImp = (NumericArrayResizePtr)dlsym(lvModule, "NumericArrayResize");
        DSDisposeHandleImp = (DSDisposeHandlePtr)dlsym(lvModule, "DSDisposeHandle");
        PostLVUserEventImp = (PostLVUserEventPtr)dlsym(lvModule, "PostLVUserEvent");
       
    }
    if (NumericArrayResizeImp == nullptr)
    {
        lvModule = dlopen("liblvrt.so", RTLD_NOW);
        NumericArrayResizeImp = (NumericArrayResizePtr)dlsym(lvModule, "NumericArrayResize");
        DSDisposeHandleImp = (DSDisposeHandlePtr)dlsym(lvModule, "DSDisposeHandle");
        PostLVUserEventImp = (PostLVUserEventPtr)dlsym(lvModule, "PostLVUserEvent");
    }
}

#endif

MgErr NumericArrayResize(int32_t typeCode, int32_t numDims, void *handle, size_t size)
{
    return NumericArrayResizeImp(typeCode, numDims, handle, size);
}

MgErr DSDisposeHandle(void *handle)
{
    return DSDisposeHandleImp(handle);
}

MgErr PostLVUserEvent(LVUserEventRef ref, void* data){
    return PostLVUserEventImp(ref, data);
}

// utility functions for LV -> C++ and C++ -> LV types

void throw_any_lv_memory_manager_errors(MgErr err)
{
    if (err)
        throw LVMemoryManagerError(err);
}

void lv_string_write_to_handle_ptr(LStrHandle *lvStringHandlePtr, std::string outputString)
{

    if (!lvStringHandlePtr)
    {
        throw_any_lv_memory_manager_errors(mZoneErr);
    }

    // resize string handle
    load_lv_functions();
    throw_any_lv_memory_manager_errors(NumericArrayResize(LV_U8_TYPECODE, 1, lvStringHandlePtr, outputString.length()));

    // Write outputString into lvStringHandle Buffer
    std::memcpy((**lvStringHandlePtr)->str, outputString.c_str(), outputString.length());

    // Update lvStringHandle Length
    (**lvStringHandlePtr)->cnt = outputString.length();
}

int lv_error_cluster_write_err(LVErrorCluster *errorPtr, int code, std::string functionName, std::string message)
{

    // set status and code
    errorPtr->status = code !=0? LVBooleanTrue : LVBooleanFalse;
    errorPtr->code = code;

    //build outputString
    std::string outputString = "ASE Shared Library Function: " + functionName + "()" + (message.empty() ? "" : ("\n<ERR>" + message));

    try
    {
        // write outputString to error source string handle
        lv_string_write_to_handle_ptr(&(errorPtr->source), outputString);
    }
    catch (LVMemoryManagerError &e)
    {
        return e.err;
    }
    catch (...)
    {
        return ERR_UNKNOWN_ERROR;
    }

    return ERR_NO_ERROR;
}

// get std::string from lStrHandle
std::string lv_string_handle_to_string(LStrHandle handle)
{
    size_t length = handle && (*handle) ? (*handle)->cnt : 0;
    return length>0 ? std::string(reinterpret_cast<char *>((*handle)->str), length) : "";
}


int lv_error_cluster_write_null_ptr_err(LVErrorCluster *errorPtr, std::string functionName)
{
    return lv_error_cluster_write_err(errorPtr, ERR_INVALID_NON_ERROR_PTR, functionName, "A pointer or handle supplied is not valid.");
}

int lv_error_cluster_write_std_exception(LVErrorCluster *errorPtr, std::string functionName, std::string what)
{
    return lv_error_cluster_write_err(errorPtr, ERR_STD_EXCEPTION, functionName, what);
}

int lv_error_cluster_write_unknown_err(LVErrorCluster *errorPtr, std::string functionName)
{
    return lv_error_cluster_write_err(errorPtr, ERR_UNKNOWN_ERROR, functionName, "Unknown internal exception occurred.");
}