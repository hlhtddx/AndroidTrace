#pragma once
#include "Common.hpp"
#include "ProfileData.hpp"

namespace Android {

	class MethodData;
	class ProfileData;
	typedef Vector<ProfileData*> ProfileDataList;

	class ProfileNode : public Object
	{
	private:
		String mLabel;
		MethodData* mMethodData;
		ProfileDataList* mChildren;
		bool mIsParent;
		bool mIsRecursive;
	public:
		const char* getLabel();
		ProfileDataList* getChildren();
		bool isParent();
		bool isRecursive();
		ProfileNode(const char* label, MethodData* methodData, ProfileDataList* children, bool isParent, bool isRecursive);
		~ProfileNode();
	};
};
