 
#include <gst/gst.h>

static gboolean link_elements_with_filter (GstElement *element1, GstElement *element2, GstCaps* caps)
{
  gboolean link_ok;

  link_ok = gst_element_link_filtered (element1, element2, caps);
  gst_caps_unref (caps);

  if (!link_ok) {
    g_warning ("Failed to link element1 and element2!");
  }

  return link_ok;
}


int main(int argc, char *argv[]) {
  GstElement *pipeline, *source, *sink, *enc, *rtpjpgpay;
  GstElement *cfilter;
  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;

  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  /* Create the elements */
  source = gst_element_factory_make ("v4l2src", "source");
  enc = gst_element_factory_make("jpegenc", "enc");
  rtpjpgpay = gst_element_factory_make("rtpjpegpay", "rtpjpgpay");
  sink = gst_element_factory_make ("udpsink", "sink");
  // cfilter = gst_caps_new_simple ("video/x-raw",
  //         "format", G_TYPE_STRING, "I420",
  //         "width", G_TYPE_INT, 384,
  //         "height", G_TYPE_INT, 288,
  //         NULL);
  cfilter = gst_element_factory_make ("capsfilter", NULL);
  gst_util_set_object_arg (G_OBJECT (cfilter), "caps",
    "video/x-raw, width=320, height=240, "
    "format={ I420, YV12, YUY2, UYVY, AYUV, Y41B, Y42B, "
    "YVYU, Y444, v210, v216, NV12, NV21, UYVP, A420, YUV9, YVU9, IYU1 }");


  g_object_set(G_OBJECT(sink), "host", "127.0.0.1", NULL);
  g_object_set(G_OBJECT(sink), "port", 5200, NULL);

  /* Create the empty pipeline */
  pipeline = gst_pipeline_new ("test-pipeline");

  if (!pipeline || !source || !sink || !cfilter) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }

  /* Build the pipeline */
  gst_bin_add_many (GST_BIN (pipeline), source, cfilter, enc, rtpjpgpay, sink, NULL);
  // link_elements_with_filter(source, enc, cfilter);
  if (gst_element_link_many (source, cfilter, enc, rtpjpgpay, sink, NULL) != TRUE) {
    g_printerr ("Elements could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }

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