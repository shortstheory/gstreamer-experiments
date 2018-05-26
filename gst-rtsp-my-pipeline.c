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

static void
gst_rtsp_my_pipeline_class_init (GstRTSPMyPipelineClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

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
