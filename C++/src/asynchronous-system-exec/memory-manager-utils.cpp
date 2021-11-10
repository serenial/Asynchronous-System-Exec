//          Copyright serenial.io and contributors.
// Distributed under the Boost Software License, Version 1.0.
//    (See https://www.boost.org/LICENSE_1_0.txt)

#include "./memory-manager-utils.hpp"

// LabVIEW Memory Manager error thrower
void throw_any_lv_memory_manager_errors(MgErr err)
{
    if (err)
        throw LVMemoryManagerError(err);
}

// ##############################################################################
// ##                                                                          ##
// ##                              C++ to LabVIEW Types                        ##
// ##                                                                          ##
// ##############################################################################

void lv_string_write_to_handle_ptr(LStrHandle *lvStringHandlePtr, std::string outputString)
{

    if (!lvStringHandlePtr)
    {
        throw_any_lv_memory_manager_errors(mZoneErr);
    }

    // resize string handle
    throw_any_lv_memory_manager_errors(NumericArrayResize(LV_U8_TYPECODE, 1, (UHandle *)lvStringHandlePtr, outputString.length()));

    // Write outputString into lvStringHandle Buffer
    MoveBlock(outputString.c_str(), LHStrBuf(*lvStringHandlePtr), outputString.length());

    // Update lvStringHandle Length
    LStrLen(**lvStringHandlePtr) = outputString.length();
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

// ##############################################################################
// ##                                                                          ##
// ##                              LabVIEW Types to C++                        ##
// ##                                                                          ##
// ##############################################################################

// get std::string from lStrHandle
std::string lv_string_handle_to_string(LStrHandle handle)
{
    return std::string(reinterpret_cast<char *> LHStrBuf(handle), LHStrLen(handle));
}

// ##############################################################################
// ##                                                                          ##
// ##                                 Utility                                  ##
// ##                                                                          ##
// ##############################################################################

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