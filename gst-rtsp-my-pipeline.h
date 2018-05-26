#ifndef __GST_RTSP_MY_PIPELINE_H__
#define __GST_RTSP_MY_PIPELINE_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define GST_TYPE_RTSP_MY_PIPELINE            (gst_rtsp_my_pipeline_get_type ())
#define GST_RTSP_MY_PIPELINE(o)              (G_TYPE_CHECK_INSTANCE_CAST  ((o), GST_TYPE_RTSP_MY_PIPELINE, GstRTSPMyPipeline))
#define GST_IS_RTSP_MY_PIPELINE(o)           (G_TYPE_CHECK_INSTANCE_TYPE  ((o), GST_TYPE_RTSP_MY_PIPELINE))
#define GST_RTSP_MY_PIPELINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_RTSP_MY_PIPELINE, GstRTSPMyPipelineClass))
#define GST_IS_RTSP_MY_PIPELINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_RTSP_MY_PIPELINE))
#define GST_RTSP_MY_PIPELINE_GET_CLASS(o)    (G_TYPE_INSTANCE_GET_CLASS ((o), GST_TYPE_RTSP_MY_PIPELINE, GstRTSPMyPipelineClass))
#define GST_RTSP_MY_PIPELINE_GET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GST_TYPE_RTSP_MY_PIPELINE, GstRTSPMyPipelinePrivate))

typedef struct _GstRTSPMyPipeline        GstRTSPMyPipeline;
typedef struct _GstRTSPMyPipelineClass   GstRTSPMyPipelineClass;

GType              gst_rtsp_my_pipeline_get_type (void) G_GNUC_CONST;
GstRTSPMyPipeline *gst_rtsp_my_pipeline_new (void);

G_END_DECLS

#endif /* __GST_RTSP_MY_PIPELINE_H__ */
