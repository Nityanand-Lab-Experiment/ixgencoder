

#include "../includes/gst.h"
#include "../includes/headers.h"

#define GSTREAMER_VER 1.18

GstElement *multifilesrc1;
GstPad *sinkpad;

extern bool active[4];

static gboolean message_cb(GstBus *bus, GstMessage *message,
                           gpointer user_data) {
  ixgStream::gstBasePipe *gstData =
      static_cast<ixgStream::gstBasePipe *>(user_data);
  bool status;
  switch (GST_MESSAGE_TYPE(message)) {
  case GST_MESSAGE_ERROR: {
    GError *err = NULL;
    gchar *name, *debug = NULL;

    name = gst_object_get_path_string(message->src);
    gst_message_parse_error(message, &err, &debug);
    const gchar *element_name = GST_OBJECT_NAME(GST_MESSAGE_SRC(message));
    g_print("\nerror element : %s\n", element_name);
    g_printerr("\nERROR: from element %s: %s\n", name, err->message);

    if (debug != NULL) {
      g_printerr("\nAdditional debug info:\n%s\n", debug);
    }
    GstElement *error_element = GST_ELEMENT(message->src);
    cout << "srtsink: " << error_element << endl;

    g_error_free(err);
    g_free(debug);
    g_free(name);

    // g_main_loop_quit (loop);
    break;
  }
  case GST_MESSAGE_WARNING: {
    GError *err = NULL;
    gchar *name, *debug = NULL;

    name = gst_object_get_path_string(message->src);
    gst_message_parse_warning(message, &err, &debug);

    g_printerr("\nERROR: from element %s: %s\n", name, err->message);

    if (debug != NULL) {
      g_printerr("Additional debug info:\n%s\n", debug);
    }
    g_error_free(err);
    g_free(debug);
    g_free(name);
    break;
  }
  case GST_MESSAGE_EOS: {
    g_print("Got EOS\n");
    // g_main_loop_quit (loop);
    // gst_element_set_state (pipeline, GST_STATE_NULL);
    // g_main_loop_unref (loop);
    // gst_object_unref (pipeline);
    // exit(0);
    break;
  }
  case GST_MESSAGE_ELEMENT: {
    const GstStructure *s = gst_message_get_structure(message);

    if (gst_structure_has_name(s, "GstBinForwarded")) {
      GstMessage *forward_msg = NULL;

      gst_structure_get(s, "message", GST_TYPE_MESSAGE, &forward_msg, NULL);
      if (GST_MESSAGE_TYPE(forward_msg) == GST_MESSAGE_EOS) {
        g_print("\nEOS from element %s\n",
                GST_OBJECT_NAME(GST_MESSAGE_SRC(forward_msg)));
      }
      gst_message_unref(forward_msg);
    }
    break;
  }
  case GST_MESSAGE_STATE_CHANGED:
    if (GST_MESSAGE_SRC(message) == GST_OBJECT(gstData->getpipe())) {
      GstState old_state, new_state, pending_state;
      gst_message_parse_state_changed(message, &old_state, &new_state,
                                      &pending_state);
      g_print("\nPipeline state changed from %s to %s:\n",
              gst_element_state_get_name(old_state),
              gst_element_state_get_name(new_state));
    }
    break;
  default:
    break;
  }

  return TRUE;
}

