#pragma once

#include "Common.hpp"

namespace Android {
	class ThreadData;
	class MethodData;
    typedef struct Call Call;

	class CallList;

	typedef struct Call
	{
	private:
		ThreadData* mThreadData;
		MethodData* mMethodData;

	private:
		bool mIsRecursive;
		int mIndex;
        int mCaller;
        int mEnd;

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
		int getIndex() const {
			return mIndex;
		}
        int getCaller() const {
            return mCaller;
        }
        int getEnd() const {
            return mEnd;
        }
        void setEnd(int end) {
            mEnd = end;
        }

        double addWeight(int x, int y, double weight);
		void clearWeight();
		uint32_t getStartTime();
		uint32_t getEndTime();
		uint32_t getExclusiveCpuTime();
		uint32_t getInclusiveCpuTime();
		uint32_t getExclusiveRealTime();
		uint32_t getInclusiveRealTime();
		COLOR getColor();
		const char* getName();
		ThreadData* getThreadData();
		int getThreadId();
		MethodData* getMethodData();
		bool isContextSwitch();
		bool isIgnoredBlock(CallList* callList);
		int getParentBlockIndex();
		Call* getParentBlock(CallList* callList);
		bool isRecursive();

	public:
		void setRecursive(bool isRecursive);
		void addCpuTime(uint32_t elapsedCpuTime);
		void finish(CallList* callList);
		void init(ThreadData* threadData, MethodData* methodData, int caller, int index = -1);
	} Call;

    class CallList : public FastArray<Call>
    {
    public:
        //
        //// Get the sibling of this call. If no, return null
        //Call* getNextSiblings(Call* call) {
        //    int callIndex = call->getNext();
        //    if (callIndex == -1) {
        //        return nullptr;
        //    }
        //    return get(callIndex);
        //}

        //// Get the caller of this call. If it is root call, return null
        //Call* getCaller(Call* call) {
        //    int callIndex = call->getCaller();
        //    if (callIndex == -1) {
        //        return nullptr;
        //    }
        //    return get(callIndex);
        //}

        //// 1. Get the sibling of this call.
        //// 2. Try to return ancestors's siblings
        //// 3. If no, return null
        //Call* getNextCall(Call* call) {
        //    while (call != nullptr) {
        //        Call* sibling = getNextSiblings(call);
        //        if (sibling != nullptr) {
        //            return sibling;
        //        }
        //        call = getCaller(call);
        //    }
        //    return nullptr;
        //}

        //// Get the first child of this call. If no child, return null
        //Call* getFirstChild(Call* call) {
        //    int callIndex = call->getIndex() + 1;
        //    if (callIndex >= size()) {
        //        return nullptr;
        //    }
        //    if (callIndex == call->getNext()) {
        //        return nullptr;
        //    }
        //    return get(callIndex);
        //}
        //
    };
};
