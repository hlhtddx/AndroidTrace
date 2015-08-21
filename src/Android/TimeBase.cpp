#include "TimeBase.hpp"
#include "MethodData.hpp"
#include "ThreadData.hpp"
#include "ProfileData.hpp"

namespace Android {
	CpuTimeBase CpuTimeBase::TIME;
	RealTimeBase RealTimeBase::TIME;

	TimeBase* TimeBase::CPU_TIME = &CpuTimeBase::TIME;
	TimeBase* TimeBase::REAL_TIME = &RealTimeBase::TIME;

	int64_t CpuTimeBase::getTime(const ThreadData* threadData, Call::CallList* callList)
	{
		return threadData->getCpuTime(callList);
	}

	int64_t CpuTimeBase::getElapsedInclusiveTime(const MethodData* methodData)
	{
		return methodData->getElapsedInclusiveCpuTime();
	}

	int64_t CpuTimeBase::getElapsedExclusiveTime(const MethodData* methodData)
	{
		return methodData->getElapsedExclusiveCpuTime();
	}

	int64_t CpuTimeBase::getElapsedInclusiveTime(const ProfileData* profileData)
	{
		return profileData->getElapsedInclusiveCpuTime();
	}

	int64_t RealTimeBase::getTime(const ThreadData* threadData, Call::CallList* callList)
	{
		return threadData->getRealTime(callList);
	}

	int64_t RealTimeBase::getElapsedInclusiveTime(const MethodData* methodData)
	{
		return methodData->getElapsedInclusiveRealTime();
	}

	int64_t RealTimeBase::getElapsedExclusiveTime(const MethodData* methodData)
	{
		return methodData->getElapsedExclusiveRealTime();
	}

	int64_t RealTimeBase::getElapsedInclusiveTime(const ProfileData* profileData)
	{
		return profileData->getElapsedInclusiveRealTime();
	}
};
