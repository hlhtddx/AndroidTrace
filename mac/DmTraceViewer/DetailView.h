//
//  DetailView.h
//  AndroidTrace
//
//  Created by Frank Zhang on 15/9/12.
//  Copyright (c) 2015年 Frank Zhang. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Android/DmTraceData.hpp"
using namespace Android;

@interface DetailView : NSTableView
- (void)setDocument:(DmTraceData*)document;

@end
