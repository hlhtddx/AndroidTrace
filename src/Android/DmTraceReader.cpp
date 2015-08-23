#include "DmTraceReader.hpp"
#include "ColorController.hpp"
#include <fstream>
#include <iostream>

namespace Android {

	DmTraceReader::DmTraceReader(const char* traceFileName, bool regression)
		: mTraceFileName(traceFileName)
	{
		mRegression = regression;
		mTopLevel = new MethodData(0, "(toplevel)");
		mMethodMap.add(0, mTopLevel);
		mContextSwitch = new MethodData(-1, "(context switch)");
		mMethodMap.add(-1, mContextSwitch);
		mSortedThreads = nullptr;
		mSortedMethods = nullptr;
		mClockSource = UNKNOWN;
		generateTrees();
	}

	DmTraceReader::~DmTraceReader()
	{
		if (mSortedThreads != nullptr) {
			delete mSortedThreads;
		}
		if (mSortedMethods != nullptr) {
			delete mSortedMethods;
		}
		for (auto it = mMethodMap.begin(); it != mMethodMap.end(); it++) {
			delete it->second;
		}
		for (auto it = mThreadMap.begin(); it != mThreadMap.end(); it++) {
			delete it->second;
		}
	}

	void DmTraceReader::generateTrees()
	{
		std::cerr << "Parse keys" << std::endl;
		filepos offset = parseKeys();
		std::cerr << "Parse data" << std::endl;
		parseData(offset);
		std::cerr << "generateSegments" << std::endl;
		generateSegments();
		std::cerr << "analyzeData" << std::endl;
		analyzeData();
		std::cerr << "done" << std::endl;
		ColorController::assignMethodColors(mSortedMethods);
	}
	//
	//ProfileProvider* DmTraceReader::getProfileProvider()
	//{
	//    if(mProfileProvider == nullptr)
	//        mProfileProvider = new ProfileProvider(this);
	//
	//    return mProfileProvider;
	//}
	//

	void DmTraceReader::readDataFileHeader(ByteBuffer* buffer)
	{
		int magic = buffer->getInt();
		if (magic != TRACE_MAGIC) {
			throw magic;
		}
		int version = buffer->getShort();
		if (version != mVersionNumber) {
			throw version;
		}
		if ((version < 1) || (version > 3)) {
			throw version;
		}
		int offsetToData = buffer->getShort() - 16;
		buffer->getLong();
		if (version == 1) {
			mRecordSize = 9;
		}
		else if (version == 2) {
			mRecordSize = 10;
		}
		else {
			mRecordSize = buffer->getShort();
			offsetToData -= 2;
		}
		buffer->skip(offsetToData);
	}

	ByteBuffer* DmTraceReader::mapFile(const char* filename, filepos offset)
	{
		char* buffer = nullptr;
		try {
			std::ifstream in(filename, std::ios::binary | std::ios::in);
			in.seekg(0, std::ios_base::end);
			filepos size = in.tellg() - offset;

			char* buffer = new char[size];
			in.exceptions(std::ios::eofbit | std::ios::failbit | std::ios::badbit);
			in.seekg(offset, std::ios_base::beg);
			in.read(buffer, size);

			return new ByteBuffer(buffer, size);
		}
		catch(...)
		{
			if (buffer) {
				delete[] buffer;
			}
			return nullptr;
		}
	}

