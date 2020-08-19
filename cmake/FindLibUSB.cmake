#[==============================================[
FindLibUSB
-----------

Searching libusb-1.0 library and creating imported 
target LibUSB::LibUSB

LibUSB_INCLUDE_DIRS
LibUSB_LIBRARY

LibUSB_ROOT

#]==============================================]

# TODO Append parts for Version compasion and REQUIRED support

if (NOT TARGET LibUSB::LibUSB)
  find_package(PkgConfig)
  if (PKG_CONFIG_FOUND)
    pkg_check_modules(LibUSB REQUIRED
      libusb-1.0
    )
  endif()

  if(LibUSB_FOUND)
    # LibUSB_LIBRARIES and LibUSB_INCLUDEDIR are set by pkg_check_modules
    # From those, set LibUSB_LIBRARY and LibUSB_INCLUDE_DIRS which will be
    # set as target properties below.
    message(STATUS "libusb-1.0 found using pkgconfig")
    set(LibUSB_INCLUDE_DIRS ${LibUSB_INCLUDEDIR})
    find_library(LibUSB_LIBRARY
      NAMES ${LibUSB_LIBRARIES}
      PATHS ${LibUSB_LIBDIR}
      )
  else()
    find_path(LibUSB_INCLUDE_DIRS libusb-1.0/libusb.h)
    find_library(LibUSB_LIBRARY
      NAMES usb-1.0 libusb-1.0
      )
  endif()

  find_package_handle_standard_args(LibUSB DEFAULT_MSG LibUSB_LIBRARY LibUSB_INCLUDE_DIRS)

  add_library(LibUSB::LibUSB
    UNKNOWN IMPORTED
    )
  set_target_properties(LibUSB::LibUSB PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${LibUSB_INCLUDE_DIRS}
    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
    IMPORTED_LOCATION ${LibUSB_LIBRARY}
    )

  # TODO: mark_as_advanced()?
endif()
