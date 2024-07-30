#ifndef GST_INCLUDE
#define GST_INCLUDE

#include "utils.h"
#include <glib.h>
#include <gst/gst.h>
#include <memory>

namespace ixgStream {
class gstBasePipe {
private:
  GMainLoop *loop;
  GstElement *P, *src, *vidConv, *vidRate, *capsfiltersrc, *capsfilterRate,
      *queue, *vidEncoder, *muxer, *filesink, *queue_vid, *vidParse;
  GstElement *srtsink, *srtsink_enc, *audParse, *queue_aud, *appsink;
  GError *error = NULL;
  GstBus *bus;
  GstState state;
  bool live = false;

public:
  int gstBasePipeline(ixg::ixgElement **elem, int ch);
  void start(bool *, bool);
  void stop(bool *, bool);
  GMainLoop *getloop() { return loop; }
  GstElement *getpipe() { return P; }
  void startLoop();
  void getLiveStatus();
  // void inputBitrate();
  // void outputBitrate();
};
} // namespace ixgStream
#endif