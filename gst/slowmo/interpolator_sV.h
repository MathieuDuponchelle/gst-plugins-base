#ifndef INTERPOLATOR_SV_H
#define INTERPOLATOR_SV_H

#include <gst/video/video.h>

/* #include "renderPreferences_sV.h" */
/* #include "project_sV.h" */

class Interpolator_sV
{
public:
  static gboolean interpolate(GstVideoFrame *leftFrame, GstVideoFrame *rightFrame);
};

enum InterpolationType { InterpolationType_Forward = 0, InterpolationType_ForwardNew = 1,
                         InterpolationType_Twoway = 10, InterpolationType_TwowayNew = 11,
                         InterpolationType_Bezier = 20 };

#endif // INTERPOLATOR_SV_H
