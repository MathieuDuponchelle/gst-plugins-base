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
 
#ifndef __GST_BASE_MIXER_H__
#define __GST_BASE_MIXER_H__

#include <gst/gst.h>
#include <gst/video/video.h>

#include "gstbasemixerpad.h"

#include <gst/base/gstcollectpads.h>

G_BEGIN_DECLS

#define GST_TYPE_BASE_MIXER (gst_basemixer_get_type())
#define GST_BASE_MIXER(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_BASE_MIXER, GstBasemixer))
#define GST_BASE_MIXER_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_BASE_MIXER, GstBasemixerClass))
#define GST_IS_BASE_MIXER(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_BASE_MIXER))
#define GST_IS_BASE_MIXER_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_BASE_MIXER))

typedef struct _GstBasemixer GstBasemixer;
typedef struct _GstBasemixerClass GstBasemixerClass;

/**
 * GstBasemixer:
 *
 * The opaque #GstBasemixer structure.
 */
struct _GstBasemixer
{
  GstElement element;

  /* < private > */

  /* pad */
  GstPad *srcpad;

  /* Lock to prevent the state to change while mixing */
  GMutex lock;

  /* Lock to prevent two src setcaps from happening at the same time  */
  GMutex setcaps_lock;

  /* Sink pads using Collect Pads 2*/
  GstCollectPads *collect;

  /* sinkpads, a GSList of GstBasemixerPads */
  GSList *sinkpads;
  gint numpads;
  /* Next available sinkpad index */
  guint next_sinkpad;

  /* Output caps */
  GstVideoInfo info;

  /* current caps */
  GstCaps *current_caps;
  gboolean send_caps;

  gboolean newseg_pending;
  gboolean flush_stop_pending; /* Used when we receive a flushing seek,
                                  to send a flush_stop right before the
                                  following buffer */
  gboolean waiting_flush_stop; /* Used when we receive a flush_start to make
                                  sure to forward the flush_stop only once */

  /* Current downstream segment */
  GstSegment segment;
  GstClockTime ts_offset;
  guint64 nframes;

  /* QoS stuff */
  gdouble proportion;
  GstClockTime earliest_time;
  guint64 qos_processed, qos_dropped;

  gboolean send_stream_start;
};

struct _GstBasemixerClass
{
  GstElementClass parent_class;

  GstBasemixerPad*               (*create_new_pad)      (GstBasemixer *basemixer, GstPadTemplate *templ,
							 const gchar* name, const GstCaps *caps);
  gboolean			 (*modify_src_pad_info) (GstBasemixer *basemixer, GstVideoInfo *info);
  GstFlowReturn                  (*mix_frames)          (GstBasemixer *basemixer, GstVideoFrame *outframe);
};

GType gst_basemixer_get_type (void);

G_END_DECLS
#endif /* __GST_BASE_MIXER_H__ */
