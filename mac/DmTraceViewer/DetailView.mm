//
//  DetailView.m
//  AndroidTrace
//
//  Created by Frank Zhang on 15/9/12.
//  Copyright (c) 2015年 Frank Zhang. All rights reserved.
//

#import "DetailView.h"

@implementation DetailView
{
    DmTraceData* mTraceData;
}

- (void)setDocument:(DmTraceData*)document {
    mTraceData = document;
}

@end
