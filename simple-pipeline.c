#include <gst/gst.h>

int main(int argc, char *argv[]) {
  GstElement *pipeline, *source, *sink;
  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;

  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  /* Create the elements */
  source = gst_element_factory_make ("videotestsrc", "source");
  GstElement* videoconvert = gst_element_factory_make("videoconvert", NULL);
  GstElement* identity = gst_element_factory_make("identity", NULL);
  sink = gst_element_factory_make ("autovideosink", "sink");

  /* Create the empty pipeline */
  pipeline = gst_pipeline_new ("test-pipeline");

  if (!pipeline || !source || !sink) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }

  gst_bin_add_many (GST_BIN (pipeline), source, videoconvert, identity, sink, NULL);
  if (gst_element_link_many (source, videoconvert, identity, sink, NULL) != TRUE) {
    g_printerr ("Elements could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }
    GstIterator *it = gst_bin_iterate_elements (GST_BIN (pipeline));
    GValue item = G_VALUE_INIT;
    int done = FALSE;
    int i = 0;
    while (!done) {
        i++;
    switch (gst_iterator_next (it, &item)) {
    case GST_ITERATOR_OK:
        g_value_reset (&item);
        break;
    case GST_ITERATOR_RESYNC:
        gst_iterator_resync (it);
        break;
    case GST_ITERATOR_ERROR:
        done = TRUE;
        break;
    case GST_ITERATOR_DONE:
        done = TRUE;
        break;
    }
    }
    g_value_unset (&item);
    gst_iterator_free (it);
    g_warning("ele cnt %d", i);


  /* Build the pipeline */

  /* Modify the source's properties */
  g_object_set (source, "pattern", 0, NULL);

  /* Start playing */
//   ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (pipeline);
    return -1;
  }

  /* Wait until error or EOS */
  bus = gst_element_get_bus (pipeline);
  msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);


  /* Parse message */
//   if (msg != NULL) {
//     GError *err;
//     gchar *debug_info;

//     switch (GST_MESSAGE_TYPE (msg)) {
//       case GST_MESSAGE_ERROR:
//         gst_message_parse_error (msg, &err, &debug_info);
//         g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
//         g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
//         g_clear_error (&err);
//         g_free (debug_info);
//         break;
//       case GST_MESSAGE_EOS:
//         g_print ("End-Of-Stream reached.\n");
//         break;
//       default:
//         /* We should not reach here because we only asked for ERRORs and EOS */
//         g_printerr ("Unexpected message received.\n");
//         break;
//     }
//     gst_message_unref (msg);
//   }

  /* Free resources */
  gst_object_unref (bus);
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);
  return 0;
}