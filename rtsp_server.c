#include <gst/gst.h>
// #include <gst/rtp/gstrtpbin.h>
#include <gst/rtp/gstrtcpbuffer.h>

#include <gst/rtsp-server/rtsp-server.h>
#include <unistd.h>
#include <stdlib.h>
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

void rtcp_callback(GstElement *src, GstBuffer *buf, gpointer data)
{
  g_warning("RTCP!!!");
}

GstElement *pipeline, *source, *enc, *h264p, *rtph264, *identity, *identity2, *rtcp_udp_src, *fakesink;
static GstElement *
custom_create_element (GstRTSPMediaFactory * factory, const GstRTSPUrl  *url)
{
     /* you can see at query string: */
     g_warning("CUsom\n");
     g_print("query is: %s\n", url->query);
     /* according to query create GstElement, for example: */
    source = gst_element_factory_make ("v4l2src", "source");
    enc = gst_element_factory_make("x264enc", "enc");
    h264p = gst_element_factory_make("h264parse", "h264p");
    rtph264 = gst_element_factory_make("rtph264pay", "pay0");
    identity = gst_element_factory_make("identity", NULL);
    identity2 = gst_element_factory_make("identity", NULL);
    rtcp_udp_src = gst_element_factory_make("udpsrc", NULL);
    fakesink = gst_element_factory_make("fakesink", NULL);
    g_object_set(G_OBJECT(enc), "tune", 0x00000004, "bitrate", 1000, NULL);
    g_object_set(G_OBJECT(rtcp_udp_src), "caps", gst_caps_from_string("application/x-rtcp"), NULL);
    pipeline = gst_pipeline_new ("test-pipeline");
    // if (!pipeline || !source || !sink) {
    // }
    g_signal_connect(identity, "handoff", G_CALLBACK(rtcp_callback), NULL);

  gst_bin_add_many (GST_BIN (pipeline), source, enc, h264p, rtph264, rtcp_udp_src, identity, identity2, NULL);
  gst_element_link_many (source, enc, h264p, rtph264, NULL);
  gst_element_link_many(rtcp_udp_src, identity, identity2, NULL);

    return pipeline;

}




#define DEFAULT_RTSP_PORT "8554"

static char *port = (char *) DEFAULT_RTSP_PORT;

static GOptionEntry entries[] = {
  {"port", 'p', 0, G_OPTION_ARG_STRING, &port,
      "Port to listen on (default: " DEFAULT_RTSP_PORT ")", "PORT"},
  {NULL}
};

static guint16
get_port_from_socket (GSocket * socket)
{
  guint16 port;
  GSocketAddress *sockaddr;
  GError *err;

  GST_DEBUG ("socket: %p", socket);
  sockaddr = g_socket_get_local_address (socket, &err);
  if (sockaddr == NULL || !G_IS_INET_SOCKET_ADDRESS (sockaddr)) {
    g_clear_object (&sockaddr);
    GST_ERROR ("failed to get sockaddr: %s", err->message);
    g_error_free (err);
    return 0;
  }

  port = g_inet_socket_address_get_port (G_INET_SOCKET_ADDRESS (sockaddr));
  g_object_unref (sockaddr);

  return port;
}

void process_rr_pkt(GstRTCPPacket* packet)
{
    guint64 rr_time_delta_ms;
    guint64 curr_time_ms;
    gfloat bandwidth;

    guint32 exthighestseq, jitter, lsr, dlsr;
    guint32 packet_interval;
    guint32 ssrc;
    gint32 packetslost;

    guint8 fractionlost;
    gfloat curr_buffer_occ;
    gfloat curr_rtt;

    gst_rtcp_packet_get_rb(packet, 0, &ssrc, &fractionlost,
                           &packetslost, &exthighestseq, &jitter, &lsr, &dlsr);
    g_warning("    ssrc          %llu", ssrc);
    g_warning("    highest   seq %llu", exthighestseq);
    g_warning("    jitter        %llu", jitter);
    g_warning("    fraction lost %llu", fractionlost);
    g_warning("    packet   lost %llu", packetslost);
    
}

GstPadProbeReturn buffer_callback(GstPad *pad,GstPadProbeInfo *info,gpointer user_data)
{
  g_warning("Got AN RTCP BUFFER BITCH!");

    GstBuffer* buf = GST_PAD_PROBE_INFO_BUFFER(info);
    if (buf != NULL) {    
      GstRTCPBuffer *rtcp_buffer = (GstRTCPBuffer*)malloc(sizeof(GstRTCPBuffer));
      rtcp_buffer->buffer = NULL;
      gst_rtcp_buffer_map(buf, GST_MAP_READ, rtcp_buffer);
      GstRTCPPacket *packet = (GstRTCPPacket*)malloc(sizeof(GstRTCPPacket));
      gst_rtcp_buffer_get_first_packet(rtcp_buffer, packet);
      process_rr_pkt(packet);
      
      GstRTCPType type;
      type = gst_rtcp_packet_get_type(packet);
      g_warning("pkt type %d size - %d", type, gst_buffer_get_size(buf));
      switch (type) {
      case GST_RTCP_TYPE_RR:
        g_warning("RR Pckt");

          break;
      case GST_RTCP_TYPE_SR:
        g_warning("SR pkt");
          break;
      default:
          break;
      }
    free(rtcp_buffer);
    free(packet);
    } else {
      g_warning("kiss my piss :(");    
    }
}

