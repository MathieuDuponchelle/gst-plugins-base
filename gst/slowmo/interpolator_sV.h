#ifndef INTERPOLATOR_SV_H
#define INTERPOLATOR_SV_H

#include <gst/video/video.h>
#include "gstslowmo.h"

gboolean interpolate(GstSlowmo *slowmo, GstBuffer *leftFrame, GstBuffer *rightFrame, GstBuffer *out, float pos, gboolean reuse_flows);

#endif // INTERPOLATOR_SV_H
