//
//  DmTraceWrapper.m
//  AndroidTrace
//
//  Created by Frank Zhang on 9/15/15.
//  Copyright (c) 2015 Frank Zhang. All rights reserved.
//

#import "DmTraceWrapper.h"
#include <iostream>

@implementation DmTraceWrapper
- (BOOL)open: (const char*)fileName
{
    try {
        model = new DmTraceData;
        model->open(fileName, false);
        control = new DmTraceControl;
        control->setData(model);
    } catch (GeneralException& e) {
        std::cout << "Failed to open file, cause is " << e.getDescription() << std::endl;
        if(control) {
            delete control;
            control = nullptr;
        }
        if(model) {
            delete model;
            model = nullptr;
        }
        return NO;
    }
    return YES;
}

- (void)close
{
    if(control) {
        delete control;
        control = nullptr;
    }
    if(model) {
        delete model;
        model = nullptr;
    }
}

- (BOOL)isOpen
{
    if(control != nullptr) {
        return YES;
    }
    return NO;
}

@end
