// Generated from /Traceview/src/com/android/traceview/Selection.java

#pragma once

#include "Common.hpp"

namespace Android {

	class Selection : public Object
	{
	public:
		typedef enum Action {
			Highlight, Include, Exclude, Aggregate,
		} Action;


	private:
		Action mAction;
		String mName;
		Object* mValue;

	public:
		static Selection* highlight(const char* name, Object* value);
		static Selection* include(const char* name, Object* value);
		static Selection* exclude(const char* name, Object* value);
		void setName(const char* name);
		const char* getName();
		void setValue(Object* value);
		Action getValue();
		void setAction(Action action);
		Action getAction();

		// Generated
		Selection(Action action, const char* name, Object* value);
	};

};