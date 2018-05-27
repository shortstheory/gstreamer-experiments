#include <gst/gst.h>

#include <gst/rtsp-server/rtsp-server.h>

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


G_DEFINE_TYPE (TestRTSPMediaFactory, test_rtsp_media_factory, GST_TYPE_RTSP_MEDIA_FACTORY);

static void
test_rtsp_media_factory_class_init (TestRTSPMediaFactoryClass * test_klass)
{
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
     printf("query is: %s\n", url->query);
     /* according to query create GstElement, for example: */
     GstElement *element;
     GError *error = NULL;

     element = gst_parse_launch ("( videotestsrc ! x264enc ! rtph264pay name=pay0 pt=96 )",
                          &error);
     return element;
}


int main (int argc, char *argv[])
{
   ...  
   GstRTSPMediaFactory *factory;
   factory = g_object_new(TEST_TYPE_RTSP_MEDIA_FACTORY, NULL);

   ...
   g_main_loop_run (loop);

   return 0;
}