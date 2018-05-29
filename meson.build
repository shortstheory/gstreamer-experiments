project('tutorial', 'c')
gstdep = dependency('gstreamer-1.0')
executable('demo', 'server.c', dependencies : gstdep)
executable('rtp', 'rtcp_sender.c', dependencies: gstdep)
