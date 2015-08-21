#include "ProfileData.hpp"
#include "MethodData.hpp"

namespace Android {

	ProfileData::ProfileData()
	{
		mContext = nullptr;
		mElement = nullptr;
		mElementIsParent = false;
	}

	ProfileData::ProfileData(MethodData* context, MethodData* element, bool elementIsParent)
	{
		mContext = context;
		mElement = element;
		mElementIsParent = elementIsParent;
	}

	bool ProfileData::Less::operator() (const ProfileData* _Left, const ProfileData* _Right) const
	{
		return (timeBase->getElapsedInclusiveTime(_Left) < timeBase->getElapsedInclusiveTime(_Right));
	}

//	bool ProfileData::operator < (const ProfileData&)

	const char* ProfileData::getProfileName()
	{
		return mElement->getProfileName().c_str();
	}

	MethodData* ProfileData::getMethodData()
	{
		return mElement;
	}

	void ProfileData::addElapsedInclusive(int64_t cpuTime, int64_t realTime)
	{
		mElapsedInclusiveCpuTime += cpuTime;
		mElapsedInclusiveRealTime += realTime;
		mNumCalls += 1;
	}

	void ProfileData::setElapsedInclusive(int64_t cpuTime, int64_t realTime)
	{
		mElapsedInclusiveCpuTime = cpuTime;
		mElapsedInclusiveRealTime = realTime;
	}

	int64_t ProfileData::getElapsedInclusiveCpuTime() const
	{
		return mElapsedInclusiveCpuTime;
	}

	int64_t ProfileData::getElapsedInclusiveRealTime() const
	{
		return mElapsedInclusiveRealTime;
	}

	void ProfileData::setNumCalls(int numCalls)
	{
		mNumCalls = numCalls;
	}

	void ProfileData::getNumCalls(String& out_string)
	{
		//    int totalCalls;
		//    if(mElementIsParent) {
		//        totalCalls = mContext->getTotalCalls();
		//    } else
		//        totalCalls = mElement->getTotalCalls();
		//    out_string = "%d/%d";
	}

	bool ProfileData::isParent()
	{
		return mElementIsParent;
	}

	MethodData* ProfileData::getContext()
	{
		return mContext;
	}

};