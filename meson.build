project('tutorial', 'cpp')
gstdep = dependency('gstreamer-1.0')
gst_base_dep = dependency('gstreamer-rtp-1.0')
executable('rtp', 'rtcp_sender.cpp', dependencies: [gstdep, gst_base_dep])
