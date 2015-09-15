//
//  DmTraceWrapper.h
//  AndroidTrace
//
//  Created by Frank Zhang on 9/15/15.
//  Copyright (c) 2015 Frank Zhang. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Android/DmTraceControl.hpp"
using namespace Android;

@interface DmTraceWrapper : NSObject
{
    DmTraceData *model;
    DmTraceControl *control;
}

- (BOOL)open: (const char*)fileName;
- (void)close;
- (BOOL)isOpen;

@end
