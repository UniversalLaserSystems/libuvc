// Example code taken from https://ken.tossell.net/libuvc/doc/ and slightly modified.
//
// Also did these steps:
//
//   * sudo apt install libgtk2.0-dev
//
// ./cmake-linux.sh
// cd build_debug
// make
//
// Plugin camera
//  1. Use lsusb to find your camera bus and device number
//  2. sudo chmod 666 /dev/bus/<bus>/<device>       # so libusb can access device
//  3. ./example


#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <libuvc/libuvc.h>
#pragma warning (disable:4200)
#include <libusb-1.0/libusb.h>
#pragma warning (default:4200)
#include <opencv2/opencv.hpp>
#include <opencv2/videoio/registry.hpp>
#include <opencv2/core/types_c.h>
#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/core.hpp>
#include <stdio.h>
#include <chrono>
#include <sstream>
#include <thread>

#include <cstdlib>


using namespace std;


class Camera
{
public:
  Camera(
    int vid,
    int pid,
    int width,
    int height,
    int fps,
    uvc_context_t *ctx);

  ~Camera();

  void OpenDevice();
  void CloseDevice();
  void StartStreaming();
private:
  static void LibuvcCallback(uvc_frame_t *frame, void *ptr);

  int vid_;
  int pid_;
  int width_;
  int height_;
  int fps_;
  uvc_context_t *ctx_;
  uvc_device_t *dev_;
  uvc_device_handle_t *devh_;
  uvc_stream_ctrl_t ctrl_;
  int frameCounter_;
};

Camera::Camera(
  int vid,
  int pid,
  int width,
  int height,
  int fps,
  uvc_context *ctx)
  : vid_(vid)
  , pid_(pid)
  , width_(width)
  , height_(height)
  , fps_(fps)
  , ctx_(ctx)
  , dev_(nullptr)
  , devh_(nullptr)
  , frameCounter_(0)
{
}

Camera::~Camera()
{
  CloseDevice();
}

void Camera::OpenDevice()
{
  printf("Opening camera device vid:0x%04x pid:0x%04x\n", vid_, pid_);

  if (dev_ != nullptr)
  {
    throw std::runtime_error("camera device already open");
  }
  if (devh_ != nullptr)
  {
    throw std::runtime_error("device handle already open");
  }
    
  printf("uvc_find_device\n");
  auto res = uvc_find_device(
    ctx_, &dev_,
    vid_, pid_, NULL); /* filter devices: vendor_id, product_id, "serial_num" */
  if (res < 0)
  {
    throw std::runtime_error(uvc_strerror(res));
  }
    
  printf("uvc_open\n");
  res = uvc_open(dev_, &devh_);
  if (res < 0)
  {
    throw std::runtime_error(uvc_strerror(res));
  }

  uvc_print_diag(devh_, stderr);

  printf("uvc_get_stream_ctrl_format_size %dx%d @ %d FPS\n", width_, height_, fps_);
  res = uvc_get_stream_ctrl_format_size(
    devh_, &ctrl_, /* result stored in ctrl */
    UVC_FRAME_FORMAT_MJPEG,
    width_, height_, fps_);
  if (res < 0)
  {
    printf("Resolution %dx%d @ %d FPS is not a valid configuration\n", width_, height_, fps_);
    throw std::runtime_error("invalid resolution and frame rate");
  }

  uvc_print_stream_ctrl(&ctrl_, stderr);
}

void Camera::CloseDevice()
{
  if (devh_ != nullptr)
  {
    printf("uvc_stop_streaming\n");
    uvc_stop_streaming(devh_);
    printf("uvc_close\n");
    uvc_close(devh_);
    devh_ = nullptr;
  }
  if (dev_ != nullptr)
  {
    printf("uvc_unref_device\n");
    uvc_unref_device(dev_);
    dev_ = nullptr;
  }
}

void Camera::StartStreaming()
{
  printf("uvc_start_streaming\n");
  fflush(stdout);
  auto res = uvc_start_streaming(devh_, &ctrl_, &Camera::LibuvcCallback, (void*)this, 0);
  if (res < 0)
  {
    throw std::runtime_error(uvc_strerror(res));
  }
}


/* This callback function runs once per frame. Use it to perform any
 * quick processing you need, or have it put the frame into your application's
 * input queue. If this function takes too long, you'll start losing frames. */
void Camera::LibuvcCallback(uvc_frame_t *frame, void *ptr) {
  uvc_frame_t *decodedFrame;
  uvc_error_t ret;
  auto camera = reinterpret_cast<Camera*>(ptr);
  auto frameIndex = camera->frameCounter_++;

  printf("callback\n");
  decodedFrame = uvc_allocate_frame(frame->width * frame->height * 3);
  if (!decodedFrame) {
    printf("ERROR: unable to allocate decodedFrame frame!\n");
    return;
  }

  ret = uvc_mjpeg2rgb(frame, decodedFrame);
  if (ret) {
    uvc_perror(ret, "uvc_mjpeg2rgb");
    uvc_free_frame(decodedFrame);
    return;
  }

  cv::Mat mat(cv::Size(decodedFrame->width, decodedFrame->height),
               CV_8UC3,
               decodedFrame->data,
               cv::Mat::AUTO_STEP);
  cv::Mat toBgr;
  cv::cvtColor(mat, toBgr, cv::COLOR_RGB2BGR);

#if _WIN32
  // I don't have OpenCV configured with GUI support on Windows so
  // for now, just write images to file instead of displaying in window.
  ostringstream ss;
  ss << "image_"
     << hex << camera->vid_ << "_"
     << hex << camera->pid_ << "_"
     << frameIndex << ".jpg";
  
  std::string filename = ss.str();
  cv::imwrite(filename, toBgr);
#else
  cv::namedWindow("Test", cv::WINDOW_NORMAL);
  cv::resizeWindow("Test", 1920, 1080);
  cv::imshow("Test", toBgr);
  cv::waitKey(10);
#endif
  uvc_free_frame(decodedFrame);
}

// -----------------------------------------------------------------------------
// main
// -----------------------------------------------------------------------------

int main(int argc, char **argv) {
  uvc_context_t *ctx;
  uvc_error_t res;
  
  /* Initialize a UVC service context. Libuvc will set up its own libusb
   * context. Replace NULL with a libusb_context pointer to run libuvc
   * from an existing libusb context. */
  res = uvc_init(&ctx, NULL);
  if (res < 0) {
    uvc_perror(res, "uvc_init");
    return res;
  }
  printf("UVC initialized\n");

  Camera camera1(0x05A3, 0x9520,  640,  480, 30, ctx);
  Camera camera2(0x05A3, 0x2214, 3840, 2880,  5, ctx);

  camera1.OpenDevice();
  camera1.StartStreaming();
  std::this_thread::sleep_for(std::chrono::seconds(4));
  camera1.CloseDevice();

  std::this_thread::sleep_for(std::chrono::seconds(2));

#if 0
  camera2.OpenDevice();
  camera2.StartStreaming();
  std::this_thread::sleep_for(std::chrono::seconds(2));
  camera2.CloseDevice();
#endif

  /* Close the UVC context. This closes and cleans up any existing device handles,
   * and it closes the libusb context if one was not provided. */
  uvc_exit(ctx);
  printf("UVC exited\n");
  return 0;
}
