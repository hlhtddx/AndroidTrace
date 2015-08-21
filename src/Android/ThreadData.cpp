#include "ThreadData.hpp"

namespace Android {

	ThreadData::ThreadData()
	{
		mId = -1;
		mName = "Not inited";
		mIsEmpty = true;
		mRootCall = -1;
	}

	ThreadData::ThreadData(int id, String name, MethodData* topLevel, Call::CallList* callList)
	{
		mId = id;
		std::stringstream ss;
		ss << "[" << id << "] " << name;
		mName = ss.str();
		mIsEmpty = true;
		mRootCall = callList->addNull();
		callList->get(mRootCall)->init(this, topLevel, -1, mRootCall);
		mStack.push_back(mRootCall);
	}
	
	ThreadData::~ThreadData()
	{
	}

	const String& ThreadData::getName() const
	{
		return mName;
	}

	Call* ThreadData::getRootCall(Call::CallList* callList)
	{
		return callList->get(mRootCall);
	}

	bool ThreadData::isEmpty()
	{
		return mIsEmpty;
	}

	Call* ThreadData::enter(MethodData* method, TraceActionList* trace, Call::CallList* callList)
	{
		if (mIsEmpty) {
			mIsEmpty = false;
			if (trace != nullptr) {
				trace->push_back(TraceAction(TraceAction::ACTION_ENTER, mRootCall));
			}
		}
		int callerIndex = top();
		int callIndex = callList->addNull();
		Call* call = callList->get(callIndex);
		call->init(this, method, callerIndex, callIndex);
		mStack.push_back(callIndex);

		//TRACE("Dump stack(%p) for push\n");
		//for (ThreadStack::iterator it = mStack.begin(); it != mStack.end(); it++) {
		//	Call* cc = callList->get(*it);
		//	TRACE("\tcall = %p(method=%s.%s)\n", cc, cc->getMethodData()->getClassName().c_str(), cc->getMethodData()->getMethodName().c_str());
		//}
		//TRACE("End Dump\n");


		if (trace != nullptr) {
			trace->push_back(TraceAction(TraceAction::ACTION_ENTER, callIndex));
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
			String error = "Method exit (" + method->getName()
				+ ") does not match current method ("
				+ call->getMethodData()->getName()
				+ ")";
			throw error;
		}

		mStack.pop_back();

		//TRACE("Dump stack(%p) for push\n");
		//for (ThreadStack::iterator it = mStack.begin(); it != mStack.end(); it++) {
		//	Call* cc = callList->get(*it);
		//	TRACE("\tcall = %p(method=%s.%s)\n", cc, cc->getMethodData()->getClassName().c_str(), cc->getMethodData()->getMethodName().c_str());
		//}
		//TRACE("End Dump\n");


		if (trace != nullptr) {
			trace->push_back(TraceAction(TraceAction::ACTION_EXIT, call->getIndex()));
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
				trace->push_back(TraceAction(TraceAction::ACTION_INCOMPLETE, call->getIndex()));
			}
		}
		mStack.clear();
		mStackMethods.clear();
	}

	void ThreadData::updateRootCallTimeBounds(Call::CallList* callList)
	{
		if (!mIsEmpty) {
			callList->get(mRootCall)->mGlobalStartTime = mGlobalStartTime;
			callList->get(mRootCall)->mGlobalEndTime = mGlobalEndTime;
			callList->get(mRootCall)->mThreadStartTime = mThreadStartTime;
			callList->get(mRootCall)->mThreadEndTime = mThreadEndTime;
		}
	}

	int ThreadData::getId()
	{
		return mId;
	}

	int64_t ThreadData::getCpuTime(Call::CallList* callList) const
	{
		return callList->get(mRootCall)->mInclusiveCpuTime;
	}

	int64_t ThreadData::getRealTime(Call::CallList* callList) const
	{
		return callList->get(mRootCall)->mInclusiveRealTime;
	}
};
