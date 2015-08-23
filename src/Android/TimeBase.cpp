#include "TimeBase.hpp"
#include "MethodData.hpp"
#include "ThreadData.hpp"
#include "ProfileData.hpp"

namespace Android {
	CpuTimeBase CpuTimeBase::TIME;
	RealTimeBase RealTimeBase::TIME;

	TimeBase* TimeBase::CPU_TIME = &CpuTimeBase::TIME;
	TimeBase* TimeBase::REAL_TIME = &RealTimeBase::TIME;

	uint32_t CpuTimeBase::getTime(const ThreadData* threadData, CallList* callList)
	{
		return threadData->getCpuTime(callList);
	}

	uint32_t CpuTimeBase::getElapsedInclusiveTime(const MethodData* methodData)
	{
		return methodData->getElapsedInclusiveCpuTime();
	}

	uint32_t CpuTimeBase::getElapsedExclusiveTime(const MethodData* methodData)
	{
		return methodData->getElapsedExclusiveCpuTime();
	}

	uint32_t CpuTimeBase::getElapsedInclusiveTime(const ProfileData* profileData)
	{
		return profileData->getElapsedInclusiveCpuTime();
	}

	uint32_t RealTimeBase::getTime(const ThreadData* threadData, CallList* callList)
	{
		return threadData->getRealTime(callList);
	}

	uint32_t RealTimeBase::getElapsedInclusiveTime(const MethodData* methodData)
	{
		return methodData->getElapsedInclusiveRealTime();
	}

	uint32_t RealTimeBase::getElapsedExclusiveTime(const MethodData* methodData)
	{
		return methodData->getElapsedExclusiveRealTime();
	}

	uint32_t RealTimeBase::getElapsedInclusiveTime(const ProfileData* profileData)
	{
		return profileData->getElapsedInclusiveRealTime();
	}
};
