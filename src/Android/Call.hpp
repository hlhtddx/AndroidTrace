#pragma once

#include "Common.hpp"

namespace Android {
	class ThreadData;
	class MethodData;
	class Call;

	typedef FastArray<Call> CallList;

	typedef struct Call
	{
	private:
		ThreadData* mThreadData;
		MethodData* mMethodData;

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
	
	class RowData : public Object
	{
	public:
		String mName;
		uint32_t mRank;
		uint32_t mElapsed;
		Vector<int> mStack;
		uint32_t mEndTime;
		
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

	typedef struct Segment
	{
	public:
		RowData* mRowData;
		uint32_t mStartTime;
		uint32_t mEndTime;
		int mBlock;
		bool mIsContextSwitch;
		
	public:
		void init(RowData* rowData, CallList* callList, int callIndex, uint32_t startTime, uint32_t endTime);
	} Segment;
	
	typedef FastArray<Segment> SegmentList;

};
