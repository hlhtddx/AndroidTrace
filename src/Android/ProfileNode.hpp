#pragma once
#include "Common.hpp"
#include "ProfileData.hpp"

namespace Android {

	class MethodData;
	class ProfileData;

	typedef Vector<ProfileData*> PDataPtrList;

	class ProfileNode : public Object
	{
	private:
		String mLabel;
		MethodData* mMethodData;
		PDataPtrList* mChildren;
		bool mIsParent;
		bool mIsRecursive;
	public:
		const char* getLabel();
		PDataPtrList* getChildren();
		bool isParent();
		bool isRecursive();
		ProfileNode(const char* label, MethodData* methodData, PDataPtrList* children, bool isParent, bool isRecursive);
		~ProfileNode();
	};

	typedef Vector<ProfileNode*> PNodePtrList;
};
