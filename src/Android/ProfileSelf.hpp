#pragma once

#include "Common.hpp"
#include "ProfileData.hpp"

namespace Android {

	class ProfileSelf
		: public ProfileData
	{
	public:
		const char* getProfileName() override;
		uint32_t getElapsedInclusiveCpuTime() const override;
		uint32_t getElapsedInclusiveRealTime() const override;

		// Generated
		ProfileSelf(MethodData* methodData);
	};
};
