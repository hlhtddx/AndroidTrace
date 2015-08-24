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
        int mNext;

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
        int getNext() const {
            return mNext;
        }
        void setNext(int next) {
            mNext = next;
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
        Call* getNextSiblings(Call* call) {
            int callIndex = call->getNext();
            if (callIndex == -1) {
                return nullptr;
            }
            return get(callIndex);
        }
        Call* getCaller(Call* call) {
            int callIndex = call->getCaller();
            if (callIndex == -1) {
                return nullptr;
            }
            return get(callIndex);
        }
        Call* getNextCall(Call* call) {
            int callIndex = call->getNext();
            while (callIndex == -1) {
                int caller = call->getParentBlockIndex();
                if (caller == -1) {
                    // Call List is over
                    return nullptr;
                }
                callIndex = get(caller)->getNext();
            }
        }
    };
#if 0
	class RowData : public Object
	{
	public:
		String      mName;
		int         mRank;
		uint32_t    mElapsed;
		Vector<int> mStack;
		uint32_t    mEndTime;
		
	public:
		void push(int index);
		int top();
		void pop();
		
	public:
		RowData(ThreadData* row);
		struct Less : public std::binary_function<RowData*, RowData*, bool> {
			bool operator() (const RowData* _Left, const RowData* _Right) const {
				return _Left->mElapsed < _Right->mElapsed;
			}
		};
	};
#endif
	typedef struct Segment
	{
	public:
        ThreadData* mThreadData;
        Call*       mBlock;
        uint32_t    mStartTime;
		uint32_t    mEndTime;
		bool mIsContextSwitch;
		
	public:
		void init(ThreadData* rowData, CallList* callList, int callIndex, uint32_t startTime, uint32_t endTime);
	} Segment;
	
	typedef FastArray<Segment> SegmentList;

};
