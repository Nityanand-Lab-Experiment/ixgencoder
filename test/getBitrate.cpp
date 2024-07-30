#include <gst/gst.h>
#include <iostream>
#include <thread>

typedef struct _CustomData {
  GstElement *pipeline;
  GstElement *source;
  GstElement *encoder;
  GstElement *payloader;
  GstElement *udpsink;
  GMainLoop *loop; // Added GMainLoop pointer for main loop handling
  GstPad *sink_pad;
} CustomData;

// Bus callback function to handle messages from GStreamer
static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer user_data) {
  CustomData *data = (CustomData *)user_data;

  switch (GST_MESSAGE_TYPE(msg)) {
  case GST_MESSAGE_ERROR: {
    GError *err;
    gchar *debug_info;

    gst_message_parse_error(msg, &err, &debug_info);
    g_printerr("Error received from element %s: %s\n",
               GST_OBJECT_NAME(msg->src), err->message);
    g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
    g_clear_error(&err);
    g_free(debug_info);

    gst_element_set_state(data->pipeline, GST_STATE_NULL);
    g_main_loop_quit(data->loop);
    break;
  }
  case GST_MESSAGE_EOS:
    g_print("End-Of-Stream reached.\n");
    gst_element_set_state(data->pipeline, GST_STATE_NULL);
    g_main_loop_quit(data->loop);
    break;
  default:
    break;
  }

  return TRUE;
}

// Initialize GStreamer and create pipeline elements
static gboolean init_gstreamer(CustomData *data) {
  GstCaps *caps;
  GstBus *bus;

  // Initialize GStreamer
  gst_init(NULL, NULL);

  // Create elements
  data->source = gst_element_factory_make("videotestsrc", "source");
  data->encoder = gst_element_factory_make("x264enc", "encoder");
  data->payloader = gst_element_factory_make("rtph264pay", "payloader");
  data->udpsink = gst_element_factory_make("udpsink", "udpsink");

  // Create pipeline
  data->pipeline = gst_pipeline_new("test-pipeline");

  if (!data->pipeline || !data->source || !data->encoder || !data->payloader ||
      !data->udpsink) {
    g_printerr("Not all elements could be created.\n");
    return FALSE;
  }

  // Build the pipeline
  gst_bin_add_many(GST_BIN(data->pipeline), data->source, data->encoder,
                   data->payloader, data->udpsink, NULL);
  if (gst_element_link_many(data->source, data->encoder, data->payloader,
                            data->udpsink, NULL) != TRUE) {
    g_printerr("Elements could not be linked.\n");
    gst_object_unref(data->pipeline);
    return FALSE;
  }

  // Set caps on the source
  caps = gst_caps_from_string(
      "video/x-raw, width=640, height=480, framerate=30/1");
  g_object_set(data->source, "caps", caps, NULL);
  gst_caps_unref(caps);

  // Set UDP sink properties (replace with your multicast IP and port)
  g_object_set(data->udpsink, "host", "224.1.1.1", "port", 5004, NULL);

  // Start playing
  gst_element_set_state(data->pipeline, GST_STATE_PLAYING);

  // Get bus and add watch for messages
  bus = gst_element_get_bus(data->pipeline);
  gst_bus_add_watch(bus, (GstBusFunc)bus_call, data);
  gst_object_unref(bus);

  return TRUE;
}

// Function to get current bitrate from a pad
guint get_bitrate_from_pad(GstPad *pad) {
  GstQuery *query;
  gboolean success;
  guint bitrate = 0;

  // Create a new bitrate query
  query = gst_query_new_bitrate();

  // Execute the query on the pad
  success = gst_pad_query(pad, query);

  if (success) {
    // If the query succeeded, extract the bitrate
    gst_query_parse_bitrate(query, &bitrate);
  } else {
    g_print("Bitrate query failed\n");
  }

  // Free the query
  gst_query_unref(query);

  return bitrate;
}

void getbitrate(GstPad *pad) {
  // CustomData *data = (CustomData *)usrdata;
  guint bitrate;
  // Query the bitrate from the sink pad
  while (true) {
    bitrate = get_bitrate_from_pad(pad);
    g_print("Current bitrate: %u bps\n", bitrate);
  }
}

int main(int argc, char *argv[]) {
  CustomData data;

  guint bitrate;

  // Initialize CustomData and GStreamer
  memset(&data, 0, sizeof(data));
  data.loop = g_main_loop_new(NULL, FALSE);

  // Initialize GStreamer and create pipeline
  if (!init_gstreamer(&data)) {
    g_printerr("Failed to initialize GStreamer.\n");
    return -1;
  }

  // Get the sink pad of the udpsink element
  data.sink_pad = gst_element_get_static_pad(data.udpsink, "sink");

  std::thread t(getbitrate, data.sink_pad);

  // Run main loop
  g_print("Running...\n");
  g_main_loop_run(data.loop);
  t.join();
  // Clean up
  gst_element_set_state(data.pipeline, GST_STATE_NULL);
  gst_object_unref(data.pipeline);
  g_main_loop_unref(data.loop);

  return 0;
}
