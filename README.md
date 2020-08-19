`libuvc` is a cross-platform library for USB video devices, built atop `libusb`.
It enables fine-grained control over USB video devices exporting the standard USB Video Class
(UVC) interface, enabling developers to write drivers for previously unsupported devices,
or just access UVC devices in a generic fashion.

This version of libuvc was forked from github.com/libuvc/libuvc and modified to add Windows
support.
* Added Windows support to CMake files. Now the only custom Find*.cmake file is FindLibUSB.cmake.
On Windows, you'll need to help CMake find libraries/includes by defining OpenCV_ROOT, LibUSB_ROOT,
DJPEG_ROOT and JPEG_LIBRARY.
* Replaced usage of pthreads and clock functions with C++ standards. In its current state, the
code is an ugly combination of C and C++ styles (malloc/free vs smart pointers, etc), but it does
work on Linux, Mac and Windows.


## Getting and Building libuvc

### Prerequisites

* [CMake](http://www.cmake.org/)
* libjpeg (or libjpeg-turbo)
* libusb

### Additional Linux Prerequisites

* [`libgtk2.0-dev`] if building example
* [OpenCV] if building example

### Additional Windows Prerequisites

* `libusb` >= 1.0.22
* `libusbK` driver installed for your USB Video Class camera(s). To do this, you can use [Zadig](https://zadig.akeo.ie/). If your camera is a **USB Composite Device**, you must install libusbK for the **USB Composite Device**.
  1. Run Zadig
  1. Select "Options" then check "List all devices"
  1. Select "Options" then uncheck "Ignore Hubs or Composite Parents"
  1. In the list, find the "Composite Parent". Make sure it has the VID/PID you expect. For the new driver, find libusbK in the combo box, then choose "Replace driver".
  1. When prompted something like "Are you sure you want to replace a system driver", select YES/OK.

### Build examples

Linux
```
git clone https://github.com/UniversalLaserSystems/libuvc
cd libuvc
mkdir build
cd build
cmake \
  -G "Unix Makefiles" \
  ..
cmake --build build --target install
```

Linux (with example and test binaries)
```
git clone https://github.com/UniversalLaserSystems/libuvc
cd libuvc
mkdir build
cd build
cmake \
  -G "Unix Makefiles" \
  -DOpenCV_ROOT="/path/to/opencv/install" \
  -DENABLE_UVC_DEBUGGING=ON \
  -DBUILD_EXAMPLE=ON \
  -DBUILD_TEST=ON \
  ..
cmake --build build --target install
```

Windows
```
git clone https://github.com/UniversalLaserSystems/libuvc
cd libuvc
mkdir build
cd build
cmake -G "Visual Studio 15 2017"^
  -DLibUSB_ROOT:PATH="path/to/libusb/install"^
  -DJPEG_ROOT:PATH="path/to/jpeg/install"^
  -DJPEG_LIBRARY:FILEPATH="path/to/jpeg/library"^
  -DCMAKE_CXX_STANDARD=17^
  ..
```

Windows (with example and test binaries)
```
git clone https://github.com/UniversalLaserSystems/libuvc
cd libuvc
mkdir build
cd build
cmake -G "Visual Studio 15 2017"^
  -DOpenCV_ROOT:PATH="path/to/opencv/install"^
  -DLibUSB_ROOT:PATH="path/to/libusb/install"^
  -DJPEG_ROOT:PATH="path/to/jpeg/install"^
  -DJPEG_LIBRARY:FILEPATH="path/to/jpeg/library"^
  -DCMAKE_CXX_STANDARD=17^
  -DENABLE_UVC_DEBUGGING=ON^
  -DBUILD_EXAMPLE=ON^
  -DBUILD_TEST=OFF^
  ..
```

If you want to change the build configuration, you can edit `CMakeCache.txt`
in the build directory, or use a CMake GUI to make the desired changes.

There is also `BUILD_EXAMPLE` and `BUILD_TEST` options to enable the compilation of `example` and `uvc_test` programs. To use them, replace the `cmake ..` command above with `cmake .. -DBUILD_TEST=ON -DBUILD_EXAMPLE=ON`.
Then you can start them with `./example` and `./uvc_test` respectively. Note that you need OpenCV to build the later (for displaying image).

## Developing with libuvc

The documentation for `libuvc` can currently be found at https://int80k.com/libuvc/doc/.

Happy hacking!
