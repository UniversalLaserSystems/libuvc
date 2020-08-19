/** @file libuvc_internal.h
  * @brief Implementation-specific UVC constants and structures.
  * @cond include_hidden
  */
#ifndef LIBUVC_INTERNAL_H
#define LIBUVC_INTERNAL_H

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#pragma warning (disable:4200)
#include <libusb-1.0/libusb.h>
#pragma warning (default:4200)
#include "utlist.h"

/** Converts an unaligned four-byte little-endian integer into an int32 */
#define DW_TO_INT(p) ((p)[0] | ((p)[1] << 8) | ((p)[2] << 16) | ((p)[3] << 24))
/** Converts an unaligned two-byte little-endian integer into an int16 */
#define SW_TO_SHORT(p) ((p)[0] | ((p)[1] << 8))
/** Converts an int16 into an unaligned two-byte little-endian integer */
#define SHORT_TO_SW(s, p) \
  (p)[0] = (s); \
  (p)[1] = (s) >> 8;
/** Converts an int32 into an unaligned four-byte little-endian integer */
#define INT_TO_DW(i, p) \
  (p)[0] = (i); \
  (p)[1] = (i) >> 8; \
  (p)[2] = (i) >> 16; \
  (p)[3] = (i) >> 24;

/** Selects the nth item in a doubly linked list. n=-1 selects the last item. */
#define DL_NTH(head, out, n) \
  do { \
    int dl_nth_i = 0; \
    LDECLTYPE(head) dl_nth_p = (head); \
    if ((n) < 0) { \
      while (dl_nth_p && dl_nth_i > (n)) { \
        dl_nth_p = dl_nth_p->prev; \
        dl_nth_i--; \
      } \
    } else { \
      while (dl_nth_p && dl_nth_i < (n)) { \
        dl_nth_p = dl_nth_p->next; \
        dl_nth_i++; \
      } \
    } \
    (out) = dl_nth_p; \
  } while (0);

#ifdef UVC_DEBUGGING
#define UVC_DEBUG(format, ...) fprintf(stderr, "[%s:%d/%s] " format "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define UVC_ENTER() fprintf(stderr, "[%s:%d] begin %s\n", __FILE__, __LINE__, __func__)
#define UVC_EXIT(code) fprintf(stderr, "[%s:%d] end %s (%d)\n", __FILE__, __LINE__, __func__, code)
#define UVC_EXIT_VOID() fprintf(stderr, "[%s:%d] end %s\n", __FILE__, __LINE__, __func__)
#else
#define UVC_DEBUG(format, ...)
#define UVC_ENTER()
#define UVC_EXIT_VOID()
#define UVC_EXIT(code)
#endif

/** Video interface subclass code (A.2) */
enum uvc_int_subclass_code {
  UVC_SC_UNDEFINED = 0x00,
  UVC_SC_VIDEOCONTROL = 0x01,
  UVC_SC_VIDEOSTREAMING = 0x02,
  UVC_SC_VIDEO_INTERFACE_COLLECTION = 0x03
};

/** Video interface protocol code (A.3) */
enum uvc_int_proto_code {
  UVC_PC_PROTOCOL_UNDEFINED = 0x00
};

/** VideoControl interface descriptor subtype (A.5) */
enum uvc_vc_desc_subtype {
  UVC_VC_DESCRIPTOR_UNDEFINED = 0x00,
  UVC_VC_HEADER = 0x01,
  UVC_VC_INPUT_TERMINAL = 0x02,
  UVC_VC_OUTPUT_TERMINAL = 0x03,
  UVC_VC_SELECTOR_UNIT = 0x04,
  UVC_VC_PROCESSING_UNIT = 0x05,
  UVC_VC_EXTENSION_UNIT = 0x06
};

/** UVC endpoint descriptor subtype (A.7) */
enum uvc_ep_desc_subtype {
  UVC_EP_UNDEFINED = 0x00,
  UVC_EP_GENERAL = 0x01,
  UVC_EP_ENDPOINT = 0x02,
  UVC_EP_INTERRUPT = 0x03
};

/** VideoControl interface control selector (A.9.1) */
enum uvc_vc_ctrl_selector {
  UVC_VC_CONTROL_UNDEFINED = 0x00,
  UVC_VC_VIDEO_POWER_MODE_CONTROL = 0x01,
  UVC_VC_REQUEST_ERROR_CODE_CONTROL = 0x02
};

/** Terminal control selector (A.9.2) */
enum uvc_term_ctrl_selector {
  UVC_TE_CONTROL_UNDEFINED = 0x00
};

/** Selector unit control selector (A.9.3) */
enum uvc_su_ctrl_selector {
  UVC_SU_CONTROL_UNDEFINED = 0x00,
  UVC_SU_INPUT_SELECT_CONTROL = 0x01
};

/** Extension unit control selector (A.9.6) */
enum uvc_xu_ctrl_selector {
  UVC_XU_CONTROL_UNDEFINED = 0x00
};

