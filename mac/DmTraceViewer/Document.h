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
@property (weak) IBOutlet NSScrollView *mScrollView;
@property (weak) IBOutlet MyTimeLineView *mTimeLineView;
@property (weak) IBOutlet DetailView *mDetailView;

@end

