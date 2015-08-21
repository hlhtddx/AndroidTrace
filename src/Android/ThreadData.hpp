#pragma once

#include "Common.hpp"
#include "MethodData.hpp"
#include "Call.hpp"
#include "TraceAction.hpp"

namespace Android {

	class ThreadData : public Object
	{
	public:
		typedef List<size_t> ThreadStack;
		typedef HashMap<MethodData*, int> MethodIntMap;
		typedef List<TraceAction> TraceActionList;

	private:
		int mId;
		String mName;
		bool mIsEmpty;
		int mRootCall;
		ThreadStack mStack;
		MethodIntMap mStackMethods;

	public:
		bool mHaveGlobalTime;
		uint32_t mGlobalStartTime;
		uint32_t mGlobalEndTime;
		bool mHaveThreadTime;
		uint32_t mThreadStartTime;
		uint32_t mThreadEndTime;
		uint32_t mThreadCurrentTime;

	public:
		const String& getName() const;
		Call* getRootCall(Call::CallList* callList);
		bool isEmpty();

	public:
		Call* enter(MethodData* method, TraceActionList* trace, Call::CallList* callList);
		Call* exit(MethodData* method, TraceActionList* trace, Call::CallList* callList);
		Call* top(Call::CallList* callList);
		int top();
		void endTrace(TraceActionList* trace, Call::CallList* callList);
		void updateRootCallTimeBounds(Call::CallList* callList);

	public:
		String toString();
		int getId();
		int64_t getCpuTime(Call::CallList* callList) const;
		int64_t getRealTime(Call::CallList* callList) const;

		// Generated

	public:
		ThreadData();
		ThreadData(int id, String name, MethodData* topLevel, Call::CallList* callList);
		~ThreadData();
		struct Less : public std::binary_function<ThreadData*, ThreadData*, bool> {
			TimeBase* timeBase;
			Call::CallList* callList;

			Less(TimeBase* timeBase, Call::CallList* callList) {
				this->timeBase = timeBase;
				this->callList = callList;
			}

			bool operator() (const ThreadData* _Left, const ThreadData* _Right) const {
				return timeBase->getTime(_Left, callList) < timeBase->getTime(_Right, callList);
			}
		};
	};
};
