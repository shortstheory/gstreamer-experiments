#include <gst/gst.h>
#include <stdio.h>
#include <stdlib.h>
#include <gst/base/gstbasesink.h>
#include <gst/rtp/gstrtcpbuffer.h>
#include <time.h>
int videoport = 5000;
int rtcpport = 5005;
int rtcpsinkport = 5001;
int bitrate = 1000000;
int bitrate2 = 500;
gboolean switchvalue = TRUE;
// char recv_addr[] = "192.168.1.7";
char recv_addr[] = "127.0.0.1";

GstElement *pipeline, *source, *sink, *enc, *h264p, *rtph264, *bin, *identity, *senderidentity;
static gboolean
process_rtcp_packet(GstRTCPPacket *packet){
	guint32 ssrc, rtptime, packet_count, octet_count;
	guint64 ntptime;
	guint count, i;

	count = gst_rtcp_packet_get_rb_count(packet);
	g_debug("    count         %d", count);
	for (i=0; i<count; i++) {
		guint32 exthighestseq, jitter, lsr, dlsr;
		guint8 fractionlost;
		gint32 packetslost;

		gst_rtcp_packet_get_rb(packet, i, &ssrc, &fractionlost,
				&packetslost, &exthighestseq, &jitter, &lsr, &dlsr);

		// g_warning("    block         %llu", i);
		// g_warning("    ssrc          %llu", ssrc);
		// g_warning("    highest   seq %llu", exthighestseq);
		// g_warning("    jitter        %llu", jitter);
		// g_warning("    fraction lost %llu", fractionlost);
		// g_warning("    packet   lost %llu", packetslost);
        g_warning("lsr %llu", lsr>>16);
	}

	//g_debug("Received rtcp packet");

	return TRUE;
}

void process_sender_packet(GstRTCPPacket *packet)
{
    guint32 ssrc, rtptime, packet_count, octet_count;
    guint64 ntptime;
    gst_rtcp_packet_sr_get_sender_info(packet, &ssrc, &ntptime, &rtptime, &packet_count, &octet_count);
    // g_warning("Sender report");
    ntptime = ntptime >> 32;
    ntptime = (ntptime & 0x0000FFFF);
    // g_warning("ssrc %llu, ntptime %llu, rtptime %llu, packetcount %llu", ssrc, ntptime, rtptime, packet_count);
}

static void rtcp_recv_callback(GstElement *src, GstBuffer *buf, gpointer data)
{
    // g_warning("rtcp_received %d", gst_rtcp_buffer_get_packet_count(buf));
    GstRTCPBuffer *rtcpbuf = (GstRTCPBuffer*)malloc(sizeof(GstRTCPBuffer));
    rtcpbuf->buffer = NULL;
    // rtcpbuf->map = GST_MAP_INFO_INIT;
    gst_rtcp_buffer_map(buf, GST_MAP_READ, rtcpbuf);
    GstRTCPPacket *packet = (GstRTCPPacket*)malloc(sizeof(GstRTCPPacket));
    gboolean more = gst_rtcp_buffer_get_first_packet(rtcpbuf, packet);
	while (more) {
		GstRTCPType type;
		type = gst_rtcp_packet_get_type(packet);
        // g_warning("pkt type %d", type);
		switch (type) {
		case GST_RTCP_TYPE_RR:
            process_rtcp_packet(packet);
            // if (switchvalue) {
            //     g_warning("Inc bitrate");
            //     g_object_set(G_OBJECT(enc), "bitrate", bitrate, NULL);
            //     switchvalue = FALSE;
            // } else {
            //     g_warning("dec bitrate");
            //     g_object_set(G_OBJECT(enc), "bitrate", bitrate2, NULL);
            //     switchvalue = TRUE;
            // }
            // send_event_to_encoder(venc, &rtcp_pkt);
            break;
        case GST_RTCP_TYPE_SR:
            process_sender_packet(packet);
			break;
		default:
			g_debug("Other types");
			break;
		}
		more = gst_rtcp_packet_move_to_next(packet);
	}

}

static gboolean
gst_my_filter_sink_event (GstPad    *pad,
                  GstObject *parent,
                  GstEvent  *event)
{
    GstEventType type = GST_EVENT_TYPE(event);
    g_warning("event %d", type);
    return gst_pad_event_default (pad, parent, event);
    // return gst_pad_push_event (filter->srcpad, event);
//   gboolean ret;
//     g_warning("got event");
//   switch (GST_EVENT_TYPE (event)) {
//     case GST_EVENT_CAPS:
//       /* we should handle the format here */

//       /* push the event downstream */
//     //   ret = gst_pad_push_event (filter->srcpad, event);
//       break;
//     case GST_EVENT_EOS:
//       /* end-of-stream, we should close down all stream leftovers here */
//     //   gst_my_filter_stop_processing (filter);

//       ret = gst_pad_event_default (pad, parent, event);
//       break;
//     default:
//       /* just call the default handler */
//       ret = gst_pad_event_default (pad, parent, event);
//       break;
//   }
//   return ret;
}



