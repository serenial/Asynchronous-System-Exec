
include(${CMAKE_CURRENT_LIST_DIR}/lv_interop/CMakeLists.txt)

target_sources(ase PUBLIC
  FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
  FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/include/ase/event_handler.hpp
)

target_sources(ase PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/asynchronous_system_exec.cpp
    ${CMAKE_CURRENT_LIST_DIR}/event_handler.cpp
)