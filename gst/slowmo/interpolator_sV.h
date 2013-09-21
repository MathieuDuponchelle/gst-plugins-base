#ifndef INTERPOLATOR_SV_H
#define INTERPOLATOR_SV_H

#include <gst/video/video.h>

/* #include "renderPreferences_sV.h" */
/* #include "project_sV.h" */

gboolean interpolate(GstBuffer *leftFrame, GstBuffer *rightFrame, GstBuffer *out, float pos, gboolean reuse_flows);

#endif // INTERPOLATOR_SV_H
