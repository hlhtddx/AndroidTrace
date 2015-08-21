#pragma once

#include "Common.hpp"
#include "ProfileData.hpp"

namespace Android {

	class ProfileSelf
		: public ProfileData
	{
	public:
		const char* getProfileName() override;
		int64_t getElapsedInclusiveCpuTime() const override;
		int64_t getElapsedInclusiveRealTime() const override;

		// Generated
		ProfileSelf(MethodData* methodData);
	};
};
