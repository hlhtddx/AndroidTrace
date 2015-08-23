#include "ProfileNode.hpp"
#include "MethodData.hpp"
#include "ProfileData.hpp"

namespace Android {

	ProfileNode::ProfileNode(const char* label, MethodData* methodData, PDataPtrList* children, bool isParent, bool isRecursive)
	{
		mLabel = label;
		mMethodData = methodData;
		mChildren = children;
		mIsParent = isParent;
		mIsRecursive = isRecursive;
	}

	ProfileNode::~ProfileNode()
	{
		if (mChildren) {
			delete mChildren;
		}
	}

	const char* ProfileNode::getLabel()
	{
		return mLabel.c_str();
	}

	PDataPtrList* ProfileNode::getChildren()
	{
		return mChildren;
	}

	bool ProfileNode::isParent()
	{
		return mIsParent;
	}

	bool ProfileNode::isRecursive()
	{
		return mIsRecursive;
	}
};