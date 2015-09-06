#include "DmTraceData.hpp"
#include "ColorController.hpp"
#include <fstream>
#include <iostream>

#ifdef _MSC_VER
#include <windows.h>
#undef min
#undef max
#else
#include <mach/mach_time.h>
#endif

namespace Android {

	DmTraceData::DmTraceData()
	{
		mRegression = false;
		mTopLevel = new MethodData(0, "(toplevel)");
		mMethodMap.add(0, mTopLevel);
		mContextSwitch = new MethodData(-1, "(context switch)");
		mMethodMap.add(-1, mContextSwitch);
		mSortedThreads = nullptr;
		mSortedMethods = nullptr;
		mClockSource = UNKNOWN;
	}

	DmTraceData::~DmTraceData()
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

    void DmTraceData::open(const char* traceFileName, bool regression)
    {
        mRegression = regression;
        mTraceFileName = traceFileName;
        generateTrees();
    }

	void DmTraceData::generateTrees()
	{
#ifdef _MSC_VER
        ULONGLONG s1 = GetTickCount64();
#else
        uint64_t s1 = mach_absolute_time();
#endif
		std::cerr << "Parse keys ...";
		filepos offset = parseKeys();
#ifdef _MSC_VER
        ULONGLONG s2 = GetTickCount64();
#else
        uint64_t s2 = mach_absolute_time();
#endif
        std::cerr << s2 - s1 << "ms" << std::endl;

        std::cerr << "Parse data ...";
		parseData(offset);
#ifdef _MSC_VER
        ULONGLONG s3 = GetTickCount64();
#else
        uint64_t s3 = mach_absolute_time();
#endif
        std::cerr << s3 - s2 << "ms" << std::endl;

        std::cerr << "analyzeData ...";
		analyzeData();
#ifdef _MSC_VER
        ULONGLONG s4 = GetTickCount64();
#else
        uint64_t s4 = mach_absolute_time();
#endif
        std::cerr << s4 - s3 << "ms" << std::endl;

        std::cerr << "done" << std::endl;
		ColorController::assignMethodColors(mSortedMethods);

        std::cerr << "Dumping data ...";
        if (mRegression) {
            printf("totalCpuTime %dus\n", mTotalCpuTime);
            printf("totalRealTime %dus\n", mTotalRealTime);
            dumpThreadTimes();
            dumpCallTimes();
        }
#ifdef _MSC_VER
        ULONGLONG s5 = GetTickCount64();
#else
        uint64_t s5 = mach_absolute_time();
#endif
        std::cerr << s5 - s4 << "ms" << std::endl;

        std::cerr << "done" << std::endl;
    }
	//
	//ProfileProvider* DmTraceData::getProfileProvider()
	//{
	//    if(mProfileProvider == nullptr)
	//        mProfileProvider = new ProfileProvider(this);
	//
	//    return mProfileProvider;
	//}
	//

