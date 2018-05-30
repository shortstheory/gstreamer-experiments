#include <gst/gst.h>

int videoport = 5000;
int rtcpport = 5005;

int main(int argc, char *argv[])
{
    GstElement *pipeline, *source, *sink, *enc, *h264p, *rtph264, *bin, *identity;
    GstElement *filter0;
    GstElement *rtcpsrc;

    GstBus *bus;
    GstMessage *msg;
    gst_init(&argc, &argv);
    source = gst_element_factory_make ("v4l2src", "source");
    enc = gst_element_factory_make("x264enc", "enc");
    h264p = gst_element_factory_make("h264parse", "h264p");
    rtph264 = gst_element_factory_make("rtph264pay", "rtph264");
    sink = gst_element_factory_make ("udpsink", "sink");
    bin = gst_element_factory_make("rtpbin", "bin");
    filter0 = gst_element_factory_make ("capsfilter", NULL);
    identity = gst_element_factory_make("identity", NULL);

    rtcpsrc = gst_element_factory_make("udpsrc", "rtcpsrc");

    g_object_set(G_OBJECT (rtcpsrc), "caps", gst_caps_from_string("application/x-rtcp"), "port", 5005, NULL);

    g_object_set(G_OBJECT(sink), "host", "192.168.1.7", "port", videoport, NULL);
    gst_util_set_object_arg (G_OBJECT (filter0), "caps", "video/x-raw, width=640, height=360");

    // g_object_set(G_OBJECT(sink), "host", "127.0.0.1", "port", 5000, NULL);
    g_object_set(G_OBJECT(enc), "tune", 0x00000004, "bitrate", 1000, NULL);
    g_object_set(G_OBJECT(bin), "latency", 0, NULL);

    pipeline = gst_pipeline_new ("test-pipeline");
    // gst_bin_add_many (GST_BIN (pipeline), source, rtcpsrc, identity, enc, h264p, rtph264, bin, sink, NULL);
    //jumbling order of elements doesn't matter here
    gst_bin_add_many (GST_BIN (pipeline), enc, bin, rtph264, sink, h264p, source, identity, rtcpsrc, NULL);

    gst_element_link_many(source, enc, h264p, rtph264, NULL);
    gst_element_link(rtcpsrc, identity);

    GstPad* videosinkpad, *videosrcpad;
    GstPad* rtcp_pad = gst_element_get_request_pad(bin, "recv_rtcp_sink_%u");
    videosinkpad = gst_element_get_request_pad(bin, "send_rtp_sink_%u");
    // videosrcpad = gst_element_get_request_pad(bin, "send_rtp_src_%u");

    gst_pad_link(gst_element_get_static_pad(identity,"src"),rtcp_pad);
    gst_pad_link(gst_element_get_static_pad(rtph264,"src"), videosinkpad);
    gst_element_link(bin, sink);

    gst_element_set_state (pipeline, GST_STATE_PLAYING);
    // gst_pipeline_set_delay(pipeline, 2000);
    // g_print("%lu", gst_pipeline_get_delay(pipeline));
    bus = gst_element_get_bus (pipeline);
    msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);


    // g_print("%s", gst_pad_get_name(videosinkpad));

    return 0;
}