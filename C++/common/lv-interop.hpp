//          Copyright serenial.io and contributors.
// Distributed under the Boost Software License, Version 1.0.
//    (See https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string>
#include <type_traits>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "./types.hpp"

// dynamically loaded functions imported from LabVIEW executable/runtime
void load_lv_functions();

MgErr NumericArrayResize(int32_t, int32_t, void*, size_t);

MgErr PostLVUserEvent(LVUserEventRef, void*);

MgErr DSDisposeHandle(void *handle);

// utility functions for LV -> C++ and C++ -> LV types
void throw_any_lv_memory_manager_errors(MgErr err);

void lv_string_write_to_handle_ptr(LStrHandle *, std::string);

int lv_error_cluster_write_err(LVErrorCluster *, int, std::string, std::string);

std::string lv_string_handle_to_string(LStrHandle);

int lv_error_cluster_write_null_ptr_err(LVErrorCluster *, std::string);

int lv_error_cluster_write_std_exception(LVErrorCluster *, std::string, std::string);

int lv_error_cluster_write_unknown_err(LVErrorCluster *, std::string);

