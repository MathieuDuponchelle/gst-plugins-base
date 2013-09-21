#include <iostream>

//#include "interpolator_sV.h"
#include "flowField_sV.h"
#include "interpolate_sV.h"
#include "abstractFlowSource_sV.h"
#include "flowSourceOpenCV_sV.h"
#include "math.h"
//#include <QtCore/QObject>

#define MIN_FRAME_DIST .001

enum InterpolationType { InterpolationType_Forward = 0, InterpolationType_ForwardNew = 1,
                         InterpolationType_Twoway = 10, InterpolationType_TwowayNew = 11,
                         InterpolationType_Bezier = 20 };

static FlowField_sV *forwardFlow;
static FlowField_sV *backwardFlow;

static gboolean cpp_interpolate(GstBuffer *left, GstBuffer *right, GstBuffer *out, float pos, gboolean reuse_flows)
{
  InterpolationType interpolation = InterpolationType_Twoway;
  static AbstractFlowSource_sV *m_flowSource = NULL;
  if (m_flowSource == NULL)
    m_flowSource = new FlowSourceOpenCV_sV ();

  if (interpolation == InterpolationType_Twoway) {
    if (!reuse_flows)
      {
	forwardFlow = m_flowSource->buildFlow(left, right);
	backwardFlow = m_flowSource->buildFlow(right, left);
      }

    g_assert(forwardFlow != NULL);
    g_assert(backwardFlow != NULL);

    if (forwardFlow == NULL || backwardFlow == NULL) {
      std::cout << "No flow received!" << std::endl;
      g_assert(false);
    }

    Interpolate_sV::twowayFlow(left, right, forwardFlow, backwardFlow, pos, out);
    //    delete forwardFlow;
    //delete backwardFlow;

  } else if (interpolation == InterpolationType_TwowayNew) {
    forwardFlow = NULL;
    backwardFlow = NULL;

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
    forwardFlow = NULL;

    g_assert(forwardFlow != NULL);

    if (forwardFlow == NULL) {
      std::cout << "No flow received!" << std::endl;
      g_assert(false);
    }

    Interpolate_sV::forwardFlow(left, forwardFlow, pos, out);
    delete forwardFlow;

  } else if (interpolation == InterpolationType_ForwardNew) {
    forwardFlow = NULL;

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

    g_assert(currPrev != NULL);

    Interpolate_sV::bezierFlow(left, right, currPrev, currNext, pos, out);

    delete currNext;
    delete currPrev;

  } else {
    std::cout << "Unsupported interpolation type!" << std::endl;
    g_assert(false);
  }
  return TRUE;
}

extern "C" {
  gboolean interpolate(GstBuffer *leftFrame, GstBuffer *rightFrame, GstBuffer *out, float pos, gboolean reuse_flows);

  gboolean interpolate(GstBuffer *leftFrame, GstBuffer *rightFrame, GstBuffer *out, float pos, gboolean reuse_flows)
  {
    return cpp_interpolate(leftFrame, rightFrame, out, pos, reuse_flows);
  }
}
