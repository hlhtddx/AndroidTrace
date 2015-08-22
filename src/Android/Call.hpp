#pragma once

#include "Common.hpp"

namespace Android {
	class ThreadData;
	class MethodData;

	typedef struct Call
	{
	private:
		ThreadData* mThreadData;
		MethodData* mMethodData;

	public:
		typedef FastArray<Call> CallList;

	private:
		bool mIsRecursive;
		int mCaller;
		int mIndex;

	public:
		uint32_t mGlobalStartTime;
		uint32_t mGlobalEndTime;
		uint32_t mThreadStartTime;
		uint32_t mThreadEndTime;
		uint32_t mInclusiveRealTime;
		uint32_t mExclusiveRealTime;
		uint32_t mInclusiveCpuTime;
		uint32_t mExclusiveCpuTime;

	public:
		int getCaller() const {
			return mCaller;
		}
		int getIndex() const {
			return mIndex;
		}
		void updateName();
		double addWeight(int x, int y, double weight);
		void clearWeight();
		uint32_t getStartTime();
		uint32_t getEndTime();
		uint32_t getExclusiveCpuTime();
		uint32_t getInclusiveCpuTime();
		uint32_t getExclusiveRealTime();
		uint32_t getInclusiveRealTime();
		uint32_t getColor();
		const char* getName();
		ThreadData* getThreadData();
		int getThreadId();
		MethodData* getMethodData();
		bool isContextSwitch();
		bool isIgnoredBlock(Call::CallList* callList);
		int getParentBlockIndex();
		Call* getParentBlock(Call::CallList* callList);
		bool isRecursive();

	public:
		void setRecursive(bool isRecursive);
		void addCpuTime(uint32_t elapsedCpuTime);
		void finish(CallList* callList);
		void init(ThreadData* threadData, MethodData* methodData, int caller, int index = -1);
	} Call;

};