GstRTSPFilterResult
media_filter (GstRTSPSession *sess,
                             GstRTSPSessionMedia *session_media,
                             gpointer user_data)
{
  g_warning("media!");
  GstRTSPMedia* media = gst_rtsp_session_media_get_media (session_media);
  GstElement* e = gst_rtsp_media_get_element (media);
    GstElement* parent = (GstElement*)gst_object_get_parent(GST_OBJECT(e));
  if (parent!=NULL) {
    g_warning("got parent!");
    GList* list = GST_BIN_CHILDREN(parent);
    GList* l;
    int i = 0;
    GstElement* e;
    for (l = list; l != NULL; l = l->next)
    {
        e = l->data;
        char* str = gst_element_get_name(e);
        g_warning("element name = %s %d", str, i++);
        if (i == 19) {
          break;
        }
        // if ()
    }
    g_warning("rtpbinelement name = %s\n", gst_element_get_name(e));
    GList* pads = GST_ELEMENT_PADS(e);
    GstPad* p;
    i = 0;
     for (l = pads; l != NULL; l = l->next)
    {
        p = l->data;
        char* str = gst_pad_get_name(p);
        g_warning("rtpbinpad name = %s %d", str, i);
        if (i == 4){
          break;
        }
        i++;
        // if ()
    }

    gst_pad_add_probe (p,GST_PAD_PROBE_TYPE_BUFFER,G_CALLBACK(buffer_callback),NULL, NULL);
  } else {
    g_warning("fuckme");
  }

//   g_warning("Media streams - %d", gst_rtsp_media_n_streams (media));
//   GstRTSPStream* stream = gst_rtsp_media0_get_stream (media,0);
//   if (stream != NULL) {
//     // g_warning("BABYEEE");
//     GSocket *mysocket1= gst_rtsp_stream_get_rtcp_socket (stream, G_SOCKET_FAMILY_IPV4);
//     GSocket *mysocket2= gst_rtsp_stream_get_rtp_socket (stream, G_SOCKET_FAMILY_IPV4);

//     if (mysocket1 != NULL) {
//       guint16 port1 = get_port_from_socket(mysocket1);
//       guint16 port2 = get_port_from_socket(mysocket2);
//       g_object_set(rtcp_udp_src, "port", port1, NULL);
//       g_warning("KNACK22 + %d %d", port1, port2);
//     } else {
//       g_warning("fuck");
//     }
//   } else {
//     g_warning("Oh dear");
//   }
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
  TestRTSPMediaFactory *factory;
  GstRTSPMediaFactory* my_factory;
  GOptionContext *optctx;
  GError *error = NULL;
  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);

  /* create a server instance */
  server = gst_rtsp_server_new ();
  g_object_set (server, "service", port, NULL);
  gst_rtsp_server_set_address(server, "172.17.0.2");

  mounts = gst_rtsp_server_get_mount_points (server);


  my_factory = gst_rtsp_media_factory_new ();
  GST_RTSP_MEDIA_FACTORY_GET_CLASS(my_factory)->create_element = custom_create_element;
  //  factory = g_object_new(TEST_TYPE_RTSP_MEDIA_FACTORY, NULL);
   
    // factory = gst_rtsp_media_factory_new ();
  // gst_rtsp_media_factory_set_launch (factory, "videotestsrc ! video/x-raw,width=352,height=288,framerate=15/1 ! "
  //     "x264enc ! rtph264pay name=pay0 pt=96 ");
//  gst_rtsp_mount_points_add_factory (mounts, "/test", GST_RTSP_MEDIA_FACTORY(factory));
  gst_rtsp_mount_points_add_factory (mounts, "/test", my_factory);

  g_object_unref (mounts);

  gst_rtsp_server_attach (server, NULL);
  g_timeout_add_seconds (2, (GSourceFunc) timeout, server);


  /* start serving */
  // GstRTSPSessionPool* pool = gst_rtsp_server_get_session_pool(server);
  g_print ("custom Stream ready at rtsp://172.17.0.2:%s/test\n", port);
  // g_signal_connect(server, "client-connected", G_CALLBACK(client_connect_callback), pool);
  g_main_loop_run (loop);

  return 0;
}
