/* GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
 * This file:
 * Copyright (C) 2003 Ronald Bultje <rbultje@ronald.bitfreak.net>
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

#ifndef __GST_SLOWMO_H__
#define __GST_SLOWMO_H__

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>

G_BEGIN_DECLS

#define GST_TYPE_SLOWMO	          (gst_slowmo_get_type())
#define GST_SLOWMO(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_SLOWMO,GstSlowmo))
#define GST_SLOWMO_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_SLOWMO,GstSlowmoClass))
#define GST_IS_SLOWMO(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_SLOWMO))
#define GST_IS_SLOWMO_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_SLOWMO))
#define GST_SLOWMO_CAST(obj)       ((GstSlowmo *)(obj))

typedef struct _GstSlowmo GstSlowmo;
typedef struct _GstSlowmoClass GstSlowmoClass;

/**
 * GstSlowmo:
 *
 * Opaque object data structure.
 */
struct _GstSlowmo {
  GstVideoFilter element;
};

struct _GstSlowmoClass
{
  GstVideoFilterClass parent_class;
};

G_END_DECLS

#endif /* __GST_SLOWMO_H__ */
