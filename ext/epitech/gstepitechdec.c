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

/**
 * SECTION:element-epitechdec
 * @see_also: epitechenc, oggdemux
 *
 * This element decodes epitech streams into raw video
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "gstepitechdec.h"
#include <gst/tag/tag.h>
#include <gst/video/video.h>
#include <gst/video/gstvideometa.h>
#include <gst/video/gstvideopool.h>

#include "huffman.h"

#define GST_CAT_DEFAULT epitechdec_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
GST_DEBUG_CATEGORY_EXTERN (GST_CAT_PERFORMANCE);

/* This was removed from the base class, this is used as a
   temporary return to signal the need to call _drop_frame,
   and does not leave epitechenc. */
#define GST_CUSTOM_FLOW_DROP GST_FLOW_CUSTOM_SUCCESS_1

static GstStaticPadTemplate epitech_dec_src_factory =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw, "
        "format = (string) { RGB }, "
        "framerate = (fraction) [0/1, MAX], "
        "width = (int) [ 1, MAX ], " "height = (int) [ 1, MAX ]")
    );

static GstStaticPadTemplate epitech_dec_sink_factory =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-epitech")
    );

#define gst_epitech_dec_parent_class parent_class
G_DEFINE_TYPE (GstEpitechDec, gst_epitech_dec, GST_TYPE_VIDEO_DECODER);

static void epitech_dec_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);
static void epitech_dec_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);

static gboolean epitech_dec_reset (GstVideoDecoder * bdec, gboolean hard);
static gboolean epitech_dec_start (GstVideoDecoder * decoder);
static gboolean epitech_dec_stop (GstVideoDecoder * decoder);
static gboolean epitech_dec_set_format (GstVideoDecoder * decoder,
    GstVideoCodecState * state);
static GstFlowReturn epitech_dec_parse (GstVideoDecoder * decoder,
    GstVideoCodecFrame * frame, GstAdapter * adapter, gboolean at_eos);
static GstFlowReturn epitech_dec_handle_frame (GstVideoDecoder * decoder,
    GstVideoCodecFrame * frame);
static gboolean epitech_dec_decide_allocation (GstVideoDecoder * decoder,
    GstQuery * query);

static void
gst_epitech_dec_class_init (GstEpitechDecClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
  GstVideoDecoderClass *video_decoder_class = GST_VIDEO_DECODER_CLASS (klass);

  gobject_class->set_property = epitech_dec_set_property;
  gobject_class->get_property = epitech_dec_get_property;

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&epitech_dec_src_factory));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&epitech_dec_sink_factory));
  gst_element_class_set_static_metadata (element_class,
      "Epitech video decoder", "Codec/Decoder/Video",
      "decode raw epitech streams to raw YUV video",
      "Benjamin Otte <otte@gnome.org>, Wim Taymans <wim@fluendo.com>");

  video_decoder_class->start = GST_DEBUG_FUNCPTR (epitech_dec_start);
  video_decoder_class->stop = GST_DEBUG_FUNCPTR (epitech_dec_stop);
  video_decoder_class->reset = GST_DEBUG_FUNCPTR (epitech_dec_reset);
  video_decoder_class->set_format = GST_DEBUG_FUNCPTR (epitech_dec_set_format);
  video_decoder_class->parse = GST_DEBUG_FUNCPTR (epitech_dec_parse);
  video_decoder_class->handle_frame =
      GST_DEBUG_FUNCPTR (epitech_dec_handle_frame);
  video_decoder_class->decide_allocation =
      GST_DEBUG_FUNCPTR (epitech_dec_decide_allocation);

  GST_DEBUG_CATEGORY_INIT (epitechdec_debug, "epitechdec", 0,
      "Epitech decoder");
}

static void
gst_epitech_dec_init (GstEpitechDec * dec)
{
  /* input is packetized,
   * but is not marked that way so data gets parsed and keyframes marked */
  gst_video_decoder_set_packetized (GST_VIDEO_DECODER (dec), FALSE);
  dec->format_set = FALSE;
}

static gboolean
epitech_dec_start (GstVideoDecoder * decoder)
{
  return TRUE;
}

