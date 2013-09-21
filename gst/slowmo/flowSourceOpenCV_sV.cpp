/*
This file is part of slowmoVideo.
Copyright (C) 2012  Lucas Walter
              2012  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "flowSourceOpenCV_sV.h"
#include "flowField_sV.h"
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <fstream>

#include <sys/time.h>

using namespace cv;
using namespace std;

FlowSourceOpenCV_sV::FlowSourceOpenCV_sV()
{
}

static void drawOptFlowMap(const Mat& flow, Mat& cflowmap, int step,
                    double, const Scalar& color, FlowField_sV *field)
{
  cv::Mat log_flow, log_flow_neg;
  //log_flow = cv::abs( flow/3.0 );
  cv::log(cv::abs(flow) * 3 + 1, log_flow);
  cv::log(cv::abs(flow * (-1.0)) * 3 + 1, log_flow_neg);
  const float scale = 64.0;
  const float offset = 128.0;
  int ofst = 0;
  float *data = (float *) (field->data());

  float max_flow = 0.0;

  for(int y = 0; y < cflowmap.rows; y += step)
    for(int x = 0; x < cflowmap.cols; x += step)
      {
	const Point2f& fxyo = flow.at<Point2f>(y, x);

	data[ofst] = (float) fxyo.x;
	ofst += 1;
	data[ofst] = (float) fxyo.y;
	ofst += 1;

	Point2f& fxy = log_flow.at<Point2f>(y, x);
	const Point2f& fxyn = log_flow_neg.at<Point2f>(y, x);

	if (fxyo.x < 0) {
	  fxy.x = -fxyn.x;
	}
	if (fxyo.y < 0) {
	  fxy.y = -fxyn.y;
	}


	cv::Scalar col = cv::Scalar(offset + fxy.x*scale, offset + fxy.y*scale, offset);
	//line(cflowmap, Point(x,y), Point(cvRound(x+fxy.x), cvRound(y+fxy.y)),
	//     color);
	circle(cflowmap, Point(x,y), 0, col, -1);

	if (fabs(fxy.x) > max_flow) max_flow = fabs(fxy.x);
	if (fabs(fxy.y) > max_flow) max_flow = fabs(fxy.y);
      }

  std::cout << max_flow << " max flow" << std::endl;
}

FlowField_sV* FlowSourceOpenCV_sV::buildFlow(GstSlowmo *slowmo, GstBuffer *leftFrame, GstBuffer *rightFrame)
{
  Mat flow, cflow, prevgray, gray;
  GstMapInfo left_info, right_info;
  timeval t1, t2;
  double elapsedTime;
  FlowField_sV *field;

  gettimeofday(&t1, NULL);

  field = new FlowField_sV(slowmo->width, slowmo->height);

  gst_buffer_map(leftFrame, &left_info, GST_MAP_READ);
  gst_buffer_map(rightFrame, &right_info, GST_MAP_READ);
  Mat left(Size(slowmo->width, slowmo->height), CV_8UC3, left_info.data, Mat::AUTO_STEP);
  Mat right(Size(slowmo->width, slowmo->height), CV_8UC3, right_info.data, Mat::AUTO_STEP);
  cvtColor(left, prevgray, CV_RGB2GRAY);
  cvtColor(right, gray, CV_RGB2GRAY);

  gst_buffer_unmap(leftFrame, &left_info);
  gst_buffer_unmap(rightFrame, &right_info);

  // prevgray = imread(project()->frameSource()->framePath(leftFrame, frameSize).toStdString(), 0);
  // gray = imread(project()->frameSource()->framePath(rightFrame, frameSize).toStdString(), 0);
  {
    if( prevgray.data )
      {
	const float pyrScale = 0.5;
	const float levels = 3;
	const float winsize = 15;
	const float iterations = 8;
	const float polyN = 5;
	const float polySigma = 1.2;
	const int flags = 0;
	// TBD need sliders for all these parameters
	calcOpticalFlowFarneback(
				 prevgray, gray,
				 //gray, prevgray,  // TBD this seems to match V3D output better but a sign flip could also do that
				 flow,
				 pyrScale, //0.5,
				 levels, //3,
				 winsize, //15,
				 iterations, //3,
				 polyN, //5,
				 polySigma, //1.2,
				 flags //0
				 );
	cvtColor(prevgray, cflow, CV_GRAY2BGR);
	//drawOptFlowMap(flow, cflow, 16, 1.5, CV_RGB(0, 255, 0));
	drawOptFlowMap(flow, cflow, 1, 1.5, CV_RGB(0, 255, 0), field);
	//imshow("flow", cflow);
	//imwrite(argv[4],cflow);
      }
  }

  gettimeofday(&t2, NULL);

  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;

  GST_DEBUG("elapsed time for one frame in calculating flow : %lf milliseconds", elapsedTime);

  return field;
}
