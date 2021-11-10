//          Copyright serenial.io and contributors.
// Distributed under the Boost Software License, Version 1.0.
//    (See https://www.boost.org/LICENSE_1_0.txt)

#ifndef ASYNCHRONOUS_SYSTEM_EXEC_MEMORY_MANAGER_UTILS
#define ASYNCHRONOUS_SYSTEM_EXEC_MEMORY_MANAGER_UTILS

#include <string>

#include "./types.hpp"

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

// LabVIEW Memory Manager error thrower
void throw_any_lv_memory_manager_errors(MgErr err);

// ##############################################################################
// ##                                                                          ##
// ##                              C++ to LabVIEW Types                        ##
// ##                                                                          ##
// ##############################################################################

void lv_string_write_to_handle_ptr(LStrHandle *, std::string);

int lv_error_cluster_write_err(LVErrorCluster *, int, std::string, std::string);

// ##############################################################################
// ##                                                                          ##
// ##                              LabVIEW Types to C++                        ##
// ##                                                                          ##
// ##############################################################################

std::string lv_string_handle_to_string(LStrHandle);

// ##############################################################################
// ##                                                                          ##
// ##                                 Utility                                  ##
// ##                                                                          ##
// ##############################################################################

int lv_error_cluster_write_null_ptr_err(LVErrorCluster *, std::string);

int lv_error_cluster_write_std_exception(LVErrorCluster *, std::string, std::string);

int lv_error_cluster_write_unknown_err(LVErrorCluster *, std::string);

#endif