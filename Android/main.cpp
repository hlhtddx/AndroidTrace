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
	
	const char* fileName = argv[1];
	
	Android::DmTraceReader *reader = new Android::DmTraceReader(fileName, false);
	Android::TimeLineView *view = new Android::TimeLineView(reader);
	int a;
	std::cin >> a;
	delete view;
	delete reader;
	std::cout << "Hello, World!\n";
    return 0;
}
