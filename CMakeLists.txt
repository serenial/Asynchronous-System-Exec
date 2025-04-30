cmake_minimum_required(VERSION 3.27)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

# SET the toolchain file to the one in the VCPKG git-submodule
SET(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(ase)

# add source
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/shared-binary)