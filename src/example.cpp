// Example code taken from https://ken.tossell.net/libuvc/doc/ and slightly modified.
//
// Also did these steps:
//
//   * sudo apt install libgtk2.0-dev
//
// mkdir build && cd build
// cmake -G "Unix Makefiles" ..
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
#include <libusb-1.0/libusb.h>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio/registry.hpp>
#include <opencv2/core/types_c.h>
#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/core.hpp>
#include <stdio.h>

/* This callback function runs once per frame. Use it to perform any
 * quick processing you need, or have it put the frame into your application's
 * input queue. If this function takes too long, you'll start losing frames. */
void cb(uvc_frame_t *frame, void *ptr) {
  uvc_frame_t *decodedFrame;
  uvc_error_t ret;
  printf("callback\n");
  decodedFrame = uvc_allocate_frame(frame->width * frame->height * 3);
  if (!decodedFrame) {
    printf("unable to allocate decodedFrame frame!");
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

  cv::namedWindow("Test", cv::WINDOW_NORMAL);
  cv::resizeWindow("Test", 1920, 1080);
  cv::imshow("Test", toBgr);
  cv::waitKey(10);

  uvc_free_frame(decodedFrame);
}

struct libuvc_callback_data_t
{
};

libuvc_callback_data_t libuvc_callback_data;

int main(int argc, char **argv) {
  uvc_context_t *ctx;
  uvc_device_t *dev;
  uvc_device_handle_t *devh;
  uvc_stream_ctrl_t ctrl;
  uvc_error_t res;
  /* Initialize a UVC service context. Libuvc will set up its own libusb
   * context. Replace NULL with a libusb_context pointer to run libuvc
   * from an existing libusb context. */
  res = uvc_init(&ctx, NULL);
  if (res < 0) {
    uvc_perror(res, "uvc_init");
    return res;
  }
  puts("UVC initialized");
  /* Locates the first attached UVC device, stores in dev */
  res = uvc_find_device(
      ctx, &dev,
      0, 0, NULL); /* filter devices: vendor_id, product_id, "serial_num" */
  if (res < 0) {
    uvc_perror(res, "uvc_find_device"); /* no devices found */
  } else {
    puts("Device found");
    /* Try to open the device: requires exclusive access */
    res = uvc_open(dev, &devh);
    if (res < 0) {
      uvc_perror(res, "uvc_open"); /* unable to open device */
    } else {
      puts("Device opened");
#if 0
      {
        res = libusb_detach_kernel_driver(devh, 1);
        if (res < 0) {
          printf("libusb_detach_kernel_driver %d %s\n", res, libusb_strerror(res));
        }
      }
#endif
      /* Print out a message containing all the information that libuvc
       * knows about the device */
      uvc_print_diag(devh, stderr);
      /* Try to negotiate a 640x480 30 fps YUYV stream profile */
      res = uvc_get_stream_ctrl_format_size(
          devh, &ctrl, /* result stored in ctrl */
          UVC_FRAME_FORMAT_MJPEG,
//          640, 480, 5 /* width, height, fps */
          3840, 2880, 5 /* width, height, fps */
      );
      /* Print out the result */
      uvc_print_stream_ctrl(&ctrl, stderr);
      if (res < 0) {
        uvc_perror(res, "get_mode"); /* device doesn't provide a matching stream */
      } else {
        /* Start the video stream. The library will call user function cb:
         *   cb(frame, (void*) libuvc_callback_data)
         */
        res = uvc_start_streaming(devh, &ctrl, cb, (void*)&libuvc_callback_data, 0);
        if (res < 0) {
          uvc_perror(res, "start_streaming"); /* unable to start stream */
        } else {
          puts("Streaming...");
//          uvc_set_ae_mode(devh, 1); /* auto exposure */
          sleep(10); /* stream for 10 seconds */
          /* End the stream. Blocks until last callback is serviced */
          uvc_stop_streaming(devh);
          puts("Done streaming.");
        }
      }
      /* Release our handle on the device */
      uvc_close(devh);
      puts("Device closed");
    }
    /* Release the device descriptor */
    uvc_unref_device(dev);
  }
  /* Close the UVC context. This closes and cleans up any existing device handles,
   * and it closes the libusb context if one was not provided. */
  uvc_exit(ctx);
  puts("UVC exited");
  return 0;
}
