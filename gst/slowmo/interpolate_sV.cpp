/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "interpolate_sV.h"
#include "flowField_sV.h"
#include "flowTools_sV.h"
#include "sourceField_sV.h"
#include "vector_sV.h"
#include "bezierTools_sV.h"

#ifdef WINDOWS
#include <math.h>
#else
#include <cmath>
#endif

// #include <QDebug>
// #include <QImage>
// #include <QColor>

#define CLAMP1(x) ( ((x) > 1.0) ? 1.0 : (x) )

//FIXME
#ifndef CLAMP
#define CLAMP(x,min,max) (  ((x) < (min)) ? (min) : ( ((x) > (max)) ? (max) : (x) )  )
#endif

#define INTERPOLATE
//#define FIX_FLOW
#define FIX_BORDERS
//#define DEBUG_I

enum ColorComponent { CC_Red, CC_Green, CC_Blue };

// FIXME : KILL ME PLEASE
#define WIDTH 320
#define HEIGHT 240
#define PIXEL_STRIDE 3

static void
get_pixel_at(guint8 *data, int x, int y, QColor *res)
{
  int lstride = PIXEL_STRIDE * WIDTH;

  res->redF = data[lstride * y + x * PIXEL_STRIDE];
  res->greenF = data[lstride * y + x * PIXEL_STRIDE + 1];
  res->blueF = data[lstride * y + x * PIXEL_STRIDE + 2];
  // FIXME -> accept RGBA;
  //  res.alphaF = data[pixel_stride * width * y + x * pixel_stride + 3];
}

inline
float interpR(const QColor cols[2][2], float x, float y)
{
    return (1-x)*(1-y) * cols[0][0].redF
        + x*(1-y) * cols[1][0].redF
        + y*(1-x) * cols[0][1].redF
        + x*y * cols[1][1].redF;
}

inline
float interpG(const QColor cols[2][2], float x, float y)
{
    return (1-x)*(1-y) * cols[0][0].greenF
        + x*(1-y) * cols[1][0].greenF
        + y*(1-x) * cols[0][1].greenF
        + x*y * cols[1][1].greenF;
}

inline
float interpB(const QColor cols[2][2], float x, float y)
{
    return (1-x)*(1-y) * cols[0][0].blueF
        + x*(1-y) * cols[1][0].blueF
        + y*(1-x) * cols[0][1].blueF
        + x*y * cols[1][1].blueF;
}

void Interpolate_sV::interpolate(GstBuffer *in, float x, float y, QColor *out)
{
  GstMapInfo info;

  gst_buffer_map(in, &info, GST_MAP_READ);

#ifdef DEBUG_I
    if (x >= in.width()-1 || y >= in.height()-1) {
        Q_ASSERT(false);
    }
#endif
    QColor carr[2][2];
    int floorX = floor(x);
    int floorY = floor(y);
    get_pixel_at(info.data, floorX, floorY, &(carr[0][0]));
    get_pixel_at(info.data, floorX, floorY+1, &(carr[0][1]));
    get_pixel_at(info.data, floorX+1, floorY, &(carr[1][0]));
    get_pixel_at(info.data, floorX+1, floorY+1, &(carr[1][1]));

    float dx = x - floorX;
    float dy = y - floorY;
    out->redF = interpR(carr, dx, dy);
    out->greenF = interpG(carr, dx, dy);
    out->blueF = interpB(carr, dx, dy);

    gst_buffer_unmap(in, &info);
}

/// validated. correct.
void
Interpolate_sV::blend(const QColor &left, const QColor &right, float pos, QColor *out)
{
    g_assert (pos >= 0 && pos <= 1);

    float r = (1-pos)*left.redF   + pos*right.redF;
    float g = (1-pos)*left.greenF + pos*right.greenF;
    float b = (1-pos)*left.blueF  + pos*right.blueF;
    float a = (1-pos)*left.alphaF + pos*right.alphaF;
    r = CLAMP(r,0.0,1.0);
    g = CLAMP(g,0.0,1.0);
    b = CLAMP(b,0.0,1.0);
    a = CLAMP(a,0.0,1.0);

    out->redF = r;
    out->greenF = g;
    out->blueF = b;
    out->alphaF = a;
}

