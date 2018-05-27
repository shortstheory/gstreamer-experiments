#include "gst-rtsp-my-pipeline.h"

typedef struct _GstRTSPMyPipelinePrivate GstRTSPMyPipelinePrivate;

struct _GstRTSPMyPipeline {
  GstRTSPMediaFactory parent_instance;
  GstRTSPMyPipelinePrivate *priv;
};

struct _GstRTSPMyPipelineClass {
  GstRTSPMediaFactoryClass parent_class;
};

struct _GstRTSPMyPipelinePrivate
{
  /* TODO: Add members here. */
};

G_DEFINE_TYPE (GstRTSPMyPipeline, gst_rtsp_my_pipeline, GST_TYPE_RTSP_MEDIA_FACTORY);

static GstElement *custom_create_element (GstRTSPMediaFactory *
    factory, const GstRTSPUrl * url);

static void
gst_rtsp_my_pipeline_class_init (GstRTSPMyPipelineClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
   klass->create_element = custom_create_element;
  g_type_class_add_private (klass, sizeof (GstRTSPMyPipelinePrivate));
}


static void
gst_rtsp_my_pipeline_init (GstRTSPMyPipeline *self)
{
  self->priv = GST_RTSP_MY_PIPELINE_GET_PRIVATE (self);
}

GstRTSPMyPipeline *
gst_rtsp_my_pipeline_new (void)
{
  return g_object_new (GST_TYPE_RTSP_MY_PIPELINE, NULL);
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