	void DmTraceReader::parseData(filepos offset) /* throws(IOException) */
	{
		ByteBuffer* buffer = mapFile(mTraceFileName.c_str(), offset);
        if (buffer == nullptr) {
            throw GeneralException("Failed to open file");
        }

        readDataFileHeader(buffer);
		TraceActionList lTrace;
		TraceActionList *trace = nullptr;
		if (mClockSource == THREAD_CPU) {
			trace = &lTrace;
		}
		bool haveThreadClock = mClockSource != WALL;
		bool haveGlobalClock = mClockSource != THREAD_CPU;
		ThreadData* prevThreadData = nullptr;

		while (!buffer->end()) {
			id_type threadId;
			id_type methodId;
			uint32_t threadTime = 0;
			uint32_t globalTime = 0;
			try {
				int recordSize = mRecordSize;
				if (mVersionNumber == 1) {
					threadId = buffer->getUByte();
					recordSize--;
				}
				else {
					threadId = buffer->getUShort();
					recordSize -= 2;
				}
				methodId = buffer->getUInt();
				recordSize -= 4;

				ClockSource v = mClockSource;
				if (v == WALL) {
					threadTime = 0LL;
					globalTime = buffer->getUInt();
					recordSize -= 4;
				}
				else if (v == DUAL) {
					threadTime = buffer->getUInt();
					globalTime = buffer->getUInt();
					recordSize -= 8;
				} if (((v == THREAD_CPU) || ((v != WALL) && (v != DUAL) && (v != THREAD_CPU)))) {
					threadTime = buffer->getInt();
					globalTime = 0LL;
					recordSize -= 4;
				}

				buffer->skip(recordSize);
			}
			catch (BoundaryException& e) {
				e.getDescription();
				break;
			}

			unsigned char methodAction = methodId & METHOD_ACTION_MASK;
			methodId &= METHOD_ID_MASK;

//			TRACE("record: tid=%d, mid=%d, action=%d, ttime=%lld, gtime=%lld\n", threadId, methodId, methodAction, threadTime, globalTime);
			MethodData* methodData = NULL;
			if (mMethodMap.find(methodId) == mMethodMap.end()) {
				std::stringstream ss;
				ss << "0x" << std::ios::hex << methodId;
				methodData = new MethodData(methodId, ss.str().c_str());
				mMethodMap.add(methodId, methodData);
			}
			else {
				methodData = mMethodMap[methodId];
			}

			ThreadData* threadData = NULL;
			if (mThreadMap.find(threadId) == mThreadMap.end()) {
				std::stringstream ss;
				ss << "[" << std::ios::dec << threadId << "]";
				threadData = new ThreadData(threadId, ss.str().c_str(), mTopLevel, &mCallList);
				mThreadMap.add(threadId, threadData);
			}
			else {
				threadData = mThreadMap[threadId];
			}

			uint32_t elapsedGlobalTime = 0;

			if (haveGlobalClock) {
				if (!threadData->mHaveGlobalTime) {
					threadData->mGlobalStartTime = globalTime;
					threadData->mHaveGlobalTime = true;
				}
				else {
					elapsedGlobalTime = globalTime - threadData->mGlobalEndTime;
				}
				threadData->mGlobalEndTime = globalTime;
			}
			if (haveThreadClock) {
				uint32_t elapsedThreadTime = 0;
				if (!threadData->mHaveThreadTime) {
					threadData->mThreadStartTime = threadTime;
					threadData->mThreadCurrentTime = threadTime;
					threadData->mHaveThreadTime = true;
				}
				else {
					elapsedThreadTime = threadTime - threadData->mThreadEndTime;
				}
				threadData->mThreadEndTime = threadTime;
				if (!haveGlobalClock) {
					if ((prevThreadData != nullptr) && (prevThreadData != threadData)) {
						Call* switchCall = prevThreadData->enter(mContextSwitch, trace, &mCallList);
						switchCall->mThreadStartTime = prevThreadData->mThreadEndTime;
						Call* top = threadData->top(&mCallList);
						if (top->getMethodData() == mContextSwitch) {
							threadData->exit(mContextSwitch, trace, &mCallList);
							uint32_t beforeSwitch = elapsedThreadTime / 2;
							top->mThreadStartTime += beforeSwitch;
							top->mThreadEndTime = top->mThreadStartTime;
						}
					}
					prevThreadData = threadData;
				}
				else {
					uint32_t sleepTime = elapsedGlobalTime - elapsedThreadTime;
					if (sleepTime > 100) {
						Call* switchCall = threadData->enter(mContextSwitch, trace, &mCallList);
						uint32_t beforeSwitch = elapsedThreadTime / 2;
						uint32_t afterSwitch = elapsedThreadTime - beforeSwitch;
						switchCall->mGlobalStartTime = (globalTime - elapsedGlobalTime + beforeSwitch);
						switchCall->mGlobalEndTime = (globalTime - afterSwitch);
						switchCall->mThreadStartTime = (threadTime - afterSwitch);
						switchCall->mThreadEndTime = switchCall->mThreadStartTime;
						threadData->exit(mContextSwitch, trace, &mCallList);
					}
				}
				Call* top = threadData->top(&mCallList);
				top->addCpuTime(elapsedThreadTime);
			}
			Call* call;
			switch (methodAction) {
			case METHOD_TRACE_ENTER:
				call = threadData->enter(methodData, trace, &mCallList);
				if (haveGlobalClock) {
					call->mGlobalStartTime = globalTime;
				}
				if (haveThreadClock) {
					call->mThreadStartTime = threadTime;
				}
				break;
			case METHOD_TRACE_EXIT:
			case METHOD_TRACE_UNROLL:
				call = threadData->exit(methodData, trace, &mCallList);
				if (call != nullptr) {
					if (haveGlobalClock) {
						call->mGlobalEndTime = globalTime;
					}
					if (haveThreadClock) {
						call->mThreadEndTime = threadTime;
					}
				}
				break;
			default:
				delete buffer;
				throw "Unrecognized method action: ";
			}
		}

		for (auto it = mThreadMap.begin(); it != mThreadMap.end(); it++) {
			ThreadData* threadData = it->second;
			threadData->endTrace(trace, &mCallList);
		}

		uint32_t globalTime;
		if (!haveGlobalClock) {
			globalTime = 0LL;
			prevThreadData = nullptr;
			for (auto it = trace->begin(); it != trace->end(); it++) {
				TraceAction& traceAction = *it;
				{
					Call* call = mCallList.get(traceAction.mCall);
					ThreadData* threadData = call->getThreadData();

					if (traceAction.mAction == ACTION_ENTER) {
						uint32_t threadTime = call->mThreadStartTime;
						globalTime += call->mThreadStartTime - threadData->mThreadCurrentTime;
						call->mGlobalStartTime = globalTime;
						if (!threadData->mHaveGlobalTime) {
							threadData->mHaveGlobalTime = true;
							threadData->mGlobalStartTime = globalTime;
						}
						threadData->mThreadCurrentTime = threadTime;
					}
					else if (traceAction.mAction == ACTION_EXIT) {
						uint32_t threadTime = call->mThreadEndTime;
						globalTime += call->mThreadEndTime - threadData->mThreadCurrentTime;
						call->mGlobalEndTime = globalTime;
						threadData->mGlobalEndTime = globalTime;
						threadData->mThreadCurrentTime = threadTime;
					}
					
					prevThreadData = threadData;
				}
			}
		}

		mCallList.freeExtra();

		for (auto i = mCallList.size() - 1; i >= 1; i--) {
			Call* call = mCallList.get(i);
			uint32_t realTime = call->mGlobalEndTime - call->mGlobalStartTime;
			call->mExclusiveRealTime = std::max<uint32_t>(realTime - call->mInclusiveRealTime, 0);
			call->mInclusiveRealTime = realTime;
			call->finish(&mCallList);
		}

		mTotalCpuTime = 0;
		mTotalRealTime = 0;

		for (auto it = mThreadMap.begin(); it != mThreadMap.end(); it++) {
			ThreadData* threadData = it->second;
			Call* rootCall = threadData->getRootCall(&mCallList);
			threadData->updateRootCallTimeBounds(&mCallList);
			rootCall->finish(&mCallList);
			mTotalCpuTime += rootCall->mInclusiveCpuTime;
			mTotalRealTime += rootCall->mInclusiveRealTime;
		}

		if (mRegression) {
			printf("totalCpuTime %dus\n", mTotalCpuTime);
			printf("totalRealTime %dus\n", mTotalRealTime);
			dumpThreadTimes();
			dumpCallTimes();
		}
		delete buffer;
	}

