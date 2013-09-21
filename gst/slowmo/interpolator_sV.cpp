#include <iostream>

#include "interpolate_sV.h"
#include "abstractFlowSource_sV.h"
#include "flowSourceOpenCV_sV.h"
#include "math.h"
#include "flowField_sV.h"

#define MIN_FRAME_DIST .001

enum InterpolationType { InterpolationType_Forward = 0, InterpolationType_ForwardNew = 1,
                         InterpolationType_Twoway = 10, InterpolationType_TwowayNew = 11,
                         InterpolationType_Bezier = 20 };

static gboolean cpp_interpolate(GstSlowmo *slowmo, GstBuffer *left, GstBuffer *right, GstBuffer *out, float pos, gboolean reuse_flows)
{
  InterpolationType interpolation = InterpolationType_Twoway;
  AbstractFlowSource_sV *m_flowSource;
  FlowField_sV *forwardFlow;
  FlowField_sV *backwardFlow;

  if (slowmo->m_flowSource == NULL)
    slowmo->m_flowSource = new FlowSourceOpenCV_sV ();

  forwardFlow = (FlowField_sV *) slowmo->forwardFlow;
  backwardFlow = (FlowField_sV *) slowmo->backwardFlow;

  m_flowSource = (AbstractFlowSource_sV *) slowmo->m_flowSource;

  if (interpolation == InterpolationType_Twoway) {
    if (!reuse_flows)
      {
	delete forwardFlow;
	delete backwardFlow;
	slowmo->forwardFlow = m_flowSource->buildFlow(slowmo, left, right);
	slowmo->backwardFlow = m_flowSource->buildFlow(slowmo, right, left);
	forwardFlow = (FlowField_sV *) slowmo->forwardFlow;
	backwardFlow = (FlowField_sV *) slowmo->backwardFlow;
      }

    Interpolate_sV::twowayFlow(slowmo, left, right, forwardFlow, backwardFlow, pos, out);
  } else if (interpolation == InterpolationType_TwowayNew) {
    Interpolate_sV::newTwowayFlow(slowmo, left, right, forwardFlow, backwardFlow, pos, out);
    delete forwardFlow;
    delete backwardFlow;
  } else if (interpolation == InterpolationType_Forward) {
    Interpolate_sV::forwardFlow(slowmo, left, forwardFlow, pos, out);
    delete forwardFlow;
  } else if (interpolation == InterpolationType_ForwardNew) {
    Interpolate_sV::newForwardFlow(slowmo, left, forwardFlow, pos, out);
    delete forwardFlow;
  } else if (interpolation == InterpolationType_Bezier) {
    FlowField_sV *currNext = NULL;
    FlowField_sV *currPrev = NULL;

    g_assert(currPrev != NULL);

    Interpolate_sV::bezierFlow(slowmo, left, right, currPrev, currNext, pos, out);

    delete currNext;
    delete currPrev;
  } else {
    std::cout << "Unsupported interpolation type!" << std::endl;
    g_assert(false);
  }
  return TRUE;
}

extern "C" {
  gboolean interpolate(GstSlowmo *slowmo, GstBuffer *leftFrame, GstBuffer *rightFrame, GstBuffer *out, float pos, gboolean reuse_flows);

  gboolean interpolate(GstSlowmo *slowmo, GstBuffer *leftFrame, GstBuffer *rightFrame, GstBuffer *out, float pos, gboolean reuse_flows)
  {
    return cpp_interpolate(slowmo, leftFrame, rightFrame, out, pos, reuse_flows);
  }
}