void Interpolate_sV::blend(ColorMatrix4x4 &c, const QColor &blendCol, float posX, float posY)
{
    g_assert(posX >= 0 && posX <= 1);
    g_assert(posY >= 0 && posY <= 1);

    if (c.c00.alphaF == 0) { c.c00 = blendCol; }
    else { blend(c.c00, blendCol, std::sqrt((1-posX) * (1-posY)), &(c.c00)); }

    if (c.c10.alphaF == 0) { c.c10 = blendCol; }
    else { blend(c.c10, blendCol, std::sqrt( posX * (1-posY)), &(c.c10)); }

    if (c.c01.alphaF == 0) { c.c01 = blendCol; }
    else { blend(c.c01, blendCol, std::sqrt((1-posX) * posY), &(c.c01)); }

    if (c.c11.alphaF == 0) { c.c11 = blendCol; }
    else { blend(c.c11, blendCol, std::sqrt(posX * posY), &(c.c11)); }
}

static void
set_pixel_at(GstBuffer *frame, guint8 *data, int x, int y, float r, float g, float b)
{
  int lstride = PIXEL_STRIDE * WIDTH;

  data[lstride * y + x * PIXEL_STRIDE] = r;
  data[lstride * y + x * PIXEL_STRIDE + 1] = g;
  data[lstride * y + x * PIXEL_STRIDE + 2] = b;
}

void Interpolate_sV::twowayFlow(GstBuffer *left, GstBuffer *right, const FlowField_sV *flowForward, const FlowField_sV *flowBackward, float pos, GstBuffer *output)
{
#ifdef INTERPOLATE
    const float Wmax = WIDTH - 1.0001; // A little less than the maximum pixel to avoid out of bounds when interpolating
    const float Hmax = HEIGHT - 1.0001;
    float posX, posY;
#endif

    QColor colLeft, colRight;
    float r,g, b;
    GstMapInfo info;
    Interpolate_sV::Movement forward, backward;

    gst_buffer_map(output, &info, GST_MAP_WRITE);
    for (int y = 0; y < HEIGHT; y++) {
      for (int x = 0; x < WIDTH; x++) {
            forward.moveX = flowForward->x(x, y);
            forward.moveY = flowForward->y(x, y);

            backward.moveX = flowBackward->x(x, y);
            backward.moveY = flowBackward->y(x, y);

#ifdef INTERPOLATE
            posX = x - pos*forward.moveX;
            posY = y - pos*forward.moveY;
            posX = CLAMP(posX, 0, Wmax);
            posY = CLAMP(posY, 0, Hmax);
            interpolate(left, posX, posY, &colLeft);

            posX = x - (1-pos)*backward.moveX;
            posY = y - (1-pos)*backward.moveY;
            posX = CLAMP(posX, 0, Wmax);
            posY = CLAMP(posY, 0, Hmax);
            interpolate(right, posX, posY, &colRight);
#else
            colLeft = QColor(left.pixel(x - pos*forward.moveX, y - pos*forward.moveY));
            colRight = QColor(right.pixel(x - (1-pos)*backward.moveX , y - (1-pos)*backward.moveY));
#endif
            r = (1-pos)*colLeft.redF + pos*colRight.redF;
            g = (1-pos)*colLeft.greenF + pos*colRight.greenF;
            b = (1-pos)*colLeft.blueF + pos*colRight.blueF;

	    set_pixel_at(output, info.data, x, y, CLAMP1(r), CLAMP1(g), CLAMP1(b));
        }
    }
    gst_buffer_unmap(output, &info);
}


