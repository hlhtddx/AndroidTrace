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
		int64_t mElapsedInclusiveCpuTime;
		int64_t mElapsedInclusiveRealTime;
		int mNumCalls;

	public:
//		bool operator < (const ProfileData&);
		virtual const char* getProfileName();
		MethodData* getMethodData();
		void addElapsedInclusive(int64_t cpuTime, int64_t realTime);
		void setElapsedInclusive(int64_t cpuTime, int64_t realTime);
		virtual int64_t getElapsedInclusiveCpuTime() const;
		virtual int64_t getElapsedInclusiveRealTime() const;
		void setNumCalls(int numCalls);
		void getNumCalls(String& out_string);
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