	filepos DmTraceReader::parseKeys() /* throws(IOException) */
	{
		filepos offset = 0;
		try {
			std::ifstream in(mTraceFileName, std::ios::binary | std::ios::in);
            in.exceptions(std::ios::eofbit | std::ios::failbit | std::ios::badbit);
            int mode = 0;
			for (;;) {
				String line;
				if (!readLine(in, line)) {
					throw "Key section does not have an *end marker";
				}
				offset += line.size() + 1;
				if (line.front() == '*') {
					if (line == "*version") {
						mode = 0;
					}
					else if (line == "*threads") {
						mode = 1;
					}
					else if (line == "*methods") {
						mode = 2;
					}
					else if (line == "*end") {
						break;
					}
				}
				else {
					switch (mode) {
					case 0:
						mVersionNumber = atoi(line.c_str());
						mode = 4;
						break;
					case 1:
						parseThread(line);
						break;
					case 2:
						parseMethod(line);
						break;
					case 4:
						parseOption(line);
					}
				}
			}
		}
		catch (...) {
		}
		if (mClockSource == UNKNOWN) {
			mClockSource = THREAD_CPU;
		}
		return offset;
	}

	void DmTraceReader::parseOption(const String &line)
	{
		std::stringstream str(line);
		String key, value;
		if (readToken(str, key, '=') && readToken(str, value, '\n')) {
			mPropertiesMap[key] = value;
			if (key == "clock") {
				if (value == "thread-cpu") {
					mClockSource = THREAD_CPU;
				}
				else if (value == "wall") {
					mClockSource = WALL;
				}
				else if (value == "dual") {
					mClockSource = DUAL;
				}
			}
		}
	}

