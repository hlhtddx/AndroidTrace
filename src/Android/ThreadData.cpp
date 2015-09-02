#include "ThreadData.hpp"
#include <iostream>

namespace Android {

#ifdef CLOCK_SOURCE_THREAD_CPU
    Call* ThreadData::enter(MethodData* method, TraceActionList* trace, MethodData* topLevel, CallList* callList)
#else
    Call* ThreadData::enter(MethodData* method, MethodData* topLevel)
#endif
    {
		if (mIsEmpty) {
#ifdef CLOCK_SOURCE_THREAD_CPU
            if (trace != nullptr) {
				trace->push_back(TraceAction(ACTION_ENTER, mRootCall));
			}
#endif
		}

        int callerIndex = top();
		int callIndex = mCallList.addNull();
		Call* call = mCallList.get(callIndex);
		call->init(this, method, callerIndex, callIndex);
        push(callIndex);
        mLastCall = callIndex;

#ifdef CLOCK_SOURCE_THREAD_CPU
        if (trace != nullptr) {
			trace->push_back(TraceAction(ACTION_ENTER, callIndex));
		}
#endif

		MethodIntMap::iterator it = mStackMethods.find(method);
		int num = 0;
		if (it == mStackMethods.end()) {
			num = 1;
		}
		else {
			num = it->second;
			if (num > 1) {
				call->setRecursive(true);
			}
		}
		mStackMethods[method] = num + 1;
		return call;
	}

#ifdef CLOCK_SOURCE_THREAD_CPU
    Call* ThreadData::exit(MethodData* method, TraceActionList* trace)
#else
    Call* ThreadData::exit(MethodData* method)
#endif
	{
        Call* call = topCall();

		if (call->getCaller() == -1) {
			return nullptr;
		}

        if (call->getMethodData() != method) {
			String error = "Method exit (";
			error += method->getName();
			error += ") does not match current method (";
			error += call->getMethodData()->getName();
			error += ")";
			throw error;
		}

        call->setEnd(mLastCall);
        mLastCall = call->getIndex();
		pop();

#ifdef CLOCK_SOURCE_THREAD_CPU
        if (trace != nullptr) {
			trace->push_back(TraceAction(ACTION_EXIT, call->getIndex()));
		}
#endif
		MethodIntMap::iterator it = mStackMethods.find(method);
		if (it != mStackMethods.end()) {
			mStackMethods[method] = it->second - 1;
		}

        return call;
	}

    template<> int CallStack::invalidValue = -1;

	Call* ThreadData::topCall()
	{
        int topCallIndex = top();
        if (topCallIndex == invalidValue) {
            return nullptr;
        }
		return mCallList.get(topCallIndex);
	}

#ifdef CLOCK_SOURCE_THREAD_CPU
    void ThreadData::endTrace(TraceActionList* trace)
#else
    void ThreadData::endTrace()
#endif
	{
		for (auto i = rbegin(); i != rend(); i++) {
			Call* call = mCallList.get(*i);
			call->mGlobalEndTime = mGlobalEndTime;
			call->mThreadEndTime = mThreadEndTime;
            call->setEnd((int)mCallList.size() - 1);
#ifdef CLOCK_SOURCE_THREAD_CPU
            if (trace) {
				trace->push_back(TraceAction(ACTION_INCOMPLETE, call->getIndex()));
			}
#endif
        }
        mCallList.freeExtra();
        if (mCallList.size() > 0) {
            for (auto i = mCallList.size() - 1; i >= 1; i--) {
                Call* call = mCallList.get(i);
                uint32_t realTime = call->mGlobalEndTime - call->mGlobalStartTime;
                call->mExclusiveRealTime = std::max<uint32_t>(realTime - call->mInclusiveRealTime, 0);
                call->mInclusiveRealTime = realTime;
                call->finish(&mCallList);
            }
        }

        clear();
		mStackMethods.clear();
	}

};