void Interpolate_sV::newTwowayFlow(GstBuffer *left, GstBuffer *right,
                                   const FlowField_sV *flowLeftRight, const FlowField_sV *flowRightLeft,
                                   float pos, GstBuffer *output)
{
    const int W = WIDTH;
    const int H = HEIGHT;


    SourceField_sV leftSourcePixel(flowLeftRight, pos);
    leftSourcePixel.inpaint();
    SourceField_sV rightSourcePixel(flowRightLeft, 1-pos);
    rightSourcePixel.inpaint();

    float aspect = 1 - (.5 + std::cos(M_PI*pos)/2);

#if defined(FIX_FLOW)
    FlowField_sV diffField(flowLeftRight->width(), flowLeftRight->height());
    FlowTools_sV::difference(*flowLeftRight, *flowRightLeft, diffField);
    float diffSum;
    float tmpAspect;
#endif

#ifdef FIX_BORDERS
    bool leftOk;
    bool rightOk;
#endif


    float fx, fy;
    QColor colLeft, colRight;
    GstMapInfo info;

    gst_buffer_map(output, &info, GST_MAP_WRITE);
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {

#ifdef FIX_BORDERS
            fx = leftSourcePixel.at(x,y).fromX;
            fy = leftSourcePixel.at(x,y).fromY;
            if (fx >= 0 && fx < W-1
                    && fy >= 0 && fy < H-1) {
	      interpolate(left, fx, fy, &colLeft);
                leftOk = true;
            } else {
                fx = leftSourcePixel.at(x,y).fromX;
                fy = leftSourcePixel.at(x,y).fromY;
                fx = CLAMP(fx, 0, W-1.01);
                fy = CLAMP(fy, 0, H-1.01);
                interpolate(left, fx, fy, &colLeft);
                leftOk = false;
            }

            fx = rightSourcePixel.at(x,y).fromX;
            fy = rightSourcePixel.at(x,y).fromY;
            if (fx >= 0 && fx < W-1
                    && fy >= 0 && fy < H-1) {
	      interpolate(right, fx, fy, &colRight);
                rightOk = true;
            } else {
		colRight.redF = 0;
		colRight.greenF = 255;
		colRight.blueF = 0;
                rightOk = false;
            }

            if (leftOk && rightOk) {
	      QColor blended;
	      blend(colLeft, colRight, aspect, &blended);
		set_pixel_at(output, info.data, x, y, blended.redF, blended.greenF, blended.blueF);
            } else if (rightOk) {
	      set_pixel_at(output, info.data, x, y, colRight.redF, colRight.greenF, colRight.blueF);
//                output.setPixel(x,y, qRgb(255, 0, 0));
            } else if (leftOk) {
	      set_pixel_at(output, info.data, x, y, colLeft.redF, colLeft.greenF, colLeft.blueF);
//                output.setPixel(x,y, qRgb(0, 255, 0));
            } else {
	      set_pixel_at(output, info.data, x, y, colLeft.redF, colLeft.greenF, colLeft.blueF);
            }
#else
            fx = leftSourcePixel.at(x,y).fromX;
            fy = leftSourcePixel.at(x,y).fromY;
            fx = CLAMP(fx, 0, W-1.01);
            fy = CLAMP(fy, 0, H-1.01);
            interpolate(left, fx, fy, &colLeft);

#ifdef FIX_FLOW
            diffSum = diffField.x(fx, fy)+diffField.y(fx, fy);
            if (diffSum > 5) {
                tmpAspect = 0;
            } else if (diffSum < -5) {
                tmpAspect = 1;
            } else {
                tmpAspect = aspect;
            }
#endif

            fx = rightSourcePixel.at(x,y).fromX;
            fy = rightSourcePixel.at(x,y).fromY;
            fx = CLAMP(fx, 0, W-1.01);
            fy = CLAMP(fy, 0, H-1.01);
            interpolate(right, fx, fy, &colRight);

#ifdef FIX_FLOW
            diffSum = diffField.x(fx, fy)+diffField.y(fx, fy);
            if (diffSum < 5) {
                tmpAspect = 0;
            } else if (diffSum > -5) {
                tmpAspect = 1;
            }
#endif

#ifdef FIX_FLOW
	    QColor blended;
	    blend(colLeft, colRight, tmpAspect, &blended)
            output.setPixel(x,y, blended.rgba());
#else
	    QColor blended;
	    blend(colLeft, colRight, aspect, &blended)	    
            output.setPixel(x,y, blended.rgba());
#endif

#endif
        }
    }
    gst_buffer_unmap(output, &info);
}

