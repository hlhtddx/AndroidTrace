//
//  TimeLineView.h
//  AndroidTrace
//
//  Created by Frank Zhang on 15/9/9.
//  Copyright (c) 2015年 Frank Zhang. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Android/DmTraceControl.hpp"
using namespace Android;

class CTimeLineView : public TimeLineView {
public:
    CTimeLineView(DmTraceControl* pParent)
    : TimeLineView(pParent)
    {
    }

    virtual void draw(void* context);
    virtual void redraw();
};

class CThreadLabelView : public ThreadLabelView {
public:
    CThreadLabelView(DmTraceControl* pParent)
    : ThreadLabelView(pParent)
    {
    }

    virtual void draw(void* context);
    virtual void redraw();
};

class CTimeScaleView : public TimeScaleView {
public:
    CTimeScaleView(DmTraceControl* pParent)
    : TimeScaleView(pParent)
    {
    }

    virtual void draw(void* context);
    virtual void redraw();
};


@interface MyTraceView : NSView
{
    NSColor *grayColor;
    NSColor *darkColor;
}

- (void)setDmTraceCtrl:(DmTraceControl*)traceControl;
@end

@interface MyTimeLineView : MyTraceView
@end

@interface MyThreadLabelView : MyTraceView
@end

@interface MyTimeScaleView : MyTraceView
@end
