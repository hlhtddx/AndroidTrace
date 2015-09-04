//
//  main.cpp
//  AndroidTrace
//
//  Created by Frank Zhang on 15/8/16.
//  Copyright (c) 2015 Frank Zhang. All rights reserved.
//

#include <iostream>
#include "DmTraceData.hpp"
#include "DmTraceControl.hpp"

int main(int argc, const char * argv[]) {
	if(argc < 2) {
		std::cerr << "Please input trace file name" << std::endl;
		exit(-1);
	}

    bool regression = false;

    if (argc > 2 && strcmp(argv[2], "--regression") == 0) {
        regression = true;
    }

    Android::DmTraceData *reader = nullptr;
    Android::DmTraceControl *view = nullptr;
	
	const char* fileName = argv[1];
    try {
        reader = new Android::DmTraceData(fileName, regression);
        view = new Android::DmTraceControl(reader);
    } catch (Android::GeneralException &e) {
        printf("Catch a generic exception: %s\n", e.getDescription());
    }
    if (view) delete view;
    if (reader) delete reader;
    return 0;
}
