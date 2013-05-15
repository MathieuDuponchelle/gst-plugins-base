/* GStreamer
 * Copyright (C) 2004 Benjamin Otte <in7y118@public.uni-hamburg.de>
 * Copyright (c) 2012 Collabora Ltd.
 *	Author : Edward Hervey <edward@collabora.com>
 *      Author : Mark Nauwelaerts <mark.nauwelaerts@collabora.co.uk>
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

#ifndef __GST_EPITECHDEC_H__
#define __GST_EPITECHDEC_H__

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <gst/gst.h>
#include <gst/video/gstvideodecoder.h>
#include <string.h>

G_BEGIN_DECLS

#define GST_TYPE_EPITECH_DEC \
  (gst_epitech_dec_get_type())
#define GST_EPITECH_DEC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_EPITECH_DEC,GstEpitechDec))
#define GST_EPITECH_DEC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_EPITECH_DEC,GstEpitechDecClass))
#define GST_IS_EPITECH_DEC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_EPITECH_DEC))
#define GST_IS_EPITECH_DEC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_EPITECH_DEC))

typedef struct _GstEpitechDec GstEpitechDec;
typedef struct _GstEpitechDecClass GstEpitechDecClass;

/**
 * GstEpitechDec:
 *
 * Opaque object data structure.
 */
struct _GstEpitechDec
{
  GstVideoDecoder element;

  gboolean have_header;

  gboolean need_keyframe;
  GstVideoCodecState *input_state;
  GstVideoCodecState *output_state;

  gboolean can_crop;
  gboolean format_set;
};

struct _GstEpitechDecClass
{
  GstVideoDecoderClass parent_class;
};

GType gst_epitech_dec_get_type (void);
gboolean gst_epitech_dec_register (GstPlugin * plugin);

G_END_DECLS

#endif /* __GST_EPITECHDEC_H__ */
