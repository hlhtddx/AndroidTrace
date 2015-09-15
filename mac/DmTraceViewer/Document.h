//
//  Document.h
//  DmTraceViewer
//
//  Created by Frank Zhang on 15/9/7.
//  Copyright (c) 2015年 Frank Zhang. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "GraphView.h"
#import "DetailView.h"

@interface Document : NSDocument

@property (weak) IBOutlet DetailView *mDetailView;
@property (weak) IBOutlet MyThreadLabelView *mThreadLabelView;
@property (weak) IBOutlet MyTimeScaleView *mTimeScaleView;
@property (weak) IBOutlet MyTimeLineView *mTimeLineView;

@end

