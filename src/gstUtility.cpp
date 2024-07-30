#include "../includes/gst.h"
#include "../includes/headers.h"

void ixgStream::gstBasePipe::getLiveStatus() {
  GstState state, pending;
  gst_element_get_state(P, &state, &pending, GST_CLOCK_TIME_NONE);
  if (state == GST_STATE_PLAYING)
    live = true;
  else
    live = false;
}
