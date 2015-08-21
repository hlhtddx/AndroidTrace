#include "Selection.hpp"

namespace Android {

	Selection::Selection(Action action, const char* name, Object* value)
	{
		mAction = action;
		mName = name;
		mValue = value;
	}

	Selection* Selection::highlight(const char* name, Object* value)
	{
		return new Selection(Highlight, name, value);
	}

	Selection* Selection::include(const char* name, Object* value)
	{
		return new Selection(Include, name, value);
	}

	Selection* Selection::exclude(const char* name, Object* value)
	{
		return new Selection(Exclude, name, value);
	}

	void Selection::setName(const char* name)
	{
		mName = name;
	}

	const char* Selection::getName()
	{
		return mName.c_str();
	}

	void Selection::setValue(Object* value)
	{
		mValue = value;
	}

	Object* Selection::getValue()
	{
		return mValue;
	}

	void Selection::setAction(Action action)
	{
		mAction = action;
	}

	Action Selection::getAction()
	{
		return mAction;
	}
};
