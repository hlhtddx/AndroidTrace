//
//  TimeLineView.m
//  AndroidTrace
//
//  Created by Frank Zhang on 15/9/9.
//  Copyright (c) 2015年 Frank Zhang. All rights reserved.
//

#import "GraphView.h"


@implementation MyTraceView
{
    DmTraceControl* mTraceCtrl;
}
- (void)setDmTraceCtrl:(DmTraceControl*)traceControl {
    mTraceCtrl = traceControl;
}

@end


@implementation MyTimeLineView

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];

    [[NSColor greenColor] set];
    [NSBezierPath fillRect: dirtyRect];
}

@end

@implementation MyThreadLabelView

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];

    [[NSColor redColor] set];
    [NSBezierPath fillRect: dirtyRect];
}

@end

@implementation MyTimeScaleView

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];

    [[NSColor yellowColor] set];
    [NSBezierPath fillRect: dirtyRect];
}

@end