void Interpolate_sV::forwardFlow(GstBuffer *left, const FlowField_sV *flow, float pos, GstBuffer *output)
{
  //    qDebug() << "Interpolating flow at offset " << pos;
#ifdef INTERPOLATE
    float posX, posY;
    const float Wmax = WIDTH-1.0001;
    const float Hmax = HEIGHT-1.0001;
#endif

    QColor colOut;
    Interpolate_sV::Movement forward;    
    GstMapInfo info;

    gst_buffer_map(output, &info, GST_MAP_WRITE);

    for (int y = 0; y < HEIGHT; y++) {
      for (int x = 0; x < WIDTH; x++) {
            // Forward flow from the left to the right image tells for each pixel in the right image
            // from which location in the left image the pixel has come from.
            forward.moveX = flow->x(x, y);
            forward.moveY = flow->y(x, y);

#ifdef INTERPOLATE
            posX = x - pos*forward.moveX;
            posY = y - pos*forward.moveY;
	    posX = CLAMP(posX, 0, Wmax);
	    posY = CLAMP(posY, 0, Hmax);
	    interpolate(left, posX, posY, &colOut);
#else
            colOut = QColor(left.pixel(x - pos*forward.moveX, y - pos*forward.moveY));
#endif
	    set_pixel_at(output, info.data, x, y, colOut.redF, colOut.greenF, colOut.blueF);
	}
    }
    gst_buffer_unmap(output, &info);
}

void Interpolate_sV::newForwardFlow(GstBuffer *left, const FlowField_sV *flow, float pos, GstBuffer *output)
{
    const int W = WIDTH;
    const int H = HEIGHT;
    GstMapInfo info;

    // Calculate the source flow field
    SourceField_sV field(flow, pos);
    field.inpaint();

    gst_buffer_map(output, &info, GST_MAP_WRITE);

    // Draw the pixels
    float fx, fy;
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
	  QColor colLeft;
            // Since interpolate() uses the floor()+1 values,
            // set the maximum to a little less than size-1
            // such that the pixel always lies inside.
            fx = field.at(x,y).fromX;
            fx = CLAMP(fx, 0, W-1.01);
            fy = field.at(x,y).fromY;
            fy = CLAMP(fy, 0, H-1.01);
	    interpolate(left, fx, fy, &colLeft);
	    set_pixel_at(output, info.data, x, y, colLeft.redF, colLeft.greenF, colLeft.blueF);
        }
    }
    gst_buffer_unmap(output, &info);
}


/**
  \todo fix bézier interpolation
  \code
      C prev
     /   /
    /   /
   /   /
  A curr
   \
    \
     B next (can be NULL)
  \endcode
  */