/** VideoStreaming interface control selector (A.9.7) */
enum uvc_vs_ctrl_selector {
  UVC_VS_CONTROL_UNDEFINED = 0x00,
  UVC_VS_PROBE_CONTROL = 0x01,
  UVC_VS_COMMIT_CONTROL = 0x02,
  UVC_VS_STILL_PROBE_CONTROL = 0x03,
  UVC_VS_STILL_COMMIT_CONTROL = 0x04,
  UVC_VS_STILL_IMAGE_TRIGGER_CONTROL = 0x05,
  UVC_VS_STREAM_ERROR_CODE_CONTROL = 0x06,
  UVC_VS_GENERATE_KEY_FRAME_CONTROL = 0x07,
  UVC_VS_UPDATE_FRAME_SEGMENT_CONTROL = 0x08,
  UVC_VS_SYNC_DELAY_CONTROL = 0x09
};

/** Status packet type (2.4.2.2) */
enum uvc_status_type {
  UVC_STATUS_TYPE_CONTROL = 1,
  UVC_STATUS_TYPE_STREAMING = 2
};

/** Payload header flags (2.4.3.3) */
#define UVC_STREAM_EOH (1 << 7)
#define UVC_STREAM_ERR (1 << 6)
#define UVC_STREAM_STI (1 << 5)
#define UVC_STREAM_RES (1 << 4)
#define UVC_STREAM_SCR (1 << 3)
#define UVC_STREAM_PTS (1 << 2)
#define UVC_STREAM_EOF (1 << 1)
#define UVC_STREAM_FID (1 << 0)

/** Control capabilities (4.1.2) */
#define UVC_CONTROL_CAP_GET (1 << 0)
#define UVC_CONTROL_CAP_SET (1 << 1)
#define UVC_CONTROL_CAP_DISABLED (1 << 2)
#define UVC_CONTROL_CAP_AUTOUPDATE (1 << 3)
#define UVC_CONTROL_CAP_ASYNCHRONOUS (1 << 4)

struct uvc_streaming_interface;
struct uvc_device_info;

/** VideoStream interface */
typedef struct uvc_streaming_interface {
  struct uvc_device_info *parent;
  struct uvc_streaming_interface *prev, *next;
  /** Interface number */
  uint8_t bInterfaceNumber;
  /** Video formats that this interface provides */
  struct uvc_format_desc *format_descs;
  /** USB endpoint to use when communicating with this interface */
  uint8_t bEndpointAddress;
  uint8_t bTerminalLink;
  uint8_t bStillCaptureMethod;
} uvc_streaming_interface_t;

/** VideoControl interface */
typedef struct uvc_control_interface {
  struct uvc_device_info *parent;
  struct uvc_input_terminal *input_term_descs;
  // struct uvc_output_terminal *output_term_descs;
  struct uvc_selector_unit *selector_unit_descs;
  struct uvc_processing_unit *processing_unit_descs;
  struct uvc_extension_unit *extension_unit_descs;
  uint16_t bcdUVC;
  uint32_t dwClockFrequency;
  uint8_t bEndpointAddress;
  /** Interface number */
  uint8_t bInterfaceNumber;
} uvc_control_interface_t;

struct uvc_stream_ctrl;

struct uvc_device {
  struct uvc_context *ctx;
  int ref;
  libusb_device *usb_dev;

  uvc_device()
    : ctx(nullptr)
    , ref(0)
    , usb_dev(0) {
  }
};

typedef struct uvc_device_info {
  /** Configuration descriptor for USB device */
  struct libusb_config_descriptor *config;
  /** VideoControl interface provided by device */
  uvc_control_interface_t ctrl_if;
  /** VideoStreaming interfaces on the device */
  uvc_streaming_interface_t *stream_ifs;
} uvc_device_info_t;

struct uvc_stream_config_t {
  size_t number_of_transport_buffers;
  size_t size_of_transport_buffer;
  size_t size_of_meta_transport_buffer;
};

extern uvc_stream_config_t uvc_stream_config;

struct libusb_transfer_deleter {
  void operator()(struct libusb_transfer* t) {
    free(t->buffer);
    libusb_free_transfer(t);
  }
};

typedef std::unique_ptr<struct libusb_transfer, struct libusb_transfer_deleter> unique_ptr_libusb_transfer;

struct uvc_stream_handle {
  struct uvc_device_handle *devh;
  struct uvc_stream_handle *prev, *next;
  struct uvc_streaming_interface *stream_if;

  /** if true, stream is running (streaming video to host) */
  uint8_t running;
  /** Current control block */
  struct uvc_stream_ctrl cur_ctrl;

  /* listeners may only access hold*, and only when holding a
   * lock on cb_mutex (probably signaled with cb_cond)
   *
   * The struct libusb_transfer* callback (_uvc_stream_callback) copies bytes
   * to uint8_t *outbuf. When outbuf contains a full frame (determined by EOF
   * bit), the contents of oubuf are copied to uint8_t *holdbuf and
   * callback_cond is notified. The waiting _uvc_user_caller() thread then
   * calls the user's callback function with the completed frame.
   */
  