	void DmTraceReader::parseThread(const String &line)
	{
		std::stringstream str(line);
		String idStr, name;
		if (readToken(str, idStr, '\t') && readToken(str, name, '\n')) {
			int id = atoi(idStr.c_str());
			mThreadMap.add(id, new ThreadData(id, name.c_str(), mTopLevel, &mCallList));
		}
	}

	void DmTraceReader::parseMethod(const String &line)
	{
		std::stringstream str(line);

		id_type id = 0;
		String idStr;
		String className;
		String methodName;
		String signature;
		String pathname;
		String lineStr;
		int lineNumber = -1;

		if (!readToken(str, idStr, '\t') || !readToken(str, className, '\t')) {
			return;
		}

		char* endPtr;
		id = (id_type)strtoul(idStr.c_str(), &endPtr, 0);
		if (*endPtr != '\0') {
			throw "ERROR: bad method ID";
		}

		if (readToken(str, methodName, '\t') && readToken(str, signature, '\t')) {
			if (readToken(str, pathname, '\t') && readToken(str, lineStr, '\t')) {
				lineNumber = atoi(lineStr.c_str());
				constructPathname(className, pathname);
			}
			else {
				if (signature.front() != '(') {
					pathname = methodName;
					lineNumber = atoi(signature.c_str());
				}
			}
		}
		mMethodMap.add(id, new MethodData(id, className.c_str(), methodName.c_str(), signature.c_str(), pathname.c_str(), lineNumber));
	}

	void DmTraceReader::constructPathname(String& className, String& pathname)
	{
		size_t index = className.find('/');
		if ((index != className.npos) && (index < className.size() - 1) && (pathname.compare(className.size() - 6, 5, ".java"))) {
			pathname = className.substr(0, index + 1) + pathname;
		}
	}