	void DmTraceData::readDataFileHeader(ByteBuffer* buffer)
	{
		int magic = buffer->getInt();
		if (magic != TRACE_MAGIC) {
			throw GeneralException("Invalid magic");
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

	ByteBuffer* DmTraceData::mapFile(const char* filename, filepos offset)
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

	filepos DmTraceData::parseKeys() /* throws(IOException) */
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

	void DmTraceData::parseOption(const String &line)
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
        else {
            throw GeneralException("Invalid Option, maybe invalid file format");
        }
	}

	void DmTraceData::parseThread(const String &line)
	{
		std::stringstream str(line);
		String idStr, name;
		if (readToken(str, idStr, '\t') && readToken(str, name, '\n')) {
			int id = atoi(idStr.c_str());
			mThreadMap.add(id, new ThreadData(id, name.c_str()));
		}
	}

	void DmTraceData::parseMethod(const String &line)
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

    void DmTraceData::parseData(filepos offset) /* throws(IOException) */
    {
        ByteBuffer* buffer = mapFile(mTraceFileName.c_str(), offset);
        if (buffer == nullptr) {
            throw GeneralException("Failed to open file");
        }

        readDataFileHeader(buffer);

#ifdef CLOCK_SOURCE_THREAD_CPU
        TraceActionList lTrace;
        TraceActionList *trace = nullptr;
        if (mClockSource == THREAD_CPU) {
            trace = &lTrace;
        }
#endif

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

            //printf("record: tid=%d, mid=%p, action=%d, ttime=%lld, gtime=%lld\n", threadId, (int64_t)methodId, methodAction, threadTime, globalTime);
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
                threadData = new ThreadData(threadId, ss.str().c_str());
                mThreadMap.add(threadId, threadData);
            }
            else {
                threadData = mThreadMap[threadId];
            }

            if (threadData->isEmpty()) {
                threadData->addRoot(mTopLevel);
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
#ifdef CLOCK_SOURCE_THREAD_CPU
                        Call* switchCall = prevThreadData->enter(mContextSwitch, trace, mTopLevel);
#else
                        Call* switchCall = prevThreadData->enter(mContextSwitch, mTopLevel);
#endif
                        switchCall->mThreadStartTime = prevThreadData->mThreadEndTime;
                        Call* top = threadData->topCall();
                        if (top->getMethodData() == mContextSwitch) {
#ifdef CLOCK_SOURCE_THREAD_CPU
                            threadData->exit(mContextSwitch, trace);
#else
                            threadData->exit(mContextSwitch);
#endif
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
#ifdef CLOCK_SOURCE_THREAD_CPU
                        Call* switchCall = threadData->enter(mContextSwitch, trace, mTopLevel);
#else
                        Call* switchCall = threadData->enter(mContextSwitch, mTopLevel);
#endif
                        uint32_t beforeSwitch = elapsedThreadTime / 2;
                        uint32_t afterSwitch = elapsedThreadTime - beforeSwitch;
                        switchCall->mGlobalStartTime = (globalTime - elapsedGlobalTime + beforeSwitch);
                        switchCall->mGlobalEndTime = (globalTime - afterSwitch);
                        switchCall->mThreadStartTime = (threadTime - afterSwitch);
                        switchCall->mThreadEndTime = switchCall->mThreadStartTime;
#ifdef CLOCK_SOURCE_THREAD_CPU
                        threadData->exit(mContextSwitch, trace);
#else
                        threadData->exit(mContextSwitch);
#endif
                    }
                }
                Call* top = threadData->topCall();
                if (top) {
                    top->addCpuTime(elapsedThreadTime);
                }
            }

            Call* call;
            switch (methodAction) {
            case METHOD_TRACE_ENTER:
#ifdef CLOCK_SOURCE_THREAD_CPU
                call = threadData->enter(methodData, trace, mTopLevel);
#else
                call = threadData->enter(methodData, mTopLevel);
#endif
                if (haveGlobalClock) {
                    call->mGlobalStartTime = globalTime;
                }
                if (haveThreadClock) {
                    call->mThreadStartTime = threadTime;
                }
                break;
            case METHOD_TRACE_EXIT:
            case METHOD_TRACE_UNROLL:
#ifdef CLOCK_SOURCE_THREAD_CPU
                call = threadData->exit(methodData, trace);
#else
                call = threadData->exit(methodData);
#endif
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
#ifdef CLOCK_SOURCE_THREAD_CPU
            threadData->endTrace(trace);
#else
            threadData->endTrace();
#endif
        }

#ifdef CLOCK_SOURCE_THREAD_CPU
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
#endif

        mTotalCpuTime = 0;
        mTotalRealTime = 0;

