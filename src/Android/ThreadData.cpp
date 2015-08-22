#include "ThreadData.hpp"

namespace Android {

	Call* ThreadData::enter(MethodData* method, TraceActionList* trace, Call::CallList* callList)
	{
		if (mIsEmpty) {
			mIsEmpty = false;
			if (trace != nullptr) {
				trace->push_back(TraceAction(ACTION_ENTER, mRootCall));
			}
		}
		int callerIndex = top();
		int callIndex = callList->addNull();
		Call* call = callList->get(callIndex);
		call->init(this, method, callerIndex, callIndex);
		mStack.push_back(callIndex);

		if (trace != nullptr) {
			trace->push_back(TraceAction(ACTION_ENTER, callIndex));
		}

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

	Call* ThreadData::exit(MethodData* method, TraceActionList* trace, Call::CallList* callList)
	{
		Call* call = top(callList);
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

		mStack.pop_back();

		if (trace != nullptr) {
			trace->push_back(TraceAction(ACTION_EXIT, call->getIndex()));
		}
		MethodIntMap::iterator it = mStackMethods.find(method);
		if (it != mStackMethods.end()) {
			mStackMethods[method] = it->second - 1;
		}

		return call;
	}

	Call* ThreadData::top(Call::CallList* callList)
	{
		if (mStack.size() == 0)
			return NULL;
		return callList->get(mStack.back());
	}

	int ThreadData::top()
	{
		if (mStack.size() == 0)
			return -1;
		return (int)mStack.back();
	}

	void ThreadData::endTrace(TraceActionList* trace, Call::CallList* callList)
	{
		for (ThreadStack::reverse_iterator i = mStack.rbegin(); i != mStack.rend(); i++) {
			Call* call = callList->get(*i);
			call->mGlobalEndTime = mGlobalEndTime;
			call->mThreadEndTime = mThreadEndTime;
			if (trace) {
				trace->push_back(TraceAction(ACTION_INCOMPLETE, call->getIndex()));
			}
		}
		mStack.clear();
		mStackMethods.clear();
	}

};
