#include <gst/gst.h>

#include <gst/rtsp-server/rtsp-server.h>
#include <unistd.h>

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



GstRTSPFilterResult
media_filter (GstRTSPSession *sess,
                             GstRTSPSessionMedia *session_media,
                             gpointer user_data)
{
  g_warning("media!");
  GstRTSPMedia* media = gst_rtsp_session_media_get_media (session_media);
  g_warning("Media streams - %d", gst_rtsp_media_n_streams (media));
  return GST_RTSP_FILTER_KEEP;
}
GstRTSPFilterResult filter_func(GstRTSPSessionPool *pool,
                                 GstRTSPSession *session,
                                 gpointer user_data)
{
  int matchChars= 0;
  //  GstRTSPSessionMedia *media = gst_rtsp_session_get_media(session, "rtsp://127.0.0.1:8554/test", &matchChars);
  g_warning("connection!");
  gst_rtsp_session_filter (session, media_filter, NULL);
  return GST_RTSP_FILTER_KEEP;
}
void
client_connect_callback (GstRTSPServer *server,
               GstRTSPClient *client,
               gpointer       user_data)
{
  guint a = gst_rtsp_session_pool_get_n_sessions((GstRTSPSessionPool*)user_data);
  // if (pool != NULL) {
  g_warning("clientnew connected!! %u", a);
  // } 
}

static gboolean
timeout (GstRTSPServer * server)
{
    GstRTSPSessionPool *pool;

  pool = gst_rtsp_server_get_session_pool (server);
  g_warning("Doing timeout! %d", gst_rtsp_session_pool_get_n_sessions (pool));
  GList *session_list = gst_rtsp_session_pool_filter (pool, filter_func, NULL);

  return TRUE;
}

int
main (int argc, char *argv[])
{
  GMainLoop *loop;
  GstRTSPServer *server;
  GstRTSPMountPoints *mounts;
  GstRTSPMediaFactory *factory;
  GOptionContext *optctx;
  GError *error = NULL;
  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);

  /* create a server instance */
  server = gst_rtsp_server_new ();
  g_object_set (server, "service", port, NULL);
  // gst_rtsp_server_set_address(server, "172.17.0.2");

  mounts = gst_rtsp_server_get_mount_points (server);

  //  factory = g_object_new(TEST_TYPE_RTSP_MEDIA_FACTORY, NULL);
    factory = gst_rtsp_media_factory_new ();
  gst_rtsp_media_factory_set_launch (factory, "videotestsrc ! video/x-raw,width=352,height=288,framerate=15/1 ! "
      "x264enc ! rtph264pay name=pay0 pt=96 ");

  gst_rtsp_mount_points_add_factory (mounts, "/test", factory);

  g_object_unref (mounts);

  gst_rtsp_server_attach (server, NULL);
  g_timeout_add_seconds (2, (GSourceFunc) timeout, server);


  /* start serving */
  // GstRTSPSessionPool* pool = gst_rtsp_server_get_session_pool(server);
  g_print ("Stream ready at rtsp://localhost:%s/test\n", port);
  // g_signal_connect(server, "client-connected", G_CALLBACK(client_connect_callback), pool);
  g_main_loop_run (loop);

  return 0;
}
