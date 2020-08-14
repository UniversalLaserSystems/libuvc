cmake_minimum_required(VERSION 3.1)
project(libuvc
  VERSION 0.0.6
  LANGUAGES CXX
)

# [----------------------------------------------------------------------------
include(CMakePrintHelpers)

# CMP0074: find_package uses <PackageName>_ROOT variables
if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
endif()
# ----------------------------------------------------------------------------]

# Additional search scripts path for libusb-1.0
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

# Xcode and Visual Studio do not using CMAKE_BUILD_TYPE cache variable
# so we use Release build type only with single configuration generators.
if (NOT CMAKE_CONFIGURATION_TYPES)
  if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
  endif()
endif()

if(NOT CMAKE_BUILD_TARGET)
    message(STATUS "No target type selected, default to both shared and static library")
    set(CMAKE_BUILD_TARGET "Both" CACHE STRING "" FORCE)
endif()

option(BUILD_EXAMPLE "Build example program" ON)
option(BUILD_TEST "Build test program" OFF)
option(ENABLE_UVC_DEBUGGING "Enable UVC debugging" OFF)

set(libuvc_DESCRIPTION "A cross-platform library for USB video devices")
set(libuvc_URL "https://github.com/libuvc/libuvc")

set(SOURCES 
  src/ctrl.cpp
  src/ctrl-gen.cpp
  src/device.cpp
  src/diag.cpp
  src/frame.cpp
  src/init.cpp
  src/stream.cpp
  src/misc.cpp
)

find_package(LibUSB)

find_package(JPEG QUIET)
if(JPEG_FOUND)
  message(STATUS "Building libuvc with JPEG support.")
  set(LIBUVC_HAS_JPEG TRUE)
  list(APPEND SOURCES src/frame-mjpeg.cpp)
else()
  message(WARNING "JPEG not found. libuvc will not support JPEG decoding.")
endif()

# [-----------------------------------------------------------------------------
cmake_print_properties(TARGETS LibUSB::LibUSB
                       PROPERTIES INTERFACE_INCLUDE_DIRECTORIES IMPORTED_LOCATION)
cmake_print_properties(TARGETS JPEG::JPEG
                       PROPERTIES INTERFACE_INCLUDE_DIRECTORIES IMPORTED_LOCATION IMPORTED_LOCATION_RELEASE IMPORTED_LOCATION_DEBUG)
# -----------------------------------------------------------------------------]

if(${CMAKE_BUILD_TARGET} MATCHES "Shared")
  set(BUILD_UVC_SHARED TRUE)
elseif(${CMAKE_BUILD_TARGET} MATCHES "Static")
  set(BUILD_UVC_STATIC TRUE)
elseif(${CMAKE_BUILD_TARGET} MATCHES "Both")
  set(BUILD_UVC_SHARED TRUE)
  set(BUILD_UVC_STATIC TRUE)
else()
  message(FATAL_ERROR "Invalid build type ${CMAKE_BUILD_TARGET}" )
endif()

if(BUILD_UVC_SHARED)
  add_library(uvc SHARED ${SOURCES})
  set_target_properties(uvc PROPERTIES
    VERSION ${libuvc_VERSION}
    SOVERSION ${libuvc_VERSION_MAJOR}
    # Exported name of target within namespace LibUVC
    EXPORT_NAME UVCShared
    LINKER_LANGUAGE CXX
  )
  list(APPEND UVC_TARGETS uvc)
  if(NOT LibUVC_STATIC)  
    add_library(LibUVC::UVC ALIAS uvc)
  endif()
  # Aliases defined here available only if project included 
  # via addsubdirectory
  add_library(LibUVC::UVCShared ALIAS uvc)
endif()

if(BUILD_UVC_STATIC)
  add_library(uvc_static STATIC ${SOURCES})
  set_target_properties(uvc_static PROPERTIES
    OUTPUT_NAME uvc
    # Exported name of target within namespace LibUVC
    EXPORT_NAME UVCStatic
    LINKER_LANGUAGE CXX
  )
  list(APPEND UVC_TARGETS uvc_static)
  add_library(LibUVC::UVCStatic ALIAS uvc_static)
  if(LibUVC_STATIC)
    add_library(LibUVC::UVC ALIAS uvc_static)
  endif()
