/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "bezierTools_sV.h"

#include <cmath>
#include <iostream>

void BezierTools_sV::interpolateAtX(float x, QPointF p0, QPointF p1, QPointF p2, QPointF p3, QPointF *res)
{
    float delta = 1;
    float t = 0;
    int iterations = 10*(p3.x-p0.x);

    for (int i = 0; i < iterations; i++) {
      QPointF tmp;
      interpolate(t+delta, p0, p1, p2, p3, &tmp);
      float plus  = tmp.x;
      interpolate(t-delta, p0, p1, p2, p3, &tmp);
      float minus = tmp.x;
      interpolate(t, p0, p1, p2, p3, &tmp);
      float norm  = tmp.x;
      if ((t+delta) <= 1 && fabs(plus-x) < fabs(norm-x)) {
	t += delta;
      } else if ((t-delta) >= 0 && fabs(minus-x) < fabs(norm-x)) {
	t -= delta;
      }
      delta /= 2;
    }
//    std::cout << "Interpolating at t=" << t << " for x time " << 100*interpolate(t, p0, p1, p2, p3).x << ": " << interpolate(t, p0, p1, p2, p3).y << std::endl;
    return interpolate(t, p0, p1, p2, p3, res);
}

// FIXME
void BezierTools_sV::interpolate(float t, QPointF p0, QPointF p1, QPointF p2, QPointF p3, QPointF *res)
{
  float powed;

  powed = pow(t,0) * pow(1-t, 3);
  p0.x *= powed;
  p0.y *= powed;
  powed = 3 * pow(t,1) * pow(1-t, 2);
  p1.x *= powed;
  p1.y *= powed;
  powed = 3 * pow(t,2) * pow(1-t, 1);
  p2.x *= powed;
  p2.y *= powed;
  powed = pow(t,3) * pow(1-t, 0);
  p3.x *= powed;
  p3.y *= powed;

  res->x = p0.x + p1.x + p2.x + p3.x;
  res->y = p0.y + p1.y + p2.y + p3.y;
}
