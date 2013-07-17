/* GStreamer
 * Copyright (C) 2004 Wim Taymans <wim@fluendo.com>
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
 * SECTION:element-epitechenc
 * @see_also: epitechdec, oggmux
 *
 * This element encodes raw video into a Epitech stream.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdlib.h>

#include <gst/tag/tag.h>
#include <gst/video/video.h>
#include <gst/video/gstvideometa.h>

#include "huffman.h"
#include "dct.h"
#include "yuv.h"
#include "rle.h"
#include "gstepitechenc.h"

#define GST_CAT_DEFAULT epitechenc_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

static GstStaticPadTemplate epitech_enc_sink_factory =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw, "
        "format = (string) { RGB }, "
        "framerate = (fraction) [1/MAX, MAX], "
        "width = (int) [ 1, MAX ], " "height = (int) [ 1, MAX ]"));

static GstStaticPadTemplate epitech_enc_src_factory =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-epitech, "
        "framerate = (fraction) [1/MAX, MAX], "
        "width = (int) [ 1, MAX ], " "height = (int) [ 1, MAX ]"));

#define gst_epitech_enc_parent_class parent_class
G_DEFINE_TYPE (GstEpitechEnc, gst_epitech_enc, GST_TYPE_VIDEO_ENCODER);

static gboolean epitech_enc_start (GstVideoEncoder * enc);
static gboolean epitech_enc_stop (GstVideoEncoder * enc);
static gboolean epitech_enc_set_format (GstVideoEncoder * enc,
    GstVideoCodecState * state);
static GstFlowReturn epitech_enc_handle_frame (GstVideoEncoder * enc,
    GstVideoCodecFrame * frame);
static GstFlowReturn epitech_enc_pre_push (GstVideoEncoder * benc,
    GstVideoCodecFrame * frame);
static GstFlowReturn epitech_enc_finish (GstVideoEncoder * enc);
static gboolean epitech_enc_propose_allocation (GstVideoEncoder * encoder,
    GstQuery * query);

static GstCaps *epitech_enc_getcaps (GstVideoEncoder * encoder,
    GstCaps * filter);
static void epitech_enc_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);
static void epitech_enc_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void epitech_enc_finalize (GObject * object);

static void
gst_epitech_enc_class_init (GstEpitechEncClass * klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;
  GstElementClass *element_class = (GstElementClass *) klass;
  GstVideoEncoderClass *gstvideo_encoder_class =
      GST_VIDEO_ENCODER_CLASS (klass);

  gobject_class->set_property = epitech_enc_set_property;
  gobject_class->get_property = epitech_enc_get_property;
  gobject_class->finalize = epitech_enc_finalize;

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&epitech_enc_src_factory));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&epitech_enc_sink_factory));
  gst_element_class_set_static_metadata (element_class,
      "Epitech video encoder",
      "Codec/Encoder/Video",
      "encode raw YUV video to a epitech stream",
      "Julien delaigue <delaigue@whatever.com>");

  gstvideo_encoder_class->start = GST_DEBUG_FUNCPTR (epitech_enc_start);
  gstvideo_encoder_class->stop = GST_DEBUG_FUNCPTR (epitech_enc_stop);
  gstvideo_encoder_class->set_format =
      GST_DEBUG_FUNCPTR (epitech_enc_set_format);
  gstvideo_encoder_class->handle_frame =
      GST_DEBUG_FUNCPTR (epitech_enc_handle_frame);
  gstvideo_encoder_class->pre_push = GST_DEBUG_FUNCPTR (epitech_enc_pre_push);
  gstvideo_encoder_class->finish = GST_DEBUG_FUNCPTR (epitech_enc_finish);
  gstvideo_encoder_class->getcaps = GST_DEBUG_FUNCPTR (epitech_enc_getcaps);
  gstvideo_encoder_class->propose_allocation =
      GST_DEBUG_FUNCPTR (epitech_enc_propose_allocation);

  GST_DEBUG_CATEGORY_INIT (epitechenc_debug, "epitechenc", 0,
      "Epitech encoder");
}

static void
gst_epitech_enc_init (GstEpitechEnc * enc)
{
  enc->format_set = FALSE;
  enc->input_state = NULL;
}

static void
epitech_enc_finalize (GObject * object)
{
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
epitech_enc_start (GstVideoEncoder * benc)
{
  return TRUE;
}

static gboolean
epitech_enc_stop (GstVideoEncoder * benc)
{
  return TRUE;
}

static GstCaps *
epitech_enc_getcaps (GstVideoEncoder * encoder, GstCaps * filter)
{
  GstCaps *caps, *ret;
  gchar *caps_string;

  caps_string = g_strdup_printf ("video/x-raw, "
      "framerate = (fraction) [1/MAX, MAX], "
      "width = (int) [ 1, MAX ], " "height = (int) [ 1, MAX ]");
  caps = gst_caps_from_string (caps_string);
  g_free (caps_string);
  GST_DEBUG ("Supported caps: %" GST_PTR_FORMAT, caps);

  ret = gst_video_encoder_proxy_getcaps (encoder, caps, filter);
  gst_caps_unref (caps);

  return ret;
}

static gboolean
epitech_enc_set_format (GstVideoEncoder * benc, GstVideoCodecState * state)
{
  GstEpitechEnc *enc = GST_EPITECH_ENC (benc);

  enc->input_state = gst_video_codec_state_ref (state);

  return TRUE;
}

static GstFlowReturn
epitech_enc_pre_push (GstVideoEncoder * benc, GstVideoCodecFrame * frame)
{
  return GST_FLOW_OK;
}

static GstFlowReturn
epitech_enc_handle_frame (GstVideoEncoder * benc, GstVideoCodecFrame * frame)
{
  GstEpitechEnc *enc = GST_EPITECH_ENC (benc);

  if (enc->input_state) {
    GstMapInfo info_in;
    const guint8 *data_in;
    void *res;
    unsigned int res_size;
    unsigned int size = 240 * 320 * 2;
    char *dct;
    unsigned char *buffer_yuv;
    unsigned char *rle = g_malloc (240 * 320 * 2);

    if (!enc->format_set) {
      GstCaps *caps;
      caps = gst_caps_new_empty_simple ("video/x-epitech");

      gst_video_encoder_set_output_state (benc, caps, enc->input_state);
      gst_video_encoder_negotiate (benc);
      enc->format_set = TRUE;
    }
    //    GST_ERROR("timestamp : %" GST_TIME_FORMAT, GST_TIME_ARGS(GST_BUFFER_TIMESTAMP(frame->input_buffer)));

    /* Here we map the buffers. input_buffer contains the RGB data, output_buffer has to be filled */
    gst_buffer_map (frame->input_buffer, &info_in, GST_MAP_READ);

    /* Here be dragons (and a char *) */

    data_in = info_in.data;

    buffer_yuv = yuv422 (data_in, 240, 320);
    dct = dct_encode (buffer_yuv, 240, 320 * 2);
    rle_encode ((unsigned char *) dct, rle, &size);

    free (buffer_yuv);
    free (dct);

    res = huffman_encode ((unsigned char *) rle, size, &res_size);

    free (rle);

    GST_ERROR ("Encoded buffer, original / compressed %u %u",
        (unsigned int) info_in.size, res_size);

    /* Here we unmap the buffers. No more access is possible */
    gst_buffer_unmap (frame->input_buffer, &info_in);

    GST_VIDEO_CODEC_FRAME_SET_SYNC_POINT (frame);

    /* Here the purpose is to do frame->output_buffer = outbuf */
    /* For now let's just copy the input_buffer */
    frame->output_buffer = gst_buffer_new_wrapped (res, res_size);

    GST_BUFFER_TIMESTAMP (frame->output_buffer) =
        GST_BUFFER_TIMESTAMP (frame->input_buffer);
    GST_BUFFER_DURATION (frame->output_buffer) =
        GST_BUFFER_DURATION (frame->input_buffer);

    gst_video_encoder_finish_frame (benc, frame);
  }

  return GST_FLOW_OK;
}

static gboolean
epitech_enc_finish (GstVideoEncoder * benc)
{
  return TRUE;
}

static gboolean
epitech_enc_propose_allocation (GstVideoEncoder * encoder, GstQuery * query)
{
  gst_query_add_allocation_meta (query, GST_VIDEO_META_API_TYPE, NULL);

  return GST_VIDEO_ENCODER_CLASS (parent_class)->propose_allocation (encoder,
      query);
}

static void
epitech_enc_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
epitech_enc_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

gboolean
gst_epitech_enc_register (GstPlugin * plugin)
{
  return gst_element_register (plugin, "epitechenc",
      GST_RANK_PRIMARY, GST_TYPE_EPITECH_ENC);
}
