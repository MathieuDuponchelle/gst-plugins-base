/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef ABSTRACTFLOWSOURCE_SV_H
#define ABSTRACTFLOWSOURCE_SV_H

#include <gst/gst.h>
#include "gstslowmo.h"
#include "QPoint_crack.h"
#include <iostream>

class Project_sV;
class FlowField_sV;

class AbstractFlowSource_sV
{
public:
  //    AbstractFlowSource_sV();
    virtual ~AbstractFlowSource_sV() {}

    /** \return The flow field from \c leftFrame to \c rightFrame */
    virtual FlowField_sV* buildFlow(GstSlowmo *slowmo, GstBuffer *left, GstBuffer *right) = 0;
};

#endif // ABSTRACTFLOWSOURCE_SV_H
