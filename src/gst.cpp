

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

int ixgStream::gstOutputTeePipe::gstOutputPipeStart(GstElement *P,
                                                    ixg::ixgElement **elemt,
                                                    int pch, int cch) {
  // GstPad *sinkpad;
  GstPadTemplate *muxtempl;

  if ((elemt[pch]->__ixgElemt.__ixgteeOutput[cch]._ixgtype == srt) ||
      (elemt[pch]->__ixgElemt.__ixgteeOutput[cch]._ixgtype == multicast) ||
      (elemt[pch]->__ixgElemt.__ixgteeOutput[cch]._ixgtype == record) ||
      (elemt[pch]->__ixgElemt.__ixgteeOutput[cch]._ixgtype == delay))
    muxtee = gst_bin_get_by_name(GST_BIN(P), "mpegtsmuxtee");
  else
    muxtee = gst_bin_get_by_name(GST_BIN(P), "flvmuxertee");

  if (!muxtee) {
    gst_print("no element with name \"muxtee\" found\n");
    gst_object_unref(P);
    return -3;
  }

  muxtempl = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(muxtee),
                                                "src_%u");
  muxteepad = gst_element_request_pad(muxtee, muxtempl, NULL, NULL);
  queue_mux = gst_element_factory_make("queue", NULL);

  if (elemt[pch]->__ixgElemt.__ixgteeOutput[cch]._ixgtype == srt) { // srt
    sink = gst_element_factory_make("srtsink", NULL);
    g_object_set(sink, "uri",
                 elemt[pch]->__ixgElemt.__ixgteeOutput[cch]._ixgIp._ixgSrt.uri,
                 "wait-for-connection", true, "poll-timeout", 2000, NULL);

  } else if (elemt[pch]->__ixgElemt.__ixgteeOutput[cch]._ixgtype ==
             multicast) { // multicast
    cout << "calling multicast" << endl;
    sink = gst_element_factory_make("udpsink", NULL);
    g_object_set(
        sink, "host",
        elemt[pch]
            ->__ixgElemt.__ixgteeOutput[cch]
            ._ixgIp._ixgMulticast.ip.c_str(),
        "port",
        elemt[pch]->__ixgElemt.__ixgteeOutput[cch]._ixgIp._ixgMulticast.port,
        NULL);
  } else if (elemt[pch]->__ixgElemt.__ixgteeOutput[cch]._ixgtype ==
             rtmp) { // rtmp
    sink = gst_element_factory_make("rtmpsink", NULL);
    g_object_set(
        sink, "location",
        elemt[pch]->__ixgElemt.__ixgteeOutput[cch]._ixgIp._ixgRtmp.url.c_str(),
        NULL);
  } else if (elemt[pch]->__ixgElemt.__ixgteeOutput[cch]._ixgtype ==
             record) { // record mpegts type only now
    sink = gst_element_factory_make("filesink", NULL);
    string location =
        elemt[pch]->__ixgElemt.__ixgteeOutput[cch]._ixgRec.rec_location;
    g_object_set(sink, "location", location.c_str(), NULL);
  } else if (elemt[pch]->__ixgElemt.__ixgteeOutput[cch]._ixgtype ==
             delay) { // delay
    sink = gst_element_factory_make("multifilesink", NULL);
    string location =
        elemt[pch]->__ixgElemt.__ixgteeOutput[cch]._ixgDelay.rec_location +
        "/test-%05d.ts";
    g_object_set(sink, "location", location.c_str(), "next-file", 2,
                 "max-files", 60, "min-keyframe-distance", 30000000000, "index",
                 0, "aggregate-gops", true, NULL);
  }

  gst_bin_add_many(GST_BIN(P), queue_mux, sink, NULL);
  gst_element_link_many(queue_mux, sink, NULL);

  gst_element_sync_state_with_parent(queue_mux);
  gst_element_sync_state_with_parent(sink);

  sinkpad = gst_element_get_static_pad(queue_mux, "sink");
  gst_pad_link(muxteepad, sinkpad);
  // gst_object_unref(sinkpad);

  return 0;
}

static GstPadProbeReturn unlink_output_cb(GstPad *pad, GstPadProbeInfo *info,
                                          gpointer user_data) {
  g_print("Unlinking.........1\n");

  GstPad *sinkpad;
  ixgStream::gstOutputTeePipe *gstData =
      static_cast<ixgStream::gstOutputTeePipe *>(user_data);
  sinkpad = gst_element_get_static_pad(gstData->queue_mux, "sink");

  // Softly unlink the pads from the tee
  gst_pad_send_event(sinkpad, gst_event_new_flush_start());
  gst_pad_unlink(gstData->muxteepad, sinkpad);
  // gst_pad_send_event(sinkpad, gst_event_new_flush_stop(false));
  gst_element_send_event(gstData->sink, gst_event_new_eos());
  // gst_pad_unlink (gstData->muxteepad, sinkpad);
  gst_object_unref(sinkpad);

  // gst_element_send_event(gstData->sink, gst_event_new_eos());
  // g_usleep(2000000);
  return GST_PAD_PROBE_REMOVE;
}

void ixgStream::gstOutputTeePipe::gstOutputPipeStop(GstElement *P, int type) {
  // g_print("stop srt in type mode\n");
  PP = P;
  outType = type;
  gst_pad_add_probe(muxteepad, GST_PAD_PROBE_TYPE_IDLE, unlink_output_cb, this,
                    NULL);
  finalize();
  PP = NULL;
  outType = -1;
}

void ixgStream::gstOutputTeePipe::finalize() {
  GstElement *tee;

  if (outType == 1)
    tee = gst_bin_get_by_name(GST_BIN(PP), "mpegtsmuxtee");
  else if (outType == 2)
    tee = gst_bin_get_by_name(GST_BIN(PP), "flvmuxtee");
  else {
    cout << "tee not fount:No action" << endl;
    return;
  }

  if (!tee) {
    gst_print("no element with name \"tee\" found\n");
    gst_object_unref(PP);
  }

  gst_element_set_state(queue_mux, GST_STATE_PAUSED);
  gst_element_set_state(sink, GST_STATE_PAUSED);

  gst_element_set_state(queue_mux, GST_STATE_NULL);
  gst_element_set_state(sink, GST_STATE_NULL);

  gst_bin_remove(GST_BIN(PP), queue_mux);
  gst_bin_remove(GST_BIN(PP), sink);

  // int count = GST_OBJECT_REFCOUNT( queue_aenc );
  // cout<<"queue_aenc count: "<<count<<endl;

  // gst_object_unref(queue_mux);
  // gst_object_unref(sink);

  gst_element_release_request_pad(tee, muxteepad);
  gst_object_unref(muxteepad);

  g_print("Unlinked\n");
}
