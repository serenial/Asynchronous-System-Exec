//          Copyright serenial.io and contributors.
// Distributed under the Boost Software License, Version 1.0.
//    (See https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#ifdef _WIN32
#ifdef ASE_EXPORTS
#define ASE_API __declspec(dllexport)
#else
#define ASE_API __declspec(dllimport)
#endif
#else
#define ASE_API
#endif

// ##############################################################################
// ##                                                                          ##
// ##                            Type Definitions                              ##
// ##                                                                          ##
// ##############################################################################
#ifdef BYTE_PACKING_4
#pragma pack(push, 1)
#endif

#define LV_U8_TYPECODE 5

// declare any LabVIEW types
typedef uint32_t LVMagicCookie;

typedef LVMagicCookie LVUserEventRef ;

typedef uint8_t LVBoolean;

#define LVBooleanTrue static_cast<LVBoolean>(1)
#define LVBooleanFalse static_cast<LVBoolean>(0)

typedef int MgErr;

typedef struct {
    int32_t cnt;
    uint8_t str[1];
} LStr, *LStrPtr, **LStrHandle;

// LabVIEW Array
template <unsigned ndims, typename datatype>
struct LVArray_t
{
    unsigned dims[ndims];
    uint8_t buffer[1];

    datatype* data(size_t byteOffset=0)
        {
    #ifndef BYTE_PACKING_4
            if (sizeof(datatype) < 8)
            {
                return reinterpret_cast<datatype*>(buffer + byteOffset);
            }
            return reinterpret_cast<datatype*>(buffer + 4 + byteOffset); // 8-byte aligned data
    #else
            datatype* p = reinterpret_cast<datatype*>(buffer + byteOffset);
            return p;
    #endif
        };
};

// LabVIEW Error Cluster type
typedef struct
{
    LVBoolean status;
    int32_t code;
    LStrHandle source;
} LVErrorCluster, *LVErrorClusterPtr;

typedef LVBoolean* LVBooleanPtr;

typedef struct
{
    LStrHandle data;
    LStrHandle id;
} outEventData;

typedef struct
{
    int32_t exitCode;
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
    LVUserEventRef ref;
} UE;

#ifdef BYTE_PACKING_4
#pragma pack(pop)
#endif

// ##############################################################################
// ##                                                                          ##
// ##                              Errors Codes                                ##
// ##                                                                          ##
// ##############################################################################

// Error Codes
// LabVIEW Memory Manager Errs in range 1->3 so skip that range
#define noErr 0
#define mZoneErr 1
#define mFullErr 2

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

// Custom Exceptions
struct LVMemoryManagerError : public std::exception
{
    const MgErr err;
    LVMemoryManagerError(MgErr err) : err(err){};
    const char *what() const throw()
    {
        switch (err)
        {
        case mZoneErr:
            return "An \"mZoneErr\" LabVIEW Memory Manager error has occurred.";
        case mFullErr:
            return "An \"mFullErr\" LabVIEW Memory Manager error has occurred.";
        default:
            return "An unspecified LabVIEW Memory Manager error has occurred.";
        }
    };
};