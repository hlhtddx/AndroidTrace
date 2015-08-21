

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
		virtual int64_t getTime(const ThreadData* paramThreadData, Call::CallList* callList) = 0;
		virtual int64_t getElapsedInclusiveTime(const MethodData* paramMethodData) = 0;
		virtual int64_t getElapsedExclusiveTime(const MethodData* paramMethodData) = 0;
		virtual int64_t getElapsedInclusiveTime(const ProfileData* paramProfileData) = 0;
	};

	class CpuTimeBase final
		: public TimeBase
	{
	public:
		static CpuTimeBase TIME;
		virtual int64_t getTime(const ThreadData* threadData, Call::CallList* callList) override;
		virtual int64_t getElapsedInclusiveTime(const MethodData* methodData) override;
		virtual int64_t getElapsedExclusiveTime(const MethodData* methodData) override;
		virtual int64_t getElapsedInclusiveTime(const ProfileData* profileData) override;
	};

	class RealTimeBase final
		: public TimeBase
	{
	public:
		static RealTimeBase TIME;
		virtual int64_t getTime(const ThreadData* threadData, Call::CallList* callList) override;
		virtual int64_t getElapsedInclusiveTime(const MethodData* methodData) override;
		virtual int64_t getElapsedExclusiveTime(const MethodData* methodData) override;
		virtual int64_t getElapsedInclusiveTime(const ProfileData* profileData) override;
	};
};
