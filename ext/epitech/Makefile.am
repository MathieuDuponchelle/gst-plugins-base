plugin_LTLIBRARIES = libgstepitech.la

noinst_HEADERS = gstepitechenc.h \
		 gstepitechdec.h

libgstepitech_la_SOURCES = gstepitech.c \
			  gstepitechenc.c \
			  gstepitechdec.c \
			  huffman.c

libgstepitech_la_CFLAGS = $(GST_PLUGINS_BASE_CFLAGS) $(GST_BASE_CFLAGS) $(GST_CFLAGS)
libgstepitech_la_LIBADD = \
	$(top_builddir)/gst-libs/gst/tag/libgsttag-$(GST_API_VERSION).la \
	$(top_builddir)/gst-libs/gst/video/libgstvideo-$(GST_API_VERSION).la \
	$(GST_BASE_LIBS) \
	$(GST_LIBS)

libgstepitech_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstepitech_la_LIBTOOLFLAGS = $(GST_PLUGIN_LIBTOOLFLAGS)

