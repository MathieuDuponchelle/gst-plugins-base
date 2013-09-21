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

#include <stdlib.h>
#include <stdio.h>
#include "gstslowmo.h"
#include "interpolator_sV.h"

#include <gst/video/video.h>
#include <gst/video/gstvideometa.h>
#include <gst/video/gstvideopool.h>
#include "gstvideorate.h"

#include <string.h>

typedef struct
{
  GstClockTime timestamp;
} PendingFrame;

GST_DEBUG_CATEGORY (slowmo_debug);
#define GST_CAT_DEFAULT slowmo_debug
GST_DEBUG_CATEGORY_EXTERN (GST_CAT_PERFORMANCE);

GType gst_slowmo_get_type (void);

#define gst_slowmo_parent_class parent_class
G_DEFINE_TYPE (GstSlowmo, gst_slowmo, GST_TYPE_BASE_TRANSFORM);

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

static GstFlowReturn gst_slowmo_transform_ip (GstBaseTransform * trans,
    GstBuffer * buffer);

static void
gst_slowmo_class_init (GstSlowmoClass * klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;
  GstElementClass *gstelement_class = (GstElementClass *) klass;
  GstBaseTransformClass *gstbasetransform_class =
      (GstBaseTransformClass *) klass;

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

  gstbasetransform_class->transform_ip =
      GST_DEBUG_FUNCPTR (gst_slowmo_transform_ip);
}

static void
gst_slowmo_init (GstSlowmo * slowmo)
{
  slowmo->prevbuf = NULL;
  slowmo->pending_frames = NULL;
  slowmo->dummy = 0;
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

static void
dumpToFile (GstBuffer * buffer, const char *fName, int count)
{
  int i = 0;
  FILE *fpOut;
  GstMapInfo info;

  gst_buffer_map (buffer, &info, GST_MAP_READ);

  if ((fpOut = fopen (fName, "wb")) == NULL) {
    perror (fName);
    exit (EXIT_FAILURE);
  }

  while (i < count) {
    fputc (info.data[i], fpOut);
    i += 1;
  }
  fclose (fpOut);

  GST_ERROR ("dumped");
}

static GstFlowReturn
interpolate_pending_frames (GstSlowmo * slowmo, GstBuffer * buffer)
{
  GList *tmp;
  GstBuffer *outbuf;
  gboolean ret = GST_FLOW_OK;
  float step;
  float pos = 0;
  gchar *name = malloc (sizeof (char) * 20);
  gboolean reuse = FALSE;

  /* Don't divide by zero please */
  if (!g_list_length (slowmo->pending_frames))
    return GST_FLOW_OK;

  step = 1.0 / (g_list_length (slowmo->pending_frames) + 1);

  pos += step;

  if (FALSE)
    dumpToFile (slowmo->prevbuf, "cool.rgb", 1280 * 720 * 3);
  GST_ERROR_OBJECT (slowmo,
      "outputting one buffer with timestamp : %" GST_TIME_FORMAT,
      GST_TIME_ARGS (GST_BUFFER_TIMESTAMP (slowmo->prevbuf)));
  ret =
      gst_pad_push (GST_BASE_TRANSFORM_SRC_PAD (slowmo),
      gst_buffer_ref (slowmo->prevbuf));

  for (tmp = slowmo->pending_frames; tmp; tmp = tmp->next) {
    outbuf = gst_buffer_new_allocate (NULL, 1280 * 720 * 3, NULL);

    interpolate (slowmo->prevbuf, buffer, outbuf, pos, reuse);

    GST_BUFFER_TIMESTAMP (outbuf) = ((PendingFrame *) tmp->data)->timestamp;

    GST_ERROR_OBJECT (slowmo,
        "outputting one buffer with timestamp : %" GST_TIME_FORMAT,
        GST_TIME_ARGS (GST_BUFFER_TIMESTAMP (outbuf)));

    slowmo->dummy += 1;

    sprintf (name, "dummy%d.rgb", slowmo->dummy);

    if (FALSE)
      dumpToFile (outbuf, name, 1280 * 720 * 3);

    ret = gst_pad_push (GST_BASE_TRANSFORM_SRC_PAD (slowmo), outbuf);
    if (ret != GST_FLOW_OK) {
      GST_ERROR ("fuck what");
      break;
    }

    reuse = TRUE;
    GST_ERROR ("pos : %f", pos);
    pos += step;
  }

  free (name);

  g_list_free_full (slowmo->pending_frames, g_free);
  slowmo->pending_frames = NULL;

  return ret;
}

static GstFlowReturn
gst_slowmo_transform_ip (GstBaseTransform * trans, GstBuffer * buffer)
{
  GstSlowmo *slowmo;
  GstFlowReturn ret = GST_BASE_TRANSFORM_FLOW_DROPPED;
  GstVideoRateMeta *meta;

  meta =
      (GstVideoRateMeta *) gst_buffer_get_meta (buffer,
      gst_video_rate_meta_api_get_type ());

  if (!meta) {
    GST_ERROR ("We need videorate metadata to operate");
    return GST_FLOW_ERROR;
  }

  GST_DEBUG_OBJECT (trans,
      "Received buffer %p with timestamp : %" GST_TIME_FORMAT, buffer,
      GST_TIME_ARGS (GST_BUFFER_TIMESTAMP (buffer)));

  GST_DEBUG_OBJECT (trans, "buffer has meta %p, original is : %d", meta,
      meta->original);

  slowmo = GST_SLOWMO_CAST (trans);

  if (!meta->original) {
    PendingFrame *pending = g_malloc (sizeof (PendingFrame));
    pending->timestamp = GST_BUFFER_TIMESTAMP (buffer);
    slowmo->pending_frames = g_list_append (slowmo->pending_frames, pending);
    return ret;
  }

  if (slowmo->prevbuf) {
    GST_DEBUG_OBJECT (slowmo,
        "pushing buffer with timestamp : %" GST_TIME_FORMAT,
        GST_TIME_ARGS (GST_BUFFER_TIMESTAMP (slowmo->prevbuf)));

    interpolate_pending_frames (slowmo, buffer);
  }

  slowmo->prevbuf = gst_buffer_ref (buffer);

  GST_DEBUG_OBJECT (slowmo, "storing buffer with timestamp : %" GST_TIME_FORMAT,
      GST_TIME_ARGS (GST_BUFFER_TIMESTAMP (slowmo->prevbuf)));

  return ret;
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
