cmake_minimum_required(VERSION 3.19)

# configure vcpkg

# use custom triplet on windows - try and detect using variables avaliable
# before project is defined so VCPKG variables can be applied

if(WIN32 AND NOT DEFINED VCPKG_TARGET_TRIPLET)
  if(CMAKE_GENERATOR_PLATFORM STREQUAL "win32")
    set(VCPKG_TARGET_TRIPLET x86-windows-static)
  else()
  set(VCPKG_TARGET_TRIPLET x64-windows-static)
  endif()
endif()

set(VERSION_NAME "2.0.0")

if(NOT DEFINED INSTALL_DIR_LIBRARY)
  set(INSTALL_DIR_LIBRARY "${CMAKE_CURRENT_LIST_FILE}/../../LabVIEW/serenial.io-ase/ase")
endif()

if(NOT DEFINED INSTALL_DIR_TEST_BINARIES)
  set(INSTALL_DIR_TEST_BINARIES "${CMAKE_CURRENT_LIST_FILE}/../../LabVIEW/test-binaries")
endif()

if(UNIX)
    # set this flag here so it is applied during vcpkg install
    # required to produce boost libs with correct linkage characteristics
    # bundle static libs for better cross platform compatibility
    set(VCPKG_CXX_FLAGS "--fPIC")
endif()

project (AsyncSystemExec VERSION ${VERSION_NAME})

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}")

# determine the bitness
math(EXPR BITS "8*${CMAKE_SIZEOF_VOID_P}")

if(NOT DEFINED APPEND_TO_NAME)
  set(APPEND_TO_NAME ".${BITS}")
endif()

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # set to use Multithreaded Static Runtime
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

  # vcpkg has some issues with not loading the static libs
  # add tweak compiler flags and use -DVCPKG_TARGET_TRIPLET=x64-windows-static \
  string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
  string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})

  # add additional general MSVC directives
  add_definitions("-D_CRT_SECURE_NO_WARNINGS" "-D_WIN32_WINNT=0x0601")
else()
  # // some other compiler
endif()

# find boost
find_package(Boost REQUIRED COMPONENTS date_time filesystem regex system)

# add the shared library
add_library(shared-library-build SHARED 
                "asynchronous-system-exec.cpp" 
                "common/lv-interop.cpp" 
                "handler-exception-list.cpp" 
)

# Use C++ Standard 11 
set_property(TARGET shared-library-build PROPERTY CXX_STANDARD 14)
set_property(TARGET shared-library-build PROPERTY CXX_STANDARD_REQUIRED ON)

# define output file name
set_target_properties(shared-library-build PROPERTIES PREFIX "")
set_target_properties(shared-library-build PROPERTIES OUTPUT_NAME "serenial.io-ase${APPEND_TO_NAME}")

add_compile_definitions(ASE_VERSION=\"${VERSION_NAME}\")
add_compile_definitions(BYTE_PACKING_${CMAKE_SIZEOF_VOID_P})

# add boost Libs
target_link_libraries(shared-library-build PRIVATE Boost::boost Boost::system Boost::filesystem Boost::regex Boost::date_time)

# platform specific setup
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message("Platform: Windows (MSVC)")
    add_compile_definitions("ASE_EXPORTS")
    target_link_options(shared-library-build PRIVATE "/SAFESEH:NO")

elseif( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
    message("Platform: Linux (GCC)")
     # Try and avoid missing dependencies by bundling in required libs statically
else()
    message(FATAL_ERROR "Unknown Platform")
endif()

message("Install Directory ${INSTALL_DIR_LIBRARY}")

# configure install
install(TARGETS shared-library-build DESTINATION ${INSTALL_DIR_LIBRARY})


# create event test application
add_executable(EchoTester echo-tester.cpp)
set_target_properties(EchoTester PROPERTIES OUTPUT_NAME "ase-test-console-app-${BITS}")

install(TARGETS EchoTester DESTINATION ${INSTALL_DIR_TEST_BINARIES})