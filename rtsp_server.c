#include <gst/gst.h>

#include <gst/rtsp-server/rtsp-server.h>

G_BEGIN_DECLS

#define TEST_TYPE_RTSP_MEDIA_FACTORY (test_rtsp_media_factory_get_type())
// #define GST_TEST_RTSP_MEDIA_FACTORY(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), test_rtsp_media_factory_get_type, TestRTSPMediaFactory))

// G_DECLARE_FINAL_TYPE(TestRTSPMediaFactory, test_rtsp_media_factory, test_rtsp, media_factory, GST_TYPE_RTSP_MEDIA_FACTORY)

G_END_DECLS

typedef struct TestRTSPMediaFactoryClass TestRTSPMediaFactoryClass;
typedef struct TestRTSPMediaFactory TestRTSPMediaFactory;

struct TestRTSPMediaFactoryClass
{
    GstRTSPMediaFactoryClass parent;
};

struct TestRTSPMediaFactory
{
     GstRTSPMediaFactory parent;
};


static GstElement * custom_create_element(GstRTSPMediaFactory      *factory, const GstRTSPUrl *url);
GType                 test_rtsp_media_factory_get_type   (void);


G_DEFINE_TYPE (TestRTSPMediaFactory, test_rtsp_media_factory, GST_TYPE_RTSP_MEDIA_FACTORY);

static void
test_rtsp_media_factory_class_init (TestRTSPMediaFactoryClass * test_klass)
{
  g_print("Makeing custom");
   GstRTSPMediaFactoryClass *klass = (GstRTSPMediaFactoryClass *) (test_klass);
   klass->create_element = custom_create_element;
}

static void
test_rtsp_media_factory_init (TestRTSPMediaFactory * media)
{
}

static GstElement *
custom_create_element (GstRTSPMediaFactory * factory, const GstRTSPUrl  *url)
{
     /* you can see at query string: */
     g_print("query is: %s\n", url->query);
     /* according to query create GstElement, for example: */
     GstElement *pipeline, *source, *sink, *enc, *h264p, *rtph264;
    source = gst_element_factory_make ("v4l2src", "source");
    enc = gst_element_factory_make("x264enc", "enc");
    h264p = gst_element_factory_make("h264parse", "h264p");
    rtph264 = gst_element_factory_make("rtph264pay", "pay0");
    g_object_set(G_OBJECT(enc), "tune", 0x00000004, "bitrate", 1000, NULL);
  pipeline = gst_pipeline_new ("test-pipeline");
  if (!pipeline || !source || !sink) {
    g_printerr ("Not all elements could be created.\n");
    return NULL;
  }
  gst_bin_add_many (GST_BIN (pipeline), source, enc, h264p, rtph264, NULL);
  gst_element_link_many (source, enc, h264p, rtph264, NULL);

    return pipeline;

}




#define DEFAULT_RTSP_PORT "8554"

static char *port = (char *) DEFAULT_RTSP_PORT;

static GOptionEntry entries[] = {
  {"port", 'p', 0, G_OPTION_ARG_STRING, &port,
      "Port to listen on (default: " DEFAULT_RTSP_PORT ")", "PORT"},
  {NULL}
};

int
main (int argc, char *argv[])
{
  GMainLoop *loop;
  GstRTSPServer *server;
  GstRTSPMountPoints *mounts;
  TestRTSPMediaFactory *factory;
  GOptionContext *optctx;
  GError *error = NULL;

  optctx = g_option_context_new ("<launch line> - Test RTSP Server, Launch\n\n"
      "Example: \"( videotestsrc ! x264enc ! rtph264pay name=pay0 pt=96 )\"");
  g_option_context_add_main_entries (optctx, entries, NULL);
  g_option_context_add_group (optctx, gst_init_get_option_group ());
  if (!g_option_context_parse (optctx, &argc, &argv, &error)) {
    g_printerr ("Error parsing options: %s\n", error->message);
    g_option_context_free (optctx);
    g_clear_error (&error);
    return -1;
  }
  g_option_context_free (optctx);

  loop = g_main_loop_new (NULL, FALSE);

  /* create a server instance */
  server = gst_rtsp_server_new ();
  g_object_set (server, "service", port, NULL);

  /* get the mount points for this server, every server has a default object
   * that be used to map uri mount points to media factories */
  mounts = gst_rtsp_server_get_mount_points (server);

  /* make a media factory for a test stream. The default media factory can use
   * gst-launch syntax to create pipelines.
   * any launch line works as long as it contains elements named pay%d. Each
   * element with pay%d names will be a stream */
   factory = g_object_new(TEST_TYPE_RTSP_MEDIA_FACTORY, NULL);
  // gst_rtsp_media_factory_set_launch (factory, argv[1]);

  /* attach the test factory to the /test url */
  gst_rtsp_mount_points_add_factory (mounts, "/test", GST_RTSP_MEDIA_FACTORY(factory));

  /* don't need the ref to the mapper anymore */
  g_object_unref (mounts);

  /* attach the server to the default maincontext */
  gst_rtsp_server_attach (server, NULL);

  /* start serving */
  g_print ("stream ready at rtsp://127.0.0.1:%s/test\n", port);
  g_main_loop_run (loop);

  return 0;
}
