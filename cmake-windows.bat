setlocal
mkdir build
cd build
cmake -G "Visual Studio 15 2017"^
  -Wno-dev^
  -DOpenCV_ROOT:PATH="D:/working_copies/LSM/lsm-10/third-party/install"^
  -DLibUSB_ROOT:PATH="C:/local/libusb-1.0.23"^
  -DJPEG_INCLUDE_DIR:PATH="C:/local/libjpeg-turbo/include"^
  -DJPEG_LIBRARY:FILEPATH="C:/local/libjpeg-turbo/lib/libjpeg-turbod.lib"^
  -DCMAKE_CXX_STANDARD=17^
  -DBUILD_SHARED_LIBS=OFF^
  -DENABLE_UVC_DEBUGGING=ON^
  -DBUILD_EXAMPLE=ON^
  -DBUILD_TEST=OFF^
  ..
endlocal
