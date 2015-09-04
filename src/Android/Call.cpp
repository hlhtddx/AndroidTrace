#include "Call.hpp"
#include "ThreadData.hpp"
#include "MethodData.hpp"

namespace Android {

	void Call::init(ThreadData* threadData, MethodData* methodData, int caller, int index)
	{
		mIsRecursive = false;
		mThreadData = threadData;
		mMethodData = methodData;
		mCaller = caller;
		mIndex = index;
        mEnd = -1;
		mGlobalStartTime = 0;
		mGlobalEndTime = 0;
		mThreadStartTime = 0;
		mThreadEndTime = 0;
		mInclusiveRealTime = 0;
		mExclusiveRealTime = 0;
		mInclusiveCpuTime = 0;
		mExclusiveCpuTime = 0;
	}

	double Call::addWeight(int x, int y, double weight)
	{
		return mMethodData->addWeight(x, y, weight);
	}

	void Call::clearWeight()
	{
		mMethodData->clearWeight();
	}

	uint32_t Call::getStartTime()
	{
		return mGlobalStartTime;
	}

	uint32_t Call::getEndTime()
	{
		return mGlobalEndTime;
	}

	uint32_t Call::getExclusiveCpuTime()
	{
		return mExclusiveCpuTime;
	}

	uint32_t Call::getInclusiveCpuTime()
	{
		return mInclusiveCpuTime;
	}

	uint32_t Call::getExclusiveRealTime()
	{
		return mExclusiveRealTime;
	}

	uint32_t Call::getInclusiveRealTime()
	{
		return mInclusiveRealTime;
	}

	COLOR Call::getColor()
	{
		return mMethodData->getColor();
	}

	const char* Call::getName()
	{
		if (mMethodData == nullptr) {
			return "unknown";
		}
		return mMethodData->getProfileName();
	}

	ThreadData* Call::getThreadData()
	{
		return mThreadData;
	}

	int Call::getThreadId()
	{
		return mThreadData->getId();
	}

	MethodData* Call::getMethodData()
	{
		return mMethodData;
	}

	bool Call::isContextSwitch()
	{
		return mMethodData->getId() == -1;
	}

	bool Call::isIgnoredBlock(CallList* callList)
	{
		return (mCaller == -1) || (isContextSwitch() && (callList->get(mCaller)->mCaller == -1));
	}

	Call* Call::getParentBlock(CallList* callList)
	{
		return callList->get(mCaller);
	}

	int Call::getParentBlockIndex()
	{
		return mCaller;
	}

	bool Call::isRecursive()
	{
		return mIsRecursive;
	}

	void Call::setRecursive(bool isRecursive)
	{
		mIsRecursive = isRecursive;
	}

	void Call::addCpuTime(uint32_t elapsedCpuTime)
	{
		mExclusiveCpuTime += elapsedCpuTime;
		mInclusiveCpuTime += elapsedCpuTime;
	}

	void Call::finish(CallList* callList)
	{
		if (mCaller != -1) {
			callList->get(mCaller)->mInclusiveCpuTime += mInclusiveCpuTime;
			callList->get(mCaller)->mInclusiveRealTime += mInclusiveRealTime;
		}

		mMethodData->addElapsedExclusive(mExclusiveCpuTime, mExclusiveRealTime);

		if (!mIsRecursive) {
			mMethodData->addTopExclusive(mExclusiveCpuTime, mExclusiveRealTime);
		}
		mMethodData->addElapsedInclusive(mInclusiveCpuTime, mInclusiveRealTime, mIsRecursive, mCaller, callList);
	}
};
