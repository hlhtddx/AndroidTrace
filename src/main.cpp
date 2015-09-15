//
//  main.cpp
//  AndroidTrace
//
//  Created by Frank Zhang on 15/8/16.
//  Copyright (c) 2015 Frank Zhang. All rights reserved.
//

#include <iostream>
#include "DmTraceModel.hpp"
#include "DmTraceControl.hpp"

using namespace Android;

class MyTimeLineView : public TimeLineView
{
public:
    MyTimeLineView(DmTraceControl* pParent)
        : TimeLineView(pParent)
    {}
    virtual void draw(void* context) {};
    virtual void redraw() {};
};

class MyTimescaleView : public TimeScaleView
{
public:
    MyTimescaleView(DmTraceControl* pParent)
        : TimeScaleView(pParent)
    {}
    virtual void draw(void* context) {};
    virtual void redraw() {};
};

class MyThreadLabelView : public ThreadLabelView
{
public:
    MyThreadLabelView(DmTraceControl* pParent)
        : ThreadLabelView(pParent)
    {
    }
    virtual void draw(void* context) {};
    virtual void redraw() {};
};

class MyTraceControl : public DmTraceControl
{
public:
    MyTraceControl()
    {
        mTimeLine = new MyTimeLineView(this);
        mTimescale = new MyTimescaleView(this);
        mThreadLabel = new MyThreadLabelView(this);
    }
    virtual void redraw()
    {
        printf("Redraw\n");
    }

    virtual void draw(void* context)
    {
        printf("Draw it\n");
    }
};

int main(int argc, const char * argv[]) {
	if(argc < 2) {
		std::cerr << "Please input trace file name" << std::endl;
		exit(-1);
	}

    bool regression = false;

    if (argc > 2 && strcmp(argv[2], "--regression") == 0) {
        regression = true;
    }
	
	const char* fileName = argv[1];
    try {
        DmTraceModel reader;
        reader.open(fileName, regression);

        MyTraceControl view;
        view.setData(&reader);

    } catch (Android::GeneralException &e) {
        printf("Catch a generic exception: %s\n", e.getDescription());
    }
    return 0;
}
