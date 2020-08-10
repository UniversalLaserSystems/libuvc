setlocal
mkdir build
cd build
cmake -G "Visual Studio 15 2017"^
  -Wno-dev^
  -DOpenCV_ROOT:PATH="D:/working_copies/LSM/lsm-10/third-party/install"^
  -DLibUSB_ROOT:PATH="D:/working_copies/LSM/lsm-10/third-party/install"^
  -DJPEG_INCLUDE_DIRS:PATH="D:/working_copies/LSM/lsm-10/third-party/install"^
  -DJPEG_LIBRARY_RELEASE:FILEPATH="D:/working_copies/LSM/lsm-10/third-party/install/lib/jpeg-static.lib"^
  -DJPEG_LIBRARY_DEBUG:FILEPATH="D:/working_copies/LSM/lsm-10/third-party/install/lib/jpeg-staticd.lib"^
  -DCMAKE_CXX_STANDARD=17^
  -DBUILD_SHARED_LIBS=OFF^
  -DENABLE_UVC_DEBUGGING=ON^
  -DBUILD_EXAMPLE=ON^
  -DBUILD_TEST=OFF^
  ..
endlocal
