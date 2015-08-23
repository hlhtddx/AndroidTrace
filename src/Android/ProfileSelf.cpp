#include "ProfileSelf.hpp"
#include "MethodData.hpp"

namespace Android {

	ProfileSelf::ProfileSelf(MethodData* methodData)
	{
		mElement = methodData;
		mContext = methodData;
	}

	const char* ProfileSelf::getProfileName()
	{
		return "self";
	}

	uint32_t ProfileSelf::getElapsedInclusiveCpuTime() const
	{
		return mElement->getTopExclusiveCpuTime();
	}

	uint32_t ProfileSelf::getElapsedInclusiveRealTime() const
	{
		return mElement->getTopExclusiveRealTime();
	}
};
