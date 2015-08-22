//
//  main.cpp
//  AndroidTrace
//
//  Created by 张航 on 15/8/16.
//  Copyright (c) 2015年 Frank Zhang. All rights reserved.
//

#include <iostream>
#include "DmTraceReader.hpp"
#include "TimeLineView.hpp"

int main(int argc, const char * argv[]) {
	if(argc < 2) {
		std::cerr << "Please input trace file name" << std::endl;
		exit(-1);
	}

    bool regression = false;

    if (argc > 2 && strcmp(argv[2], "--regression") == 0) {
        regression = true;
    }

    Android::DmTraceReader *reader = nullptr;
    Android::TimeLineView *view = nullptr;
	
	const char* fileName = argv[1];
    try {
        reader = new Android::DmTraceReader(fileName, regression);
        view = new Android::TimeLineView(reader);
        delete view;
        delete reader;
    } catch (Android::GeneralException &e) {
        printf("Catch a generic exception: %s\n", e.getDescription());
    }
    if (view) delete view;
    if (reader) delete reader;
    return 0;
}
