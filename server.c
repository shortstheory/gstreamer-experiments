 
#include <gst/gst.h>

int main(int argc, char *argv[]) {
  GstElement *pipeline, *source, *sink, *enc, *h264p, *rtph264;
  GstElement *filter0;//, *filter1;
  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;

  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  /* Create the elements */
  source = gst_element_factory_make ("v4l2src", "source");
  enc = gst_element_factory_make("x264enc", "enc");
  h264p = gst_element_factory_make("h264parse", "h264p");
  rtph264 = gst_element_factory_make("rtph264pay", "rtph264");
  sink = gst_element_factory_make ("udpsink", "sink");

  filter0 = gst_element_factory_make ("capsfilter", NULL);
  gst_util_set_object_arg (G_OBJECT (filter0), "caps",
    "video/x-raw, width=1280, height=720, framerate=15/1");

  g_object_set(G_OBJECT(sink), "host", "127.0.0.1", "port", 5000, NULL);
  g_object_set(G_OBJECT(enc), "tune", 0x00000004, "bitrate", 1000, NULL);

  /* Create the empty pipeline */
  pipeline = gst_pipeline_new ("test-pipeline");

  if (!pipeline || !source || !sink || !filter0) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }

  /* Build the pipeline */
  gst_bin_add_many (GST_BIN (pipeline), source, enc, h264p, rtph264, sink, NULL);
  gst_element_link(source, enc);
  gst_element_link(enc, h264p);
  gst_element_link(h264p, rtph264);
  gst_element_link(rtph264, sink);
  // if (gst_element_link_many (source, filter0, enc, h264p, rtph264, sink, NULL) != TRUE) {
  //   g_printerr ("Elements could not be linked.\n");
  //   gst_object_unref (pipeline);
  //   return -1;
  // }

  /* Modify the source's properties */
  // g_object_set (source, "pattern", 0, NULL);

  /* Start playing */
  ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (pipeline);
    return -1;
  }

  /* Wait until error or EOS */
  bus = gst_element_get_bus (pipeline);
  msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

  /* Parse message */
  if (msg != NULL) {
    GError *err;
    gchar *debug_info;

    switch (GST_MESSAGE_TYPE (msg)) {
      case GST_MESSAGE_ERROR:
        gst_message_parse_error (msg, &err, &debug_info);
        g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
        g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
        g_clear_error (&err);
        g_free (debug_info);
        break;
      case GST_MESSAGE_EOS:
        g_print ("End-Of-Stream reached.\n");
        break;
      default:
        /* We should not reach here because we only asked for ERRORs and EOS */
        g_printerr ("Unexpected message received.\n");
        break;
    }
    gst_message_unref (msg);
  }

  /* Free resources */
  gst_object_unref (bus);
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);
  return 0;
}