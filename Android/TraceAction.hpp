

#pragma once
#include "Call.hpp"

namespace Android {

	class TraceAction final : public Object
	{

	public:
		typedef enum ActionType {
			ACTION_ENTER = 0,
			ACTION_EXIT = 1,
			ACTION_INCOMPLETE = 2,
		} ActionType;

		int mAction;
		int mCall;

	public:
		TraceAction(ActionType action, int call)
		{
			mAction = action;
			mCall = call;
		}
	};
};
