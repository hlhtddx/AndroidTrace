

#pragma once
#include "Call.hpp"

namespace Android {

#ifdef CLOCK_SOURCE_THREAD_CPU
    typedef enum ActionType {
		ACTION_ENTER = 0,
		ACTION_EXIT = 1,
		ACTION_INCOMPLETE = 2,
	} ActionType;

	class TraceAction final
	{
	public:
		int mAction;
		int mCall;

	public:
		TraceAction(ActionType action, int call)
		{
			mAction = action;
			mCall = call;
		}
	};
#endif
};