void Interpolate_sV::bezierFlow(GstBuffer *prev, GstBuffer *right, const FlowField_sV *flowPrevCurr, const FlowField_sV *flowCurrNext, float pos, GstBuffer *output)
{
    const float Wmax = WIDTH - 1.0001;
    const float Hmax = HEIGHT - 1.0001;

    Vector_sV a, b, c, tmpA, tmpB, tmpC, tmpD;
    Vector_sV Ta, Sa;
    float dist;

    QColor colOut;

    GstMapInfo info;

    gst_buffer_map(output, &info, GST_MAP_WRITE);

    for (int y = 0; y < HEIGHT; y++) {
      for (int x = 0; x < WIDTH; x++) {

            a = Vector_sV(x, y);
            // WHY minus?
	    a.add(Vector_sV(flowPrevCurr->x(x, y), flowPrevCurr->y(x, y)), &c);
            if (flowCurrNext != NULL) {
	      a.add(Vector_sV(flowCurrNext->x(x,y), flowCurrNext->y(x,y)), &b);
            } else {
                b = a;
            }

	    b.sub(a, &tmpA);
	    c.sub(a, &tmpB);
            dist = tmpA.length() + tmpB.length();
            if (dist > 0) {
	      c.sub(b, &tmpC);
	      tmpC.mul(tmpA.length() / dist, &tmpD);
	      b.add(tmpD, &Ta);
	      Ta.sub(a, &Sa);
	      Sa = Sa.rotate90();
	      Sa += a;
            } else {
                Sa = a;
            }
#ifdef DEBUG_I
            Sa = a;
#endif

            QPointF position;
	    QPointF cPoint, saPoint, aPoint;
	    c.toQPointF(&cPoint);
	    Sa.toQPointF(&saPoint);
	    a.toQPointF(&cPoint);
	    BezierTools_sV::interpolate(pos, cPoint, cPoint, saPoint, aPoint, &position);
            position.x = x - pos*flowPrevCurr->x(x,y);
            position.y = y - pos*flowPrevCurr->y(x,y);
            position.x = CLAMP(position.x, 0, Wmax);
            position.y = CLAMP(position.y, 0, Hmax);

#ifdef DEBUG_I
//            if (x == 100 && y == 100 && false) {
//                qDebug() << "Interpolated from " << toString(c.toQPointF()) << ", " << toString(a.toQPointF()) << ", "
//                         << toString(b.toQPointF()) << " at " << pos << ": " << toString(position);
//            }
            if (y % 4 == 0) {
                position.x = x;
                position.y = y;
            }
#endif

            interpolate(prev, position.x, position.y, &colOut);

#ifdef DEBUG_I
            if (y % 4 == 1 && x % 2 == 0) {
                colOut = right.pixel(x, y);
            }
#endif
	    set_pixel_at(output, info.data, x, y, colOut.redF, colOut.greenF, colOut.blueF);
        }
    }

    gst_buffer_unmap(output, &info);

    /*
    for (int y = 0; y < prev.height(); y++) {
        for (int x = 1; x < prev.width()-1; x++) {
            if (qAlpha(output.pixel(x,y)) == 0
                    && qAlpha(output.pixel(x-1,y)) > 0
                    && qAlpha(output.pixel(x+1,y)) > 0) {
                output.setPixel(x,y, qRgba(
                                    (qRed(output.pixel(x-1,y)) + qRed(output.pixel(x+1,y)))/2,
                                    (qGreen(output.pixel(x-1,y)) + qGreen(output.pixel(x+1,y)))/2,
                                    (qBlue(output.pixel(x-1,y)) + qBlue(output.pixel(x+1,y)))/2,
                                    (qAlpha(output.pixel(x-1,y)) + qAlpha(output.pixel(x+1,y)))/2
                                    ));
            }
        }
    }
    for (int x = 0; x < prev.width(); x++) {
        for (int y = 1; y < prev.height()-1; y++) {
            if (qAlpha(output.pixel(x,y)) == 0
                    && qAlpha(output.pixel(x,y-1)) > 0
                    && qAlpha(output.pixel(x,y+1)) > 0) {
                output.setPixel(x,y, qRgba(
                                    (qRed(output.pixel(x,y-1)) + qRed(output.pixel(x,y+1)))/2,
                                    (qGreen(output.pixel(x,y-1)) + qGreen(output.pixel(x,y+1)))/2,
                                    (qBlue(output.pixel(x,y-1)) + qBlue(output.pixel(x,y+1)))/2,
                                    (qAlpha(output.pixel(x,y-1)) + qAlpha(output.pixel(x,y+1)))/2
                                    ));
            }
        }
    }
    */
}