	void DmTraceReader::generateSegments()
	{
		mMinTime = UINT_MAX;
		mMaxTime = 0;

		if (mCallList.size() > 0) {
			mMinTime = mCallList.get(0)->getStartTime();
		}

		for (auto _i = 0; _i < mCallList.size(); _i++) {
			Call* block = mCallList.get(_i);
			ThreadData* row = block->getThreadData();
			if (!block->isIgnoredBlock(&mCallList)) {
				id_type rowId = row->getId();
				
				RowData* rd;
				HashMap<id_type, RowData*>::iterator it = mRowById.find(rowId);
				if (it == mRowById.end()) {
					rd = new RowData(row);
					mRowById.add(rowId, rd);
				}
				else {
					rd = it->second;
				}
				
				uint32_t blockStartTime = block->getStartTime();
				uint32_t blockEndTime = block->getEndTime();
				
				if (blockEndTime > rd->mEndTime) {
					auto start = std::max(blockStartTime, rd->mEndTime);
					rd->mElapsed = blockEndTime - start;
					rd->mEndTime = blockEndTime;
				}

				if (blockEndTime > mMaxTime) {
					mMaxTime = blockEndTime;
				}
				
				int top = rd->top();
				if (top == -1) {
					rd->push(block->getIndex());
				}
				else {
					Call* topCall = mCallList.get(top);
					uint32_t topStartTime = topCall->getStartTime();
					uint32_t topEndTime = topCall->getEndTime();
					
					if (topEndTime >= blockStartTime) {
						if (topStartTime < blockStartTime) {
							Segment* segment = mSegments.addNull2();
							segment->init(rd, &mCallList, topCall->getIndex(), topStartTime, blockStartTime);
						}
						if (topEndTime == blockStartTime)
							rd->pop();
						
						rd->push(block->getIndex());
					}
					else {
						popFrames(rd, &mCallList, topCall, blockStartTime, &mSegments);
						rd->push(block->getIndex());
					}
				}
			}
		}
		
		for (auto _i = mRowById.begin(); _i != mRowById.end(); _i++) {
			RowData* rd = _i->second;
			{
				int top = rd->top();
				popFrames(rd, &mCallList, mCallList.get(top), INT_MAX, &mSegments);
			}
		}
		
		mRows = mRowById.value_vector();
		std::sort(mRows->begin(), mRows->begin(), RowData::Less());
		
		int index = 0;
		for (auto ii = mRows->begin(); ii < mRows->end(); ii++) {
			(*ii)->mRank = index++;
		}
		
		mNumRows = 0;
		for (auto ii = mRows->begin(); ii < mRows->end(); ii++) {
			if ((*ii)->mElapsed == 0LL)
				break;
			
			mNumRows += 1;
		}
		std::cerr << "Begin to sort segment" << std::endl;
		mSegments.sort();
		std::cerr << "Sort segment done" << std::endl;
        if (mRegression) {
            dumpSegments();
        }

	}
	
	void DmTraceReader::popFrames(RowData* rd, CallList* callList, Call* top, uint32_t startTime, SegmentList* segmentList)
	{
		uint32_t topEndTime = top->getEndTime();
		uint32_t lastEndTime = top->getStartTime();
		
		while (topEndTime <= startTime) {
			if (topEndTime > lastEndTime) {
				auto segment = segmentList->addNull2();
				segment->init(rd, callList, top->getIndex(), lastEndTime, topEndTime);
				lastEndTime = topEndTime;
			}
			rd->pop();
			
			int topIndex = rd->top();
			if (topIndex == -1)
				return;
			top = callList->get(topIndex);
			
			topEndTime = top->getEndTime();
		}
		
		if (lastEndTime < startTime) {
			Segment* bd = segmentList->addNull2();
			bd->init(rd, callList, top->getIndex(), lastEndTime, startTime);
		}
	}

	void DmTraceReader::analyzeData()
	{
		TimeBase* timeBase = getPreferredTimeBase();

		mSortedThreads = mThreadMap.value_vector();
		std::sort(mSortedThreads->begin(), mSortedThreads->end(), ThreadData::Less(timeBase, &mCallList));

		mSortedMethods = mMethodMap.value_vector();
		std::sort(mSortedMethods->begin(), mSortedMethods->end(), MethodData::Less(timeBase));

		int nonZero = 0;
		for (auto it = mSortedMethods->begin(); it != mSortedMethods->end(); it++) {
			MethodData* md = *(it);
			if (timeBase->getElapsedInclusiveTime(md) == 0LL)
			{
				mSortedMethods->erase(it);
				break;
			}
			md->setRank(nonZero);
			nonZero++;
		}

		for (auto it = mSortedMethods->begin(); it != mSortedMethods->end(); it++) {
			MethodData* md = *(it);
			md->analyzeData(timeBase);
		}

		if (mRegression) {
			dumpMethodStats();
		}
	}
	
