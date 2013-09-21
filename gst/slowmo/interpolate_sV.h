/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef __SLOWMO_INTERPOLATE_H__
#define __SLOWMO_INTERPOLATE_H__

#include <gstslowmo.h>
#include "QPoint_crack.h"

//class QImage;
class FlowField_sV;

/**
  \short Provides interpolation methods between frames
  \todo Half resolution for optical flow
  */
class Interpolate_sV {

 public:
    /** \fn forwardFlow()
      Interpolates a frame using only the flow from the first to the second frame.
      This algorithm is simplified and only partly correct since it assumes the flow field
      to tell where a pixel came from and not where it went to, which usually leads to artifacts
      like on object boundaries or like objects that do not move as far as they should,
      but is much easier to interpolate.
      */
    /** \fn newForwardFlow
      Like forwardFlow(), but uses the forward flow correctly. This includes more work like
      filling holes if an object expanded (a pixel then becomes larger, or «multiplies», which cannot
      be expressed with usual optical flow (<em>where did the pixel go to?</em> cannot be answered
      since it went to multiple locations). The benefit is that this algorithm works more precisely.
      */
    /** \fn twowayFlow()
      Interpolates a frame using optical flow from the first to the second frame, as well as from the second to the first frame.
      */
    /** \fn newTwowayFlow()
      Like twowayFlow(), but uses forward and backward flow correctly. See also newForwardFlow().
      */
    /**
      \fn interpolate(const QImage &in, float x, float y)
      \brief Interpolates the colour at position <code>(x|y)</code>.

      \c x should fulfil \f$ 0 \leq x < width-1 \f$, same with y, to avoid reading outside the image.
      Not tested inside the function for efficiency reasons.
      */
    static void forwardFlow(GstBuffer *left, const FlowField_sV *flow, float pos, GstBuffer * output);
    static void newForwardFlow(GstBuffer * left, const FlowField_sV *flow, float pos, GstBuffer * output);
    static void twowayFlow(GstBuffer * left, GstBuffer * right, const FlowField_sV *flowForward, const FlowField_sV *flowBackward, float pos, GstBuffer * output);
    static void newTwowayFlow(GstBuffer * left, GstBuffer * right, const FlowField_sV *flowLeftRight, const FlowField_sV *flowRightLeft, float pos, GstBuffer * output);
    static void bezierFlow(GstBuffer * left, GstBuffer * right, const FlowField_sV *flowCurrPrev, const FlowField_sV *flowCurrNext, float pos, GstBuffer *output);
    static void interpolate(GstBuffer * in, float x, float y, QColor *out);

private:
    struct Movement {
        float moveX;
        float moveY;
    };
    struct ColorMatrix4x4 {
        QColor c00, c10, c01, c11;
    };

    static void blend(ColorMatrix4x4& colors, const QColor &blendCol, float posX, float posY);
    static void blend(const QColor& left, const QColor& right, float pos, QColor *out);

};

#endif
