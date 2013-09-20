/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef BEZIERTOOLS_SV_H
#define BEZIERTOOLS_SV_H

//#include "defs_sV.hpp"

#include "QPoint_crack.h"

/**
  Contains common function for working with cubic bézier curves.
  */
class BezierTools_sV
{
public:
    /**
      \brief Interpolates the bézier curve at x value \c x.

      For an injective bézier curve (i.e. only one y value for each x value) this function calculates
      the y value at a given x , which may differ from the time \c t in interpolate().
      */
  static void interpolateAtX(float x, QPointF p0, QPointF p1, QPointF p2, QPointF p3, QPointF *result);
    /**
      \brief Interpolates the bézier curve at time \c t.

      This function interpolates the cubic bézier curve defined by end points \c p0 and \c p3
      and handles \c p1 and \c p2 at time \f$ t \in [0,1] \f$.
      */
  static void interpolate(float t, QPointF p0, QPointF p1, QPointF p2, QPointF p3, QPointF *result);
};

#endif // BEZIERTOOLS_SV_H
