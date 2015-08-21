#include "DmTraceReader.hpp"
#include "ColorController.hpp"
#include <fstream>

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
		for (MethodMap::iterator it = mMethodMap.begin(); it != mMethodMap.end(); it++) {
			delete it->second;
		}
		for (ThreadMap::iterator it = mThreadMap.begin(); it != mThreadMap.end(); it++) {
			delete it->second;
		}
	}

	void DmTraceReader::generateTrees()
	{
		auto offset = parseKeys();
		parseData(offset);
		analyzeData();
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
		int version = buffer->getByte();
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

	ByteBuffer* DmTraceReader::mapFile(const char* filename, int64_t offset)
	{
		char* buffer = nullptr;
		try {
			std::ifstream in(filename, std::ios::binary | std::ios::in);
			in.seekg(0, std::ios_base::end);
			int64_t size = in.tellg() - offset;

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

	void DmTraceReader::parseData(int64_t offset) /* throws(IOException) */
	{
		ByteBuffer* buffer = mapFile(mTraceFileName.c_str(), offset);
		readDataFileHeader(buffer);
		List<TraceAction> lTrace;
		List<TraceAction> *trace = nullptr;
		if (mClockSource == THREAD_CPU) {
			trace = &lTrace;
		}
		bool haveThreadClock = mClockSource != WALL;
		bool haveGlobalClock = mClockSource != THREAD_CPU;
		ThreadData* prevThreadData = nullptr;

		while (!buffer->end()) {
			int threadId;
			int methodId;
			uint32_t threadTime = 0;
			uint32_t globalTime = 0;
			try {
				int recordSize = mRecordSize;
				if (mVersionNumber == 1) {
					threadId = buffer->getByte();
					if (threadId <= 0) {
						printf("The file should reach to the end\n");
					}
					recordSize--;
				}
				else {
					threadId = buffer->getShort();
					if (threadId <= 0) {
						printf("The file should reach to the end\n");
					}
					recordSize -= 2;
				}
				methodId = buffer->getInt();
				recordSize -= 4;

				ClockSource v = mClockSource;
				if (v == WALL) {
					threadTime = 0LL;
					globalTime = buffer->getInt();
					recordSize -= 4;
				}
				else if (v == DUAL) {
					threadTime = buffer->getInt();
					globalTime = buffer->getInt();
					recordSize -= 8;
				} if (((v == THREAD_CPU) || ((v != WALL) && (v != DUAL) && (v != THREAD_CPU)))) {
					threadTime = buffer->getInt();
					globalTime = 0LL;
					recordSize -= 4;
				}

				offset += recordSize;
			}
			catch (BoundaryException& e) {
				e.getDescription();
				break;
			}

			int methodAction = methodId & METHOD_ACTION_MASK;
			methodId &= METHOD_ID_MASK;

//			TRACE("record: tid=%d, mid=%d, action=%d, ttime=%lld, gtime=%lld\n", threadId, methodId, methodAction, threadTime, globalTime);
			MethodData* methodData = NULL;
			if (mMethodMap.find(methodId) == mMethodMap.end()) {
				std::stringstream ss;
				ss << "0x" << std::ios::hex << methodId;
				methodData = new MethodData(methodId, ss.str());
				mMethodMap.add(methodId, methodData);
			}
			else {
				methodData = mMethodMap[methodId];
			}

			ThreadData* threadData = NULL;
			if (mThreadMap.find(threadId) == mThreadMap.end()) {
				std::stringstream ss;
				ss << "[" << std::ios::dec << threadId << "]";
				threadData = new ThreadData(threadId, ss.str(), mTopLevel, &mCallList);
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
						auto top = threadData->top(&mCallList);
						if (top->getMethodData() == mContextSwitch) {
							threadData->exit(mContextSwitch, trace, &mCallList);
							auto beforeSwitch = elapsedThreadTime / 2;
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
				auto top = threadData->top(&mCallList);
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

		for (ThreadMap::iterator it = mThreadMap.begin(); it != mThreadMap.end(); it++) {
			ThreadData* threadData = it->second;
			threadData->endTrace(trace, &mCallList);
		}

		uint32_t globalTime;
		if (!haveGlobalClock) {
			globalTime = 0LL;
			prevThreadData = nullptr;
			for (List<TraceAction>::iterator it = trace->begin(); it != trace->end(); it++) {
				TraceAction& traceAction = *it;
				{
					auto call = mCallList.get(traceAction.mCall);
					auto threadData = call->getThreadData();
					if (traceAction.mAction == TraceAction::ACTION_ENTER) {
						auto threadTime = call->mThreadStartTime;
						globalTime += call->mThreadStartTime - threadData->mThreadCurrentTime;
						call->mGlobalStartTime = globalTime;
						if (!threadData->mHaveGlobalTime) {
							threadData->mHaveGlobalTime = true;
							threadData->mGlobalStartTime = globalTime;
						}
						threadData->mThreadCurrentTime = threadTime;
					}
					else if (traceAction.mAction == TraceAction::ACTION_EXIT) {
						auto threadTime = call->mThreadEndTime;
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

		for (size_t i = mCallList.size() - 1; i >= 1; i--) {
			Call* call = mCallList.get(i);
			uint32_t realTime = call->mGlobalEndTime - call->mGlobalStartTime;
			call->mExclusiveRealTime = std::max<uint32_t>(realTime - call->mInclusiveRealTime, 0);
			call->mInclusiveRealTime = realTime;
			call->finish(&mCallList);
		}

		mTotalCpuTime = 0LL;
		mTotalRealTime = 0LL;

		for (ThreadMap::iterator it = mThreadMap.begin(); it != mThreadMap.end(); it++) {
			ThreadData* threadData = it->second;
			Call* rootCall = threadData->getRootCall(&mCallList);
			threadData->updateRootCallTimeBounds(&mCallList);
			rootCall->finish(&mCallList);
			mTotalCpuTime += rootCall->mInclusiveCpuTime;
			mTotalRealTime += rootCall->mInclusiveRealTime;
		}

		if (mRegression) {
			//::java::lang::System::out()->format("totalCpuTime %dus\n", new ::java::lang::ObjectArray({ static_cast<::java::lang::Object*>(::java::lang::Long::valueOf(mTotalCpuTime)) }));
			//::java::lang::System::out()->format("totalRealTime %dus\n", new ::java::lang::ObjectArray({ static_cast<::java::lang::Object*>(::java::lang::Long::valueOf(mTotalRealTime)) }));
			//dumpThreadTimes();
			//dumpCallTimes();
		}
		delete buffer;
	}

	int64_t DmTraceReader::parseKeys() /* throws(IOException) */
	{
		int64_t offset = 0;
		try {
			std::ifstream in(mTraceFileName, std::ios::binary | std::ios::in);
			auto mode = 0;
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
			mThreadMap.add(id, new ThreadData(id, name, mTopLevel, &mCallList));
		}
	}

	void DmTraceReader::parseMethod(const String &line)
	{
		std::stringstream str(line);

		int64_t id = 0;
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
		id = strtoul(idStr.c_str(), &endPtr, 0);
		if (*endPtr != '\0') {
			throw "ERROR: bad method ID";
		}

		if (readToken(str, methodName, '\t') && readToken(str, signature, '\t')) {
			if (readToken(str, pathname, '\t') && readToken(str, lineStr, '\t')) {
				lineNumber = atoi(lineStr.c_str());
				pathname = constructPathname(className, pathname);
			}
			else {
				if (signature.front() != '(') {
					pathname = methodName;
					lineNumber = atoi(signature.c_str());
				}
			}
		}
		mMethodMap.add(id, new MethodData(id, className, methodName, signature, pathname, lineNumber));
	}

	String DmTraceReader::constructPathname(const String& className, const String& pathname)
	{
		size_t index = className.find('/');
		if ((index != className.npos) && (index < className.size() - 1) && (pathname.compare(className.size() - 6, 5, ".java"))) {
			return className.substr(0, index + 1) + pathname;
		}
		return pathname;
	}

	void DmTraceReader::analyzeData()
	{
		TimeBase* timeBase = getPreferredTimeBase();

		mSortedThreads = mThreadMap.value_vector();
		std::sort(mSortedThreads->begin(), mSortedThreads->end(), ThreadData::Less(timeBase, &mCallList));

		mSortedMethods = mMethodMap.value_vector();
		std::sort(mSortedMethods->begin(), mSortedMethods->end(), MethodData::Less(timeBase));

		int nonZero = 0;
		for (MethodList::iterator it = mSortedMethods->begin(); it != mSortedMethods->end(); it++) {
			MethodData* md = *(it);
			if (timeBase->getElapsedInclusiveTime(md) == 0LL)
			{
				mSortedMethods->erase(it);
				break;
			}
			md->setRank(nonZero);
			nonZero++;
		}

		for (MethodList::iterator it = mSortedMethods->begin(); it != mSortedMethods->end(); it++) {
			MethodData* md = *(it);
			md->analyzeData(timeBase);
		}

		if (mRegression) {
			//dumpMethodStats();
		}
	}

	//
	//void DmTraceReader::dumpThreadTimes()
	//{
	//	::java::lang::System::out()->print("\nThread Times\n");
	//	::java::lang::System::out()->print("id  t-start    t-end  g-start    g-end     name\n");
	//	for (auto _i = (mThreadMap->values())->iterator(); _i->hasNext();) {
	//		ThreadData* threadData = java_cast<ThreadData*>(_i->next());
	//		{
	//			::java::lang::System::out()->format("%2d %8d %8d %8d %8d  %s\n", new ::java::lang::ObjectArray({
	//				static_cast<::java::lang::Object*>(::java::lang::Integer::valueOf(threadData->getId()))
	//				, static_cast<::java::lang::Object*>(::java::lang::Long::valueOf(threadData->mThreadStartTime))
	//				, static_cast<::java::lang::Object*>(::java::lang::Long::valueOf(threadData->mThreadEndTime))
	//				, static_cast<::java::lang::Object*>(::java::lang::Long::valueOf(threadData->mGlobalStartTime))
	//				, static_cast<::java::lang::Object*>(::java::lang::Long::valueOf(threadData->mGlobalEndTime))
	//				, static_cast<::java::lang::Object*>(threadData->getName())
	//			}));
	//		}
	//	}
	//}
	//
	//void DmTraceReader::dumpCallTimes()
	//{
	//	::java::lang::System::out()->print("\nCall Times\n");
	//	::java::lang::System::out()->print("id  t-start    t-end  g-start    g-end    excl.    incl.  method\n");
	//	for (auto _i = (mCallList)->iterator(); _i->hasNext();) {
	//		Call* call = java_cast<Call*>(_i->next());
	//		{
	//			::java::lang::System::out()->format("%2d %8d %8d %8d %8d %8d %8d  %s\n", new ::java::lang::ObjectArray({
	//				static_cast<::java::lang::Object*>(::java::lang::Integer::valueOf(call->getThreadId()))
	//				, static_cast<::java::lang::Object*>(::java::lang::Long::valueOf(call->mThreadStartTime))
	//				, static_cast<::java::lang::Object*>(::java::lang::Long::valueOf(call->mThreadEndTime))
	//				, static_cast<::java::lang::Object*>(::java::lang::Long::valueOf(call->mGlobalStartTime))
	//				, static_cast<::java::lang::Object*>(::java::lang::Long::valueOf(call->mGlobalEndTime))
	//				, static_cast<::java::lang::Object*>(::java::lang::Long::valueOf(call->mExclusiveCpuTime))
	//				, static_cast<::java::lang::Object*>(::java::lang::Long::valueOf(call->mInclusiveCpuTime))
	//				, static_cast<::java::lang::Object*>(npc(call->getMethodData())->getName())
	//			}));
	//		}
	//	}
	//}
	//
	//void DmTraceReader::dumpMethodStats()
	//{
	//	::java::lang::System::out()->print("\nMethod Stats\n");
	//	::java::lang::System::out()->print("Excl Cpu  Incl Cpu  Excl Real Incl Real    Calls  Method\n");
	//	for (auto md : *mSortedMethods) {
	//		::java::lang::System::out()->format("%9d %9d %9d %9d %9s  %s\n", new ::java::lang::ObjectArray({
	//			static_cast<::java::lang::Object*>(::java::lang::Long::valueOf(md->getElapsedExclusiveCpuTime()))
	//			, static_cast<::java::lang::Object*>(::java::lang::Long::valueOf(md->getElapsedInclusiveCpuTime()))
	//			, static_cast<::java::lang::Object*>(::java::lang::Long::valueOf(md->getElapsedExclusiveRealTime()))
	//			, static_cast<::java::lang::Object*>(::java::lang::Long::valueOf(md->getElapsedInclusiveRealTime()))
	//			, static_cast<::java::lang::Object*>(md->getCalls())
	//			, static_cast<::java::lang::Object*>(md->getProfileName())
	//		}));
	//	}
	//}
	//
	//void DmTraceReader::dumpTimeRecs(::java::util::ArrayList* timeRecs)
	//{
	//	::java::lang::System::out()->print("\nTime Records\n");
	//	::java::lang::System::out()->print("id  t-start    t-end  g-start    g-end  method\n");
	//	for (auto _i = timeRecs->iterator(); _i->hasNext();) {
	//		TimeLineView_Record* record = java_cast<TimeLineView_Record*>(_i->next());
	//		{
	//			auto call = java_cast<Call*>(record->block);
	//			::java::lang::System::out()->format("%2d %8d %8d %8d %8d  %s\n", new ::java::lang::ObjectArray({
	//				static_cast<::java::lang::Object*>(::java::lang::Integer::valueOf(call->getThreadId()))
	//				, static_cast<::java::lang::Object*>(::java::lang::Long::valueOf(call->mThreadStartTime))
	//				, static_cast<::java::lang::Object*>(::java::lang::Long::valueOf(call->mThreadEndTime))
	//				, static_cast<::java::lang::Object*>(::java::lang::Long::valueOf(call->mGlobalStartTime))
	//				, static_cast<::java::lang::Object*>(::java::lang::Long::valueOf(call->mGlobalEndTime))
	//				, static_cast<::java::lang::Object*>(npc(call->getMethodData())->getName())
	//			}));
	//		}
	//	}
	//}
	//
	//java::util::HashMap* DmTraceReader::getThreadLabels()
	//{
	//	auto labels = new ::java::util::HashMap();
	//	for (auto _i = (mThreadMap->values())->iterator(); _i->hasNext();) {
	//		ThreadData* t = java_cast<ThreadData*>(_i->next());
	//		{
	//			labels->put(static_cast<::java::lang::Object*>(::java::lang::Integer::valueOf(t->getId())), static_cast<::java::lang::Object*>(t->getName()));
	//		}
	//	}
	//	return labels;
	//}

	DmTraceReader::MethodList* DmTraceReader::getMethods()
	{
		return mSortedMethods;
	}

	DmTraceReader::ThreadList* DmTraceReader::getThreads()
	{
		return mSortedThreads;
	}

	int64_t DmTraceReader::getTotalCpuTime()
	{
		return mTotalCpuTime;
	}

	int64_t DmTraceReader::getTotalRealTime()
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

	DmTraceReader::PropertyMap& DmTraceReader::getProperties()
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
