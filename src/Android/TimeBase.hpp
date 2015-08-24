

#pragma once

#include "Common.hpp"
#include "Call.hpp"

namespace Android {
	class ThreadData;
	class MethodData;
	class ProfileData;

	class TimeBase : public Object
	{
	public:
		static TimeBase* CPU_TIME;
		static TimeBase* REAL_TIME;

	public:
		virtual uint32_t getTime(const ThreadData* paramThreadData) = 0;
		virtual uint32_t getElapsedInclusiveTime(const MethodData* paramMethodData) = 0;
		virtual uint32_t getElapsedExclusiveTime(const MethodData* paramMethodData) = 0;
		virtual uint32_t getElapsedInclusiveTime(const ProfileData* paramProfileData) = 0;
	};

	class CpuTimeBase final
		: public TimeBase
	{
	public:
		static CpuTimeBase TIME;
		virtual uint32_t getTime(const ThreadData* threadData) override;
		virtual uint32_t getElapsedInclusiveTime(const MethodData* methodData) override;
		virtual uint32_t getElapsedExclusiveTime(const MethodData* methodData) override;
		virtual uint32_t getElapsedInclusiveTime(const ProfileData* profileData) override;
	};

	class RealTimeBase final
		: public TimeBase
	{
	public:
		static RealTimeBase TIME;
		virtual uint32_t getTime(const ThreadData* threadData) override;
		virtual uint32_t getElapsedInclusiveTime(const MethodData* methodData) override;
		virtual uint32_t getElapsedExclusiveTime(const MethodData* methodData) override;
		virtual uint32_t getElapsedInclusiveTime(const ProfileData* profileData) override;
	};
};
