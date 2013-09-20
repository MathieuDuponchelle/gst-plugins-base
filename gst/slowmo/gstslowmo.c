/* GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
 * This file:
 * Copyright (C) 2003 Ronald Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2010 David Schleef <ds@schleef.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/**
 * SECTION:element-slowmo
 * Fancy shit yo.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "gstslowmo.h"

#include <gst/video/video.h>
#include <gst/video/gstvideometa.h>
#include <gst/video/gstvideopool.h>

#include <string.h>

GST_DEBUG_CATEGORY (slowmo_debug);
#define GST_CAT_DEFAULT slowmo_debug
GST_DEBUG_CATEGORY_EXTERN (GST_CAT_PERFORMANCE);

GType gst_slowmo_get_type (void);

#define gst_slowmo_parent_class parent_class
G_DEFINE_TYPE (GstSlowmo, gst_slowmo, GST_TYPE_VIDEO_FILTER);

enum
{
  PROP_0,
};

#define CSP_VIDEO_CAPS GST_VIDEO_CAPS_MAKE (GST_VIDEO_FORMATS_ALL) ";" \
    GST_VIDEO_CAPS_MAKE_WITH_FEATURES ("ANY", GST_VIDEO_FORMATS_ALL)

static GstStaticPadTemplate gst_slowmo_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (CSP_VIDEO_CAPS)
    );

static GstStaticPadTemplate gst_slowmo_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (CSP_VIDEO_CAPS)
    );

static void gst_slowmo_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_slowmo_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);

static GstFlowReturn gst_slowmo_transform_frame (GstVideoFilter * filter,
    GstVideoFrame * in_frame, GstVideoFrame * out_frame);

static void
gst_slowmo_class_init (GstSlowmoClass * klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;
  GstElementClass *gstelement_class = (GstElementClass *) klass;
  GstVideoFilterClass *gstvideofilter_class = (GstVideoFilterClass *) klass;

  gobject_class->set_property = gst_slowmo_set_property;
  gobject_class->get_property = gst_slowmo_get_property;

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&gst_slowmo_src_template));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&gst_slowmo_sink_template));

  gst_element_class_set_static_metadata (gstelement_class,
      "SlowMotion filter", "Filter/Rate/Video",
      "Slow stuff down. Don't try to slow stuff up it doesn't make sense",
      "GStreamer maintainers <gstreamer-devel@lists.sourceforge.net>");

  gstvideofilter_class->transform_frame =
      GST_DEBUG_FUNCPTR (gst_slowmo_transform_frame);
}

static void
gst_slowmo_init (GstSlowmo * space)
{
}

void
gst_slowmo_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstSlowmo *slowmo;

  slowmo = GST_SLOWMO (object);

  (void) slowmo;

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_slowmo_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstSlowmo *slowmo;

  slowmo = GST_SLOWMO (object);

  (void) slowmo;

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static GstFlowReturn
gst_slowmo_transform_frame (GstVideoFilter * filter,
    GstVideoFrame * in_frame, GstVideoFrame * out_frame)
{
  GstSlowmo *slowmo;

  slowmo = GST_SLOWMO_CAST (filter);

  (void) slowmo;

  GST_CAT_DEBUG_OBJECT (GST_CAT_PERFORMANCE, filter,
      "doing colorspace conversion from %s -> to %s",
      GST_VIDEO_INFO_NAME (&filter->in_info),
      GST_VIDEO_INFO_NAME (&filter->out_info));

  gst_video_frame_copy (out_frame, in_frame);

  return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (slowmo_debug, "slowmo", 0, "Colorspace Converter");

  return gst_element_register (plugin, "slowmo",
      GST_RANK_NONE, GST_TYPE_SLOWMO);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    slowmo,
    "This will make slow motion so smooth it will cause google's market share to drop",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
