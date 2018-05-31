#include <gst/gst.h>
#include <gst/rtp/gstrtcpbuffer.h>
#include <stdio.h>
int videoport = 5000;
int rtcpport = 5005;
int rtcpsinkport = 5001;
int bitrate = 1000;
// char recv_addr[] = "192.168.1.7";
char recv_addr[] = "127.0.0.1";

static void rtcp_recv_callback(GstElement *src, GstBuffer *buf, gpointer data)
{
    g_warning("rtcp_received");
    gboolean res = gst_rtcp_buffer_validate_data(data, 4);
    if (res == TRUE) {
        g_warning("Valid buffer");
    } else {
        g_warning("Bad buf");
    }
}


int main(int argc, char *argv[])
{
    GstElement *pipeline, *source, *sink, *enc, *h264p, *rtph264, *bin, *identity;
    GstElement *filter0;
    GstElement *rtcpsrc;
    GstElement *rtcpsink;

    GstBus *bus;
    GstMessage *msg;
    gst_init(&argc, &argv);

    GstCaps *caps, *rtcpcaps;

    caps = gst_caps_new_simple ("video/x-raw",
                                "width", G_TYPE_INT, 640,
                                "height", G_TYPE_INT, 360,
                                "framerate", GST_TYPE_FRACTION, 30, 1,
                                NULL);
    rtcpcaps = gst_caps_new_simple("application/x-rtcp", NULL); //might not be needed

    source = gst_element_factory_make ("v4l2src", "source");
    enc = gst_element_factory_make("x264enc", "enc");
    h264p = gst_element_factory_make("h264parse", "h264p");
    rtph264 = gst_element_factory_make("rtph264pay", "rtph264");
    sink = gst_element_factory_make ("udpsink", "sink");
    bin = gst_element_factory_make("rtpbin", "bin");
    filter0 = gst_element_factory_make ("capsfilter", NULL);
    identity = gst_element_factory_make("identity", NULL);
    rtcpsink = gst_element_factory_make("udpsink", "rtcpsink");
    rtcpsrc = gst_element_factory_make("udpsrc", "rtcpsrc");
    
    g_object_set(G_OBJECT (source), "device", "/dev/video1", NULL);
    g_object_set(G_OBJECT (rtcpsrc), "caps", gst_caps_from_string("application/x-rtcp"), "port", rtcpport, NULL);
    g_object_set(G_OBJECT(rtcpsink), "host", recv_addr, "port", rtcpsinkport, NULL);
    g_object_set(G_OBJECT(sink), "host", recv_addr, "port", videoport, NULL);
    g_object_set(G_OBJECT(enc), "tune", 0x00000004, "bitrate", bitrate, NULL);
    g_object_set(G_OBJECT(bin), "latency", 0, NULL);

    pipeline = gst_pipeline_new ("test-pipeline");
    gst_bin_add_many (GST_BIN (pipeline), enc, bin, rtph264, sink, h264p, source, identity, rtcpsrc, rtcpsink, NULL);
    int ret = gst_element_link_filtered(source, enc, caps);
    gst_element_link(enc, h264p);
    gst_element_link(h264p, rtph264);
    gst_element_link(rtcpsrc, identity);

    GstPad* videosinkpad = gst_element_get_request_pad(bin, "send_rtp_sink_%u");
    GstPad* rtcp_pad = gst_element_get_request_pad(bin, "recv_rtcp_sink_%u");
    GstPad* rtcp_src_pad = gst_element_get_request_pad(bin, "send_rtcp_src_%u");

    gst_pad_link(gst_element_get_static_pad(identity,"src"),rtcp_pad);
    gst_pad_link(gst_element_get_static_pad(rtph264,"src"), videosinkpad);
    gst_pad_link(rtcp_src_pad, gst_element_get_static_pad(rtcpsink,"sink"));
    gst_element_link(bin, sink);
    g_signal_connect (identity, "handoff", G_CALLBACK (rtcp_recv_callback), NULL);
    printf("Starting pipeline");
    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    bus = gst_element_get_bus (pipeline);
    msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);


    // g_print("%s", gst_pad_get_name(videosinkpad));

    return 0;
}