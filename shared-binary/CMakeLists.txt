if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # set to use Multithreaded Static Runtime
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

  # vcpkg has some issues with not loading the static libs
  # so manually fix the MSVC runtime components used
  string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
  string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
endif()


# Specify Project
project(ase_shared_binary VERSION 1.0.0 LANGUAGES C CXX)

# include tooling to generate export header
include(GenerateExportHeader)

# export a json file with the compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# find boost
find_package(Boost REQUIRED COMPONENTS system)

# specify shared-library target
add_library(ase SHARED)

# add source
include(${CMAKE_CURRENT_SOURCE_DIR}/src/CMakeLists.txt)

# add compile definitions
target_compile_definitions(ase PUBLIC ase_BYTE_PACKING_${CMAKE_SIZEOF_VOID_P})

target_include_directories(ase
  PUBLIC
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ase>"
  # add binary directory so auto-generated ase_export.h is found
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
)

# plaform specific definitions
if(WIN32)
  # windows.h definitions
  target_compile_definitions(ase PUBLIC WIN32_LEAN_AND_MEAN NOMINMAX /wd4068)
  
elseif(UNIX)
  # link against static system libs
  target_link_options(ase PRIVATE
    "-static-libgcc" "-static-libstdc++"
  )

  # try and remove unused code sections
  target_compile_options(ase PRIVATE
    "-fdata-sections" "-ffunction-sections" "-Wl,--gc-sections"
  )

  # don't use "lib"-prefix on linux
  set_target_properties(ase PROPERTIES PREFIX "")
endif()

# specify target properties
# winrt only supports C++17 std
set_property(TARGET ase PROPERTY CXX_STANDARD 17)
set_target_properties(ase PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  C_VISIBILITY_PRESET hidden
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON
  WINDOWS_EXPORT_ALL_SYMBOLS OFF
)

# generate export header
generate_export_header(ase)

# Set the Library Filename to include the bitness
math(EXPR BITNESS "8*${CMAKE_SIZEOF_VOID_P}")
set_target_properties(ase PROPERTIES OUTPUT_NAME ase_${BITNESS}-${ase_shared_binary_VERSION_MAJOR})
# target_link_libraries(ase PRIVATE
  
# )

# Install Shared Library
INSTALL(TARGETS ase
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION bin
)