static gboolean
epitech_dec_stop (GstVideoDecoder * decoder)
{
  return TRUE;
}

static gboolean
epitech_dec_reset (GstVideoDecoder * bdec, gboolean hard)
{
  return TRUE;
}

static GstFlowReturn
epitech_dec_parse (GstVideoDecoder * decoder,
    GstVideoCodecFrame * frame, GstAdapter * adapter, gboolean at_eos)
{
  gint av;

  av = gst_adapter_available (adapter);
  gst_video_decoder_add_to_frame (decoder, av);
  return gst_video_decoder_have_frame (decoder);
}


static gboolean
epitech_dec_set_format (GstVideoDecoder * bdec, GstVideoCodecState * state)
{
  GstEpitechDec *dec;

  dec = GST_EPITECH_DEC (bdec);

  /* Keep a copy of the input state */
  if (dec->input_state)
    gst_video_codec_state_unref (dec->input_state);
  dec->input_state = gst_video_codec_state_ref (state);

  return TRUE;
}

static GstFlowReturn
epitech_dec_handle_frame (GstVideoDecoder * bdec, GstVideoCodecFrame * frame)
{
  GstEpitechDec *dec;

  dec = GST_EPITECH_DEC (bdec);

  if (dec->input_state && !dec->format_set) {
    GstVideoCodecState *state;
    GstVideoInfo *info = &dec->input_state->info;

    dec->output_state = state =
        gst_video_decoder_set_output_state (GST_VIDEO_DECODER (dec),
        GST_VIDEO_FORMAT_RGB, info->width, info->height, dec->input_state);
    gst_video_decoder_negotiate (GST_VIDEO_DECODER (dec));
    dec->format_set = TRUE;
  }

  if (dec->format_set) {
    GstMapInfo info_in;
    const guint8 *data_in;
    void *res;
    unsigned int res_size;

    gst_buffer_map (frame->input_buffer, &info_in, GST_MAP_READ);

    data_in = info_in.data;

    res = huffman_decode ((unsigned char *) data_in, info_in.size, &res_size);

    /* Here we unmap the buffers. No more access is possible */
    gst_buffer_unmap (frame->input_buffer, &info_in);

    frame->output_buffer = gst_buffer_new_wrapped (res, res_size);

    GST_BUFFER_TIMESTAMP (frame->output_buffer) =
        GST_BUFFER_TIMESTAMP (frame->input_buffer);
    GST_BUFFER_DURATION (frame->output_buffer) =
        GST_BUFFER_DURATION (frame->input_buffer);
    GST_BUFFER_PTS (frame->output_buffer) =
        GST_BUFFER_TIMESTAMP (frame->input_buffer);
    gst_video_decoder_finish_frame (bdec, frame);
  }

  return GST_FLOW_OK;
}

static gboolean
epitech_dec_decide_allocation (GstVideoDecoder * decoder, GstQuery * query)
{
  GstEpitechDec *dec = GST_EPITECH_DEC (decoder);
  GstVideoCodecState *state;
  GstBufferPool *pool;
  guint size, min, max;
  GstStructure *config;

  if (!GST_VIDEO_DECODER_CLASS (parent_class)->decide_allocation (decoder,
          query))
    return FALSE;

  state = gst_video_decoder_get_output_state (decoder);

  gst_query_parse_nth_allocation_pool (query, 0, &pool, &size, &min, &max);

  dec->can_crop = FALSE;
  config = gst_buffer_pool_get_config (pool);
  if (gst_query_find_allocation_meta (query, GST_VIDEO_META_API_TYPE, NULL)) {
    gst_buffer_pool_config_add_option (config,
        GST_BUFFER_POOL_OPTION_VIDEO_META);
  }

  gst_buffer_pool_set_config (pool, config);

  gst_query_set_nth_allocation_pool (query, 0, pool, size, min, max);

  gst_object_unref (pool);
  gst_video_codec_state_unref (state);

  return TRUE;
}

static void
epitech_dec_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
epitech_dec_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

gboolean
gst_epitech_dec_register (GstPlugin * plugin)
{
  return gst_element_register (plugin, "epitechdec",
      GST_RANK_PRIMARY, GST_TYPE_EPITECH_DEC);
}
