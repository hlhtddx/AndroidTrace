#pragma once

#include "Common.hpp"

namespace Android {

	class MethodData;
	class TimeBase;

	class ProfileData : public Object
	{
	public: /* protected */
		MethodData* mElement;
		MethodData* mContext;
		bool mElementIsParent;
		uint32_t mElapsedInclusiveCpuTime;
		uint32_t mElapsedInclusiveRealTime;
		int mNumCalls;

	public:
//		bool operator < (const ProfileData&);
		virtual const char* getProfileName();
		MethodData* getMethodData();
		void addElapsedInclusive(uint32_t cpuTime, uint32_t realTime);
		void setElapsedInclusive(uint32_t cpuTime, uint32_t realTime);
		virtual uint32_t getElapsedInclusiveCpuTime() const;
		virtual uint32_t getElapsedInclusiveRealTime() const;
		void setNumCalls(int numCalls);
		void getNumCalls(String& outString);
		bool isParent();
		MethodData* getContext();

		// Generated
		ProfileData();
		ProfileData(MethodData* context, MethodData* element, bool elementIsParent);

		struct Less : public std::binary_function<ProfileData*, ProfileData*, bool> {
			TimeBase* timeBase;
			Less(TimeBase* timeBase) {
				this->timeBase = timeBase;
			}
			bool operator() (const ProfileData* _Left, const ProfileData* _Right) const;
		};
	};
};