  uint8_t fid;
  uint32_t seq, hold_seq;
  uint32_t pts, hold_pts;
  uint32_t last_scr, hold_last_scr;
  std::vector<uint8_t> outbuf;
  std::vector<uint8_t> holdbuf;
  std::mutex callback_mutex;
  std::condition_variable callback_cond;
  std::thread callback_thread;
  uint32_t last_polled_seq;
  uvc_frame_callback_t *user_cb;
  void *user_ptr;
  /*
   * Each transfer is a unique_ptr<libusb_transfer> whose underlying raw pointer
   * is managed by libusb_alloc_transfer/libusb_free_transfer. The
   * libusb_transfer also has a buffer member that we are responsible for
   * freeing which is done by the custom deleter, libusb_transfer_deleter.
   */
  std::vector<std::unique_ptr<struct libusb_transfer, libusb_transfer_deleter>> transfers;
  struct uvc_frame frame;
  enum uvc_frame_format frame_format;
  std::chrono::steady_clock::time_point capture_time_finished;
  /* raw metadata buffer if available */
  std::vector<uint8_t> meta_outbuf;
  std::vector<uint8_t> meta_holdbuf;

  uvc_stream_handle()
    : devh(nullptr)
    , prev(nullptr)
    , next(nullptr)
    , stream_if(nullptr)
    , running(0)
    //, cur_ctrl default constructed
    , fid(0)
    , seq(0)
    , hold_seq(0)
    , pts(0)
    , hold_pts(0)
    , last_scr(0)
    , hold_last_scr(0)
    , last_polled_seq(0)
    , user_cb(nullptr)
    , user_ptr(nullptr)
    , transfers(uvc_stream_config.number_of_transport_buffers)
    //, frame default constructed
    , frame_format(UVC_FRAME_FORMAT_UNKNOWN)
    //, capture_time_finished default constructed
  {
    /** @todo take only what we need */
    outbuf.reserve(uvc_stream_config.size_of_transport_buffer);
    holdbuf.reserve(uvc_stream_config.size_of_transport_buffer);
    meta_outbuf.reserve(uvc_stream_config.size_of_meta_transport_buffer);
    meta_holdbuf.reserve(uvc_stream_config.size_of_meta_transport_buffer);
  }

  ~uvc_stream_handle() {
  }
};

/** Handle on an open UVC device
 *
 * @todo move most of this into a uvc_device struct?
 */
struct uvc_device_handle {
  struct uvc_device *dev;
  struct uvc_device_handle *prev, *next;
  /** Underlying USB device handle */
  libusb_device_handle *usb_devh;
  struct uvc_device_info *info;
  struct libusb_transfer *status_xfer;
  uint8_t status_buf[32];
  /** Function to call when we receive status updates from the camera */
  uvc_status_callback_t *status_cb;
  void *status_user_ptr;
  /** Function to call when we receive button events from the camera */
  uvc_button_callback_t *button_cb;
  void *button_user_ptr;

  uvc_stream_handle_t *streams;
  /** Whether the camera is an iSight that sends one header per frame */
  uint8_t is_isight;
  uint32_t claimed;

  uvc_device_handle()
    : dev(nullptr)
    , prev(nullptr)
    , next(nullptr)
    , usb_devh(nullptr)
    , info(nullptr)
    , status_xfer(nullptr)
    //, status_buf
    , status_cb(nullptr)
    , status_user_ptr(nullptr)
    , button_cb(nullptr)
    , button_user_ptr(nullptr)
    , streams(nullptr)
    , is_isight(0)
    , claimed(0) {
    memset(status_buf, 0, sizeof(status_buf));
  }
  ~uvc_device_handle() {
    if (info)
      delete info;

    if (status_xfer)
      libusb_free_transfer(status_xfer);
  }
};

/** Context within which we communicate with devices */
struct uvc_context {
  /** Underlying context for USB communication */
  struct libusb_context *usb_ctx;
  /** True iff libuvc initialized the underlying USB context */
  uint8_t own_usb_ctx;
  /** List of open devices in this context */
  uvc_device_handle_t *open_devices;
  std::thread handler_thread;
  int kill_handler_thread;

  uvc_context()
    : usb_ctx(nullptr)
    , own_usb_ctx(0)
    , open_devices(nullptr)
    //, handler_thread default constructed
    , kill_handler_thread(0) {
  }
};

uvc_error_t uvc_query_stream_ctrl(
    uvc_device_handle_t *devh,
    uvc_stream_ctrl_t *ctrl,
    uint8_t probe,
    enum uvc_req_code req);

void uvc_start_handler_thread(uvc_context_t *ctx);
uvc_error_t uvc_claim_if(uvc_device_handle_t *devh, int idx);
uvc_error_t uvc_release_if(uvc_device_handle_t *devh, int idx);

#endif // !def(LIBUVC_INTERNAL_H)
/** @endcond */

