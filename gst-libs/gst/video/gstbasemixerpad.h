/* Generic video mixer plugin
 * Copyright (C) 2008 Wim Taymans <wim@fluendo.com>
 * Copyright (C) 2010 Sebastian Dröge <sebastian.droege@collabora.co.uk>
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
 
#ifndef __GST_BASE_MIXER_PAD_H__
#define __GST_BASE_MIXER_PAD_H__

#include <gst/gst.h>
#include <gst/video/video.h>

#include <gst/base/gstcollectpads.h>

#include "videoconvert.h"

G_BEGIN_DECLS

#define GST_TYPE_BASE_MIXER_PAD (gst_basemixer_pad_get_type())
#define GST_BASE_MIXER_PAD(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_BASE_MIXER_PAD, GstBasemixerPad))
#define GST_BASE_MIXER_PAD_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_VIDEO_MIXER_PAD, GstBasemixerPadClass))
#define GST_IS_BASE_MIXER_PAD(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_BASE_MIXER_PAD))
#define GST_IS_BASE_MIXER_PAD_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_BASE_MIXER_PAD))

typedef struct _GstBasemixerPad GstBasemixerPad;
typedef struct _GstBasemixerPadClass GstBasemixerPadClass;
typedef struct _GstBasemixerCollect GstBasemixerCollect;

/**
 * GstBasemixerPad:
 *
 * The opaque #GstBasemixerPad structure.
 */
struct _GstBasemixerPad
{
  GstPad parent;

  /* < private > */

  /* caps */
  GstVideoInfo info;

  /* properties */
  guint zorder;

  GstBasemixerCollect *mixcol;

  /* caps used for conversion if needed */
  GstVideoInfo conversion_info;

  /* Converter, if NULL no conversion is done */
  VideoConvert *convert;
  gboolean need_conversion_update;
  GstBuffer *converted_buffer;

  GstVideoFrame *mixed_frame;
};

struct _GstBasemixerPadClass
{
  GstPadClass parent_class;
};

GType gst_basemixer_pad_get_type (void);

G_END_DECLS
#endif /* __GST_BASE_MIXER_PAD_H__ */