endif()

configure_file(include/libuvc/libuvc_config.h.in
  include/libuvc/libuvc_config.h
  @ONLY
)

foreach(target_name IN LISTS UVC_TARGETS)
  target_include_directories(${target_name}
    PUBLIC
      # Different paths for includes for build and install phase supported 
      # via INSTALL_INTERFACE and BUILD_INTERFACE generator expressions.
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<INSTALL_INTERFACE:include>
  )
  target_link_libraries(${target_name}
    # libusb-1.0 used internally so we link to it privately.
    PRIVATE LibUSB::LibUSB
  )
  if(JPEG_FOUND)
    target_link_libraries(${target_name}
      PRIVATE JPEG::JPEG
    )
  endif()
  set_target_properties(${target_name} PROPERTIES
      PUBLIC_HEADER "include/libuvc/libuvc.h;${CMAKE_CURRENT_BINARY_DIR}/include/libuvc/libuvc_config.h"
  )
  if(ENABLE_UVC_DEBUGGING)
    target_compile_definitions(${target_name}
      PRIVATE
        UVC_DEBUGGING
    )
  endif()
endforeach()

if(BUILD_EXAMPLE)
  add_executable(example src/example.cpp)
  if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_target_properties(example PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB:MSVCRT /NODEFAULTLIB:LIBCMT")
  endif()
  find_package(Threads)
  find_package(OpenCV REQUIRED
      core
      highgui
    PATHS
      ${OpenCV_ROOT}
  )
  target_link_libraries(example
    PRIVATE
      uvc_static
      LibUSB::LibUSB
      Threads::Threads
      opencv_core
      opencv_highgui
  )
endif()

if(BUILD_TEST)
  # OpenCV defines targets with transitive dependencies not with namespaces but using opencv_ prefix. 
  # This targets provide necessary include directories and linked flags.
  find_package(OpenCV REQUIRED
    COMPONENTS
      core
      highgui
    PATHS
      ${OpenCV_ROOT}
  )

  add_executable(uvc_test
    src/test.cpp
    $<$<BOOL:${JPEG_FOUND}>:src/frame-mjpeg.cpp>
  )
  if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_target_properties(uvc_test PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB:MSVCRT /NODEFAULTLIB:LIBCMT")
  endif()
  target_link_libraries(uvc_test
    PRIVATE
      LibUVC::UVC
      opencv_core
      opencv_highgui
  )
endif()


include(GNUInstallDirs)
set(CMAKE_INSTALL_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/libuvc)

install(
  TARGETS ${UVC_TARGETS}
  EXPORT libuvcTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libuvc
)

install(EXPORT libuvcTargets
  FILE libuvcTargets.cmake
  NAMESPACE LibUVC::
  DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
)

install(FILES 
    cmake/FindLibUSB.cmake
  DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(libuvcConfigVersion.cmake
  COMPATIBILITY AnyNewerVersion
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/libuvcConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
)

if(JPEG_FOUND)
  # If we have used JPEG library we need to 
  # add linker flag for it in config file for pkgconfig
  set(PKGCONFIG_JPEG_LDFLAG "-ljpeg")
endif()

configure_file(libuvc.pc.in
  ${PROJECT_BINARY_DIR}/libuvc.pc
  @ONLY
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/libuvc.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

install(FILES libuvcConfig.cmake
  DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
)


if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  string(REGEX REPLACE "[.]lib" ".dll" LibUSB_DLL ${LibUSB_LIBRARY})
  cmake_print_variables(LibUSB_LIBRARY LibUSB_DLL CMAKE_BINARY_DIR)
  add_custom_command(TARGET example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${LibUSB_LIBRARY} ${CMAKE_BINARY_DIR}/$<CONFIG>
    COMMAND ${CMAKE_COMMAND} -E copy ${LibUSB_DLL}     ${CMAKE_BINARY_DIR}/$<CONFIG>
    )
  install (
    FILES ${LibUSB_LIBRARY} ${LibUSB_DLL}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()