        for (auto it = mThreadMap.begin(); it != mThreadMap.end(); it++) {
            ThreadData* threadData = it->second;
            if (threadData->isEmpty()) {
                continue;
            }
            Call* rootCall = threadData->getRootCall();
            threadData->updateRootCallTimeBounds();
            rootCall->finish(threadData->getCallList());
            mTotalCpuTime += rootCall->mInclusiveCpuTime;
            mTotalRealTime += rootCall->mInclusiveRealTime;
        }
        delete buffer;
    }

	void DmTraceData::constructPathname(String& className, String& pathname)
	{
		size_t index = className.find('/');
		if ((index != className.npos) && (index < className.size() - 1) && (pathname.compare(className.size() - 6, 5, ".java"))) {
			pathname = className.substr(0, index + 1) + pathname;
		}
	}

	void DmTraceData::analyzeData()
	{
		TimeBase* timeBase = getPreferredTimeBase();

		mSortedMethods = mMethodMap.value_vector();
		std::sort(mSortedMethods->begin(), mSortedMethods->end(), MethodData::Greater(timeBase));

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

        mMinTime = UINT_MAX;
        mMaxTime = 0;

        mSortedThreads = mThreadMap.value_vector();
        std::sort(mSortedThreads->begin(), mSortedThreads->end(), ThreadData::GreaterElapse());

        int index = 0;
        for (auto ii = mSortedThreads->begin(); ii < mSortedThreads->end(); ii++) {
            ThreadData* thread = (*ii);
            if (!thread->isEmpty()) {
                mMinTime = std::min(thread->mGlobalStartTime, mMinTime);
                mMaxTime = std::max(thread->mGlobalEndTime, mMaxTime);
            }
            thread->mRank = index++;
        }

        mNumRows = 0;
        for (auto ii = mSortedThreads->begin(); ii < mSortedThreads->end(); ii++) {
            if ((*ii)->getElapseTime() == 0LL)
                break;

            mNumRows += 1;
        }

		if (mRegression) {
			dumpMethodStats();
		}
	}
	
	void DmTraceData::dumpSegments()
	{
		//printf("\nSegments\n");
		//printf("id\tt-start\tt-end\tg-start\tg-end\texcl.\tincl.\tmethod\n");
		//for (auto _i = 0; _i < mSegments.size(); _i++) {
		//	Segment* segment = mSegments.get(_i);
		//	printf("%2d\t%s\t%4d\t%4d\t%s\n"
		//		   , call->getThreadId()
		//		   , segment->mThreadData->mName.c_str()
		//		   , segment->mStartTime
		//		   , segment->mEndTime
		//		   , call->getMethodData()->getName()
		//		   );
		//}
	}
	
	void DmTraceData::dumpThreadTimes()
	{
		printf("\nThread Times\n");
		printf("id\tt-start\tt-end\tg-start\tg-end\tname\n");
		for (auto _i = mSortedThreads->begin(); _i != mSortedThreads->end(); _i++) {
			ThreadData* threadData = *_i;
			printf("%2u\t%8d\t%8d\t%8d\t%8d\t%s\n"
				, threadData->getId()
				, threadData->mThreadStartTime
				, threadData->mThreadEndTime
				, threadData->mGlobalStartTime
				, threadData->mGlobalEndTime
				, threadData->getName());
		}
	}
	
	void DmTraceData::dumpCallTimes()
	{
		printf("\nCall Times\n");
		printf("   i\t  id\tcaller\tend\tt-start\tt-end\tg-start\tg-end\texcl.\tincl.\tmethod\n");
        for (auto _i = mSortedThreads->begin(); _i != mSortedThreads->end(); _i++) {
            ThreadData* threadData = *_i;
            CallList* callList = threadData->getCallList();
            for (auto _i = 0; _i < callList->size(); _i++) {
                Call* call = callList->get(_i);
                {
                    printf("%4u\t%4u\t%4d\t%4d\t%8d\t%8d\t%8d\t%8d\t%8d\t%8d\t%s\n"
                        , call->getIndex()
                        , call->getThreadId()
                        , call->getCaller()
                        , call->getEnd()
                        , call->mThreadStartTime
                        , call->mThreadEndTime
                        , call->mGlobalStartTime
                        , call->mGlobalEndTime
                        , call->mExclusiveCpuTime
                        , call->mInclusiveCpuTime
                        , call->getMethodData()->getName()
                        );
                }
            }
        }
	}
	
	void DmTraceData::dumpMethodStats()
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

	MethodPtrList* DmTraceData::getMethods()
	{
		return mSortedMethods;
	}

	ThreadPtrList* DmTraceData::getThreads()
	{
		return mSortedThreads;
	}
 
    ThreadData* DmTraceData::getThreadData(id_type threadId)
    {
        auto i = mThreadMap.find(threadId);
        if (i == mThreadMap.end()) {
            return nullptr;
        }
        return i->second;
    }

    CallList* DmTraceData::getCallList(id_type threadId)
    {
        ThreadData* threadData = getThreadData(threadId);
        if (threadData == nullptr) {
            return nullptr;
        }
        return threadData->getCallList();
    }

	uint32_t DmTraceData::getTotalCpuTime()
	{
		return mTotalCpuTime;
	}

	uint32_t DmTraceData::getTotalRealTime()
	{
		return mTotalRealTime;
	}

	bool DmTraceData::haveCpuTime()
	{
		return mClockSource != WALL;
	}

	bool DmTraceData::haveRealTime()
	{
		return mClockSource != THREAD_CPU;
	}

	PropertyMap& DmTraceData::getProperties()
	{
		return mPropertiesMap;
	}

	TimeBase* DmTraceData::getPreferredTimeBase()
	{
		if (mClockSource == WALL) {
			return TimeBase::REAL_TIME;
		}
		return TimeBase::CPU_TIME;
	}

	const char* DmTraceData::getClockSource()
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