int main(int argc, char *argv[])
{
    GstElement *filter0;
    GstElement *rtcpsrc;
    GstElement *rtcpsink;
    GstElement *enc_ident;
    // GstElement *identity;
    g_warning(" Hi %llu", time(NULL)+2208988800);

    GstBus *bus;
    GstMessage *msg;
    gst_init(&argc, &argv);

    GstCaps *caps, *rtcpcaps;

    caps = gst_caps_new_simple ("video/x-raw",
                                "width", G_TYPE_INT, 1280,
                                "height", G_TYPE_INT, 720,
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
    enc_ident = gst_element_factory_make("identity", "encod");
    identity = gst_element_factory_make("identity", NULL);
    senderidentity = gst_element_factory_make("identity", NULL);
    rtcpsink = gst_element_factory_make("udpsink", "rtcpsink");
    rtcpsrc = gst_element_factory_make("udpsrc", "rtcpsrc");
    
    g_object_set(G_OBJECT (source), "device", "/dev/video0", NULL);
    g_object_set(G_OBJECT(enc_ident), "qos", TRUE, NULL);
    g_object_set(G_OBJECT (rtcpsrc), "caps", gst_caps_from_string("application/x-rtcp"), "port", rtcpport, NULL);
    g_object_set(G_OBJECT(rtcpsink), "host", recv_addr, "port", rtcpsinkport, NULL);
    g_object_set(G_OBJECT(sink), "host", recv_addr, "port", videoport, NULL);//"qos", TRUE, NULL);
    gchar* strval;
    gboolean boolv;
    g_object_get(sink, "qos", &boolv);
    g_warning("V val: %d", boolv);
    g_object_set(G_OBJECT(enc), "tune", 0x00000004, "bitrate", bitrate, NULL);
    g_object_set(G_OBJECT(bin), "latency", 0, NULL);
    pipeline = gst_pipeline_new ("test-pipeline");
    gst_bin_add_many (GST_BIN (pipeline), enc, bin, rtph264, enc_ident, sink, h264p, source, identity, rtcpsrc, senderidentity, rtcpsink, NULL);
    int ret = gst_element_link_filtered(source, enc, caps);
    gst_element_link_many(enc, h264p, NULL);
    gst_element_link(h264p, rtph264);
    gst_element_link(rtcpsrc, identity);

    GstPad* videosinkpad = gst_element_get_request_pad(bin, "send_rtp_sink_%u");
    GstPad* rtcp_pad = gst_element_get_request_pad(bin, "recv_rtcp_sink_%u");
    GstPad* rtcp_src_pad = gst_element_get_request_pad(bin, "send_rtcp_src_%u");
    // gst_pad_set_event_function_full(gst_element_get_static_pad, "sink", &myfync,NULL,NULL);
    gst_pad_link(gst_element_get_static_pad(identity,"src"),rtcp_pad);
    gst_pad_link(gst_element_get_static_pad(rtph264,"src"), videosinkpad);
    
    gst_pad_link(rtcp_src_pad, gst_element_get_static_pad(senderidentity,"sink"));
    gst_pad_link(gst_element_get_static_pad(senderidentity,"src"), gst_element_get_static_pad(rtcpsink,"sink"));
    gst_element_link_many(bin, enc_ident, sink, NULL);
    g_signal_connect (identity, "handoff", G_CALLBACK (rtcp_recv_callback), NULL);
    g_signal_connect (senderidentity, "handoff", G_CALLBACK (rtcp_recv_callback), NULL);

    gst_pad_set_event_function(gst_pad_get_peer(gst_element_get_static_pad(sink,"sink")), gst_my_filter_sink_event);
    printf("Starting pipeline");
    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    bus = gst_element_get_bus (pipeline);
while(true) {
    msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS | GST_MESSAGE_QOS));

  if (msg != NULL) {
    GError *err;
    gchar *debug_info;

    switch (GST_MESSAGE_TYPE (msg)) {
      case GST_MESSAGE_ERROR:
        gst_message_parse_error (msg, &err, &debug_info);
        g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
        g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
        g_clear_error (&err);
        g_free (debug_info);
        break;
      case GST_MESSAGE_EOS:
        g_print ("End-Of-Stream reached.\n");
        break;
        case GST_MESSAGE_QOS:
                    gboolean live;
                    guint64 running_time, stream_time,timestamp,duration;
            // gst_message_parse_qos (msg,&live,&running_time,&stream_time,&timestamp,&duration);
            // g_warning("GOt a QOS event %llu %llu %llu %llu", running_time, stream_time, timestamp, duration);
            gint64 jitter;
            gdouble prop;
            gint qual;
            gst_message_parse_qos_values(msg, &jitter, &prop, &qual);
            g_warning("gotQoSE %lld %f %lu", jitter, prop, qual );
            break;
      default:
        /* We should not reach here because we only asked for ERRORs and EOS */
        g_printerr ("Unexpected message received.\n");
        break;
    }
    gst_message_unref (msg);
  }
}
    // g_print("%s", gst_pad_get_name(videosinkpad));

    return 0;
}