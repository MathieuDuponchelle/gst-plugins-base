plugin_LTLIBRARIES = libgstslowmo.la

libgstslowmo_la_SOURCES =	gstslowmo.c \
				bezierTools_sV.cpp \
				flowField_sV.cpp \
				flowTools_sV.cpp \
				interpolate_sV.cpp \
				interpolator_sV.cpp \
				kernel_sV.cpp \
				sourceField_sV.cpp \
				vector_sV.cpp \
				flowSourceOpenCV_sV.cpp \
				abstractFlowSource_sV.cpp

libgstslowmo_la_CFLAGS = \
	$(GST_PLUGINS_BASE_CFLAGS) \
	$(GST_CFLAGS) \
	-Wno-aggregate-return \
	-I /home/meh/devel/pitivi-git/gst-plugins-base/gst/videorate

libgstslowmo_la_CXXFLAGS = \
	$(GST_PLUGINS_BASE_CFLAGS) \
	$(GST_CXXFLAGS) \
	-Wno-aggregate-return

libgstslowmo_la_LIBADD = \
	$(top_builddir)/gst-libs/gst/video/libgstvideo-$(GST_API_VERSION).la \
	$(GST_BASE_LIBS) \
	$(GST_LIBS) \
	$(LIBM)

libgstslowmo_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS) `pkg-config opencv --cflags --libs`
libgstslowmo_la_LIBTOOLFLAGS = $(GST_PLUGIN_LIBTOOLFLAGS)

noinst_HEADERS = bezierTools_sV.h  flowField_sV.h  flowTools_sV.h  gstslowmo.h  interpolate_sV.h  interpolator_sV.h  kernel_sV.h  QPoint_crack.h  sourceField_sV.h  vector_sV.h abstractFlowSource_sV.h flowSourceOpenCV_sV.h

Android.mk: Makefile.am $(BUILT_SOURCES)
	androgenizer \
	-:PROJECT libgstslowmo -:SHARED libgstslowmo \
	 -:TAGS eng debug \
         -:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
	 -:SOURCES $(libgstslowmo_la_SOURCES) \
	           $(nodist_libgstslowmo_la_SOURCES) \
	 -:CFLAGS $(DEFS) $(DEFAULT_INCLUDES) $(libgstslowmo_la_CFLAGS) \
	 -:LDFLAGS $(libgstslowmo_la_LDFLAGS) \
	           $(libgstslowmo_la_LIBADD) \
	           -ldl \
	 -:PASSTHROUGH LOCAL_ARM_MODE:=arm \
		       LOCAL_MODULE_PATH:='$$(TARGET_OUT)/lib/gstreamer-0.10' \
	> $@