	void DmTraceReader::dumpSegments()
	{
		printf("\nSegments\n");
		printf("id\tt-start\tt-end\tg-start\tg-end\texcl.\tincl.\tmethod\n");
		for (auto _i = 0; _i < mSegments.size(); _i++) {
			Segment* segment = mSegments.get(_i);
			Call* call = mCallList.get(segment->mBlock);
			printf("%2d\t%s\t%4d\t%4d\t%s\n"
				   , call->getThreadId()
				   , segment->mRowData->mName.c_str()
				   , segment->mStartTime
				   , segment->mEndTime
				   , call->getMethodData()->getName()
				   );
		}
	}
	
	void DmTraceReader::dumpThreadTimes()
	{
		printf("\nThread Times\n");
		printf("id\tt-start\tt-end\tg-start\tg-end\tname\n");
		for (auto _i = mThreadMap.begin(); _i != mThreadMap.end(); _i++) {
			ThreadData* threadData = _i->second;
			printf("%2u\t%8d\t%8d\t%8d\t%8d\t%s\n"
				, threadData->getId()
				, threadData->mThreadStartTime
				, threadData->mThreadEndTime
				, threadData->mGlobalStartTime
				, threadData->mGlobalEndTime
				, threadData->getName());
		}
	}
	
	void DmTraceReader::dumpCallTimes()
	{
		printf("\nCall Times\n");
		printf("id\tt-start\tt-end\tg-start\tg-end\texcl.\tincl.\tmethod\n");
		for (auto _i = 0; _i < mCallList.size(); _i++) {
			Call* call = mCallList.get(_i);
			{
				printf("%2u\t%8d\t%8d\t%8d\t%8d\t%8d\t%8d\t%s\n"
					, call->getThreadId()
					, call->mThreadStartTime
					, call->mThreadEndTime
					, call->mGlobalStartTime
					, call->mGlobalEndTime
					, call->mExclusiveCpuTime
					, call->mInclusiveCpuTime
					, call->getMethodData()->getName());
			}
		}
	}
	
	void DmTraceReader::dumpMethodStats()
	{
		printf("\nMethod Stats\n");
		printf("Excl Cpu\tIncl Cpu\tExcl Real\tIncl Real\tCalls\tMethod\n");
		for (auto _i = mSortedMethods->begin(); _i != mSortedMethods->end(); _i++) {
			MethodData* md = *_i;
			String calls;
			printf("%9d\t%9d\t%9d\t%9d\t%9s\t%s\n"
				, md->getElapsedExclusiveCpuTime()
				, md->getElapsedInclusiveCpuTime()
				, md->getElapsedExclusiveRealTime()
				, md->getElapsedInclusiveRealTime()
				, md->getCalls(calls)
				, md->getProfileName());
		}
	}

	MethodPtrList* DmTraceReader::getMethods()
	{
		return mSortedMethods;
	}

	ThreadPtrList* DmTraceReader::getThreads()
	{
		return mSortedThreads;
	}

	uint32_t DmTraceReader::getTotalCpuTime()
	{
		return mTotalCpuTime;
	}

	uint32_t DmTraceReader::getTotalRealTime()
	{
		return mTotalRealTime;
	}

	bool DmTraceReader::haveCpuTime()
	{
		return mClockSource != WALL;
	}

	bool DmTraceReader::haveRealTime()
	{
		return mClockSource != THREAD_CPU;
	}

	PropertyMap& DmTraceReader::getProperties()
	{
		return mPropertiesMap;
	}

	TimeBase* DmTraceReader::getPreferredTimeBase()
	{
		if (mClockSource == WALL) {
			return TimeBase::REAL_TIME;
		}
		return TimeBase::CPU_TIME;
	}

	const char* DmTraceReader::getClockSource()
	{
		switch (mClockSource) {
		case THREAD_CPU:
			return "cpu time";
		case WALL:
			return "cpu time";
		case DUAL:
			return "cpu real time, dual clock";
		default:
			return nullptr;
		}
	}
};
