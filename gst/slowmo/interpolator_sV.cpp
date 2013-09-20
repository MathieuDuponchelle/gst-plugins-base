#include <iostream>

//#include "interpolator_sV.h"
#include "flowField_sV.h"
#include "interpolate_sV.h"
#include "math.h"
//#include <QtCore/QObject>

#define MIN_FRAME_DIST .001

enum InterpolationType { InterpolationType_Forward = 0, InterpolationType_ForwardNew = 1,
                         InterpolationType_Twoway = 10, InterpolationType_TwowayNew = 11,
                         InterpolationType_Bezier = 20 };

static gboolean cpp_interpolate(GstBuffer *left, GstBuffer *right)
{
  InterpolationType interpolation = InterpolationType_Twoway;

    // if (frame > pr->frameSource()->framesCount()) {
    //   return FALSE;
    // }
    if (TRUE) {

        // GstVideoFrame *left = pr->frameSource()->frameAt(floor(frame), size);
        // GstVideoFrame *right = pr->frameSource()->frameAt(floor(frame)+1, size);
      GstBuffer *out = NULL;

        /// Position between two frames, on [0 1]
        const float pos = 0.5;

        if (interpolation == InterpolationType_Twoway) {
            FlowField_sV *forwardFlow = NULL;
            FlowField_sV *backwardFlow = NULL;
            // FlowField_sV *forwardFlow = pr->requestFlow(floor(frame), floor(frame)+1, size);
            // FlowField_sV *backwardFlow = pr->requestFlow(floor(frame)+1, floor(frame), size);

            g_assert(forwardFlow != NULL);
            g_assert(backwardFlow != NULL);

            if (forwardFlow == NULL || backwardFlow == NULL) {
	      std::cout << "No flow received!" << std::endl;
                g_assert(false);
            }

            Interpolate_sV::twowayFlow(left, right, forwardFlow, backwardFlow, pos, out);
            delete forwardFlow;
            delete backwardFlow;

        } else if (interpolation == InterpolationType_TwowayNew) {
	  FlowField_sV *forwardFlow = NULL;
	  FlowField_sV *backwardFlow = NULL;
            // FlowField_sV *forwardFlow = pr->requestFlow(floor(frame), floor(frame)+1, size);
            // FlowField_sV *backwardFlow = pr->requestFlow(floor(frame)+1, floor(frame), size);

	  g_assert(forwardFlow != NULL);
          g_assert (backwardFlow != NULL);

            if (forwardFlow == NULL || backwardFlow == NULL) {
	      std::cout << "No flow received!" << std::endl;
	      g_assert(false);
            }

            Interpolate_sV::newTwowayFlow(left, right, forwardFlow, backwardFlow, pos, out);
            delete forwardFlow;
            delete backwardFlow;

        } else if (interpolation == InterpolationType_Forward) {
	  FlowField_sV *forwardFlow = NULL;
            // FlowField_sV *forwardFlow = pr->requestFlow(floor(frame), floor(frame)+1, size);

            g_assert(forwardFlow != NULL);

            if (forwardFlow == NULL) {
	      std::cout << "No flow received!" << std::endl;
              g_assert(false);
            }

            Interpolate_sV::forwardFlow(left, forwardFlow, pos, out);
            delete forwardFlow;

        } else if (interpolation == InterpolationType_ForwardNew) {
	  FlowField_sV *forwardFlow = NULL;
            // FlowField_sV *forwardFlow = pr->requestFlow(floor(frame), floor(frame)+1, size);

            g_assert(forwardFlow != NULL);

            if (forwardFlow == NULL) {
	      std::cout << "No flow received!" << std::endl;
	      g_assert(false);
            }

            Interpolate_sV::newForwardFlow(left, forwardFlow, pos, out);
            delete forwardFlow;

        } else if (interpolation == InterpolationType_Bezier) {
	  FlowField_sV *currNext = NULL;
	  FlowField_sV *currPrev = NULL;
            // FlowField_sV *currNext = pr->requestFlow(floor(frame)+2, floor(frame)+1, size); // Allowed to be NULL
            // FlowField_sV *currPrev = pr->requestFlow(floor(frame)+0, floor(frame)+1, size);

            g_assert(currPrev != NULL);

            Interpolate_sV::bezierFlow(left, right, currPrev, currNext, pos, out);

            delete currNext;
            delete currPrev;

        } else {
	  std::cout << "Unsupported interpolation type!" << std::endl;
          g_assert(false);
        }
        return TRUE;
    } else {
      std::cout << "No interpolation necessary." << std::endl;
        return TRUE;
    }
}

extern "C" {
  gboolean interpolate(GstBuffer *leftFrame, GstBuffer *rightFrame);

  gboolean interpolate(GstBuffer *leftFrame, GstBuffer *rightFrame)
  {
    return cpp_interpolate(leftFrame, rightFrame);
  }
}