int ixgStream::gstBasePipe::gstBasePipeline(ixg::ixgElement **elemt, int ch) {
  cout << "Craete gstreamer pipeline" << endl;
  P = gst_pipeline_new(NULL);

  _ixgStr inputs;

  _type inputtype = elemt[ch]->__ixgElemt.__ixgInput._ixgtype;

  if (inputtype == testsrc) {
    inputs = "videotestsrc ! video/x-raw, width=1920, height=1080 ";
  } else if (inputtype == file) {
    cout << "TO DO" << endl;
  } else if (inputtype == sdi) {
    inputs = "decklinkvideosrc device-number=" +
             to_string(elemt[ch]->__ixgElemt.__ixgInput._ixgSdi.device_number) +
             " connection=sdi mode=" +
             elemt[ch]->__ixgElemt.__ixgInput._ixgSdi.format +
             " buffer-size=5 ! video/x-raw,width=1920,height=1080 ";
  }

  _ixgStr vconvert = "! perf bitrate-interval=1000 ! videoconvert ! "
                     "video/x-raw,format=I420 ! tee name=vraw  ";
  _ixgStr vencode =
      "! nvh264enc  zerolatency=true strict-gop=false gop-size=" +
      elemt[ch]->__ixgElemt.__ixgEncoder.gop +
      " bframes=2 "
      "nonref-p=false  rc-lookahead=0 "
      "rc-mode=" +
      elemt[ch]->__ixgElemt.__ixgEncoder.bitrate_type +
      " preset=4 spatial-aq=true vbv-buffer-size=1024 bitrate=" +
      elemt[ch]->__ixgElemt.__ixgEncoder.bitrate +
      " max-bitrate=" + elemt[ch]->__ixgElemt.__ixgEncoder.max_bitrate +
      " ! video/x-h264,profile=high ! tee name=venctee ";
  _ixgStr muxer = "! queue ! h264parse config-interval=-1 ! mpegtsmux "
                  "alignment=7 name=mux ";

  _type outputtype = elemt[ch]->__ixgElemt.__ixgOutput._ixgtype;
  _ixgStr outputs;
  if (outputtype == ip) {
    outputs = "! tee name=mpegtsmuxtee ! queue ! perf "
              "bitrate-interval=1000 ! udpsink host=" +
              elemt[ch]->__ixgElemt.__ixgOutput._ixgIp._ixgMulticast.ip +
              " port=" +
              std::to_string(
                  elemt[ch]->__ixgElemt.__ixgOutput._ixgIp._ixgMulticast.port) +
              " sync=false ";
  }

  _ixgStr audiopipe;
  if (inputtype == sdi) {
    audiopipe =
        "decklinkaudiosrc device-number=" +
        to_string(elemt[ch]->__ixgElemt.__ixgInput._ixgSdi.device_number) +
        " channels=2 buffer-size=5 ! audio/x-raw,format=S16LE "
        "! audioconvert ! tee name = araw ! queue ! voaacenc "
        "bitrate = 128000 ! tee name = aenctee ! queue ! aacparse ! mux.";
  } else if (inputtype == testsrc) {
    audiopipe = "audiotestsrc ! audio/x-raw,format=S16LE ! audioconvert ! tee "
                " name=araw ! queue ! voaacenc bitrate=128000 ! tee "
                "name=aenctee ! queue ! "
                "aacparse ! mux.";
  }
  _ixgStr flvmuxer =
      "venctee. ! queue ! h264parse config-interval=-1 ! flvmux streamable = "
      "true name = flvmuxer ! tee name = flvmuxertee ! queue !"
      " fakesink aenctee. ! queue ! aacparse ! flvmuxer.";
  _ixgStr display =
      " vraw. ! queue ! videoconvert ! video/x-raw,format=UYVY ! "
      "decklinkvideosink device-number=3 mode=15 araw. ! queue ! "
      "audioconvert "
      "! audio/x-raw,format=S16LE ! decklinkaudiosink device-number=3";

  _ixgStr pipelineStr =
      inputs + vconvert + vencode + muxer + outputs + audiopipe;
  // +flvmuxer; // + display;

  cout << pipelineStr << endl;

  P = gst_parse_launch(pipelineStr.c_str(), &error);
  if (error) {
    g_print("Error while parsing pipeline description: %s\n", error->message);
    return -1;
  }
  g_object_set(GST_BIN(P), "message-forward", TRUE, NULL);

  loop = g_main_loop_new(NULL, FALSE);
  bus = gst_element_get_bus(P);
  gst_bus_add_signal_watch(bus);

  g_signal_connect(G_OBJECT(bus), "message", G_CALLBACK(message_cb), this);
  gst_object_unref(GST_OBJECT(bus));

  return 0;
}
void ixgStream::gstBasePipe::startLoop() { g_main_loop_run(loop); }

void ixgStream::gstBasePipe::start(bool *start, bool action) {
  /* Start playing the pipeline */
  cout << "start base pipeline" << endl;

  gst_element_set_state(P, GST_STATE_PLAYING);
  if (gst_element_get_state(P, &state, NULL, GST_CLOCK_TIME_NONE) ==
          GST_STATE_CHANGE_FAILURE ||
      state != GST_STATE_PLAYING) {
    g_warning("State change to playing failed");
  }

  std::thread t(&ixgStream::gstBasePipe::startLoop, this);
  t.detach();
  // t.join();
  //*start=action;
}
void ixgStream::gstBasePipe::stop(bool *start, bool action) {
  /* Start playing the pipeline */
  *start = action;
  g_main_loop_quit(loop);
  gst_element_set_state(P, GST_STATE_NULL);
  gst_object_unref(P);
}
