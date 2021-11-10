//          Copyright serenial.io and contributors.
// Distributed under the Boost Software License, Version 1.0.
//    (See https://www.boost.org/LICENSE_1_0.txt)

#ifndef ASYNCHRONOUS_SYSTEM_EXEC_TYPES
#define ASYNCHRONOUS_SYSTEM_EXEC_TYPES

#ifdef _WIN32
#ifdef ASE_EXPORTS
#define ASE_API __declspec(dllexport)
#else
#define ASE_API __declspec(dllimport)
#endif
#else
#define ASE_API
#endif

// LabVIEW Includes
#include <extcode.h>
#include <platdefines.h>

// ##############################################################################
// ##                                                                          ##
// ##                            Type Definitions                              ##
// ##                                                                          ##
// ##############################################################################

// LV NumericArrayResize Type Codes
#define LV_U8_TYPECODE uB
#define LV_U32_TYPECODE uL
#define LV_U64_TYPECODE uQ

// lv_prolog.h configures compiler dependend features
#include <lv_prolog.h>

// define a typecode that depends on the bitness of the platform to indicate the pointer size
// this is used for the typeCode when using NumericArrayResize on an array of pointers
#if IsOpSystem64Bit
#define LVuPtrCode LV_U64_TYPECODE
#else
#define LVuPtrCode LV_U32_TYPECODE
#endif

// declare any LabVIEW types

// LabVIEW Array
template <unsigned ndims, typename datatype>
struct LVArray
{
    unsigned dims[ndims];
    datatype data[1];
};

// LabVIEW Error Cluster type
typedef struct
{
    LVBoolean status;
    int32 code;
    LStrHandle source;
} LVErrorCluster;

typedef struct
{
    LStrHandle data;
    LStrHandle id;
} outEventData;

typedef struct
{
    int32 exitCode;
    LStrHandle id;
} didExitEventData;

typedef struct
{
    LStrHandle stdOutRegex;
    LStrHandle stdErrRegex;
} StreamSplitRegex;

typedef struct
{
    LVUserEventRef stdOutEventRef;
    LVUserEventRef stdErrorEventRef;
    LVUserEventRef didExitEventRef;
} StartCallEventRefs;

typedef struct{
    LVUserEventRef didTimeoutEventRef;
} WaitOnCallEventRefs;

typedef struct
{
    int32 dimSize;
    Path elt[1];
} SearchPaths;
typedef SearchPaths **SearchPathsHandle;

typedef struct
{
    LVUserEventRef ref;
} UE;

#include <lv_epilog.h>

// ##############################################################################
// ##                                                                          ##
// ##                              Errors Codes                                ##
// ##                                                                          ##
// ##############################################################################

// Error Codes
// LabVIEW Memory Manager Errs in range 1->3 so skip that range

// No Error
#define ERR_NO_ERROR noErr

// Bad Call Library Node Configuration/Parameters
#define ERR_INVALID_ERROR_PTR -100
#define ERR_INVALID_NON_ERROR_PTR -101

// General Errors
#define ERR_UNKNOWN_ERROR -200
#define ERR_STD_EXCEPTION -201

// Defined Errors
#define ERR_BAD_REGEX_EXPRESSION -300
#define ERR_UNABLE_TO_LAUNCH_EXE -301

#endif