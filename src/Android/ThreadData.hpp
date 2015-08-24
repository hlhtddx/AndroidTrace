#pragma once

#include "Common.hpp"
#include "MethodData.hpp"
#include "Call.hpp"
#include "TraceAction.hpp"

namespace Android {

	typedef List<size_t> ThreadStack;
	typedef HashMap<MethodData*, int> MethodIntMap;
	typedef List<TraceAction> TraceActionList;

	class ThreadData : public Object
	{
	public:

	private:
		id_type mId;
		String mName;
		bool mIsEmpty;
		int mRootCall;
        int mLastCall;
		ThreadStack mStack;
		MethodIntMap mStackMethods;

	public:
		bool mHaveGlobalTime;
		bool mHaveThreadTime;

		uint32_t mGlobalStartTime;
		uint32_t mGlobalEndTime;

		uint32_t mThreadStartTime;
		uint32_t mThreadEndTime;
		uint32_t mThreadCurrentTime;

	public:
		const char* getName() const
		{
			return mName.c_str();
		}
		Call* getRootCall(CallList* callList)
		{
			return callList->get(mRootCall);
		}

		bool isEmpty()
		{
			return mIsEmpty;
		}

	public:
		Call* enter(MethodData* method, TraceActionList* trace, CallList* callList);
		Call* exit(MethodData* method, TraceActionList* trace, CallList* callList);
		Call* top(CallList* callList);
		int top();
		void endTrace(TraceActionList* trace, CallList* callList);
		void updateRootCallTimeBounds(CallList* callList)
		{
			if (!mIsEmpty) {
				callList->get(mRootCall)->mGlobalStartTime = mGlobalStartTime;
				callList->get(mRootCall)->mGlobalEndTime = mGlobalEndTime;
				callList->get(mRootCall)->mThreadStartTime = mThreadStartTime;
				callList->get(mRootCall)->mThreadEndTime = mThreadEndTime;
			}
		}

    protected:
        Call* getLastCall(CallList* callList);

	public:
		id_type getId()
		{
			return mId;
		}
		
		uint32_t getCpuTime(CallList* callList) const
		{
			return callList->get(mRootCall)->mInclusiveCpuTime;
		}
		
		uint32_t getRealTime(CallList* callList) const
		{
			return callList->get(mRootCall)->mInclusiveRealTime;
		}

	public:
		ThreadData()
		{
			mId = 0;
			mName = "Not inited";
			mIsEmpty = true;
			mRootCall = -1;
            mLastCall = -1;
        }

		ThreadData(id_type id, const char* name, MethodData* topLevel, CallList* callList)
		{
			mId = id;
			std::stringstream ss;
			ss << "[" << id << "] " << name;
			mName = ss.str();
			mIsEmpty = true;
			mRootCall = callList->addNull();
			callList->get(mRootCall)->init(this, topLevel, -1, mRootCall);
			mStack.push_back(mRootCall);
            mLastCall = -1;
        }

		~ThreadData()
		{
		}

		struct Less : public std::binary_function<ThreadData*, ThreadData*, bool> {
			TimeBase* timeBase;
			CallList* callList;

			Less(TimeBase* timeBase, CallList* callList) {
				this->timeBase = timeBase;
				this->callList = callList;
			}

			bool operator() (const ThreadData* _Left, const ThreadData* _Right) const {
				return timeBase->getTime(_Left, callList) < timeBase->getTime(_Right, callList);
			}
		};
	};
};
