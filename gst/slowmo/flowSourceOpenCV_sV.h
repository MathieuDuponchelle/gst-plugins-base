/*
This file is part of slowmoVideo.
Copyright (C) 2012  Lucas Walter
              2012  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef FLOWSOURCEOPENCV_SV_H
#define FLOWSOURCEOPENCV_SV_H

#include <gst/gst.h>
#include <iostream>
#include "QPoint_crack.h"
#include "abstractFlowSource_sV.h"

class FlowSourceOpenCV_sV : public AbstractFlowSource_sV
{
public:
    FlowSourceOpenCV_sV();
    ~FlowSourceOpenCV_sV() {}

    virtual FlowField_sV* buildFlow(GstSlowmo *slowmo, GstBuffer *leftFrame, GstBuffer *rightFrame);

private:
    std::string m_dirFlowSmall;
    std::string m_dirFlowOrig;

    
    void createDirectories();
};

#endif // FLOWSOURCEOPENCV_SV_H
