#pragma once

#include "Common.hpp"
#include "Call.hpp"
#include "ProfileData.hpp"
#include "ProfileNode.hpp"
#include "TimeBase.hpp"

namespace Android {

	class MethodData : public Object
	{
	private:
		id_type mId;
		int mRank;
		String mClassName;
		String mMethodName;
		String mSignature;
		String mName;
		String mProfileName;
		String mPathname;
		int mLineNumber;
		uint32_t mElapsedExclusiveCpuTime;
		uint32_t mElapsedInclusiveCpuTime;
		uint32_t mTopExclusiveCpuTime;
		uint32_t mElapsedExclusiveRealTime;
		uint32_t mElapsedInclusiveRealTime;
		uint32_t mTopExclusiveRealTime;
		int mNumCalls[2];
		COLOR mColor;
		COLOR mFadedColor;

		class ProfileDataMap : public HashMap<id_type, ProfileData*>
		{
		public:
			~ProfileDataMap();
		};

		ProfileDataMap* mParents;
		ProfileDataMap* mChildren;
		ProfileDataMap* mRecursiveParents;
		ProfileDataMap* mRecursiveChildren;
		PNodePtrList mProfileNodes;

		int mX;
		int mY;
		double mWeight;

	public:
		double addWeight(int x, int y, double weight);
		void clearWeight();
		int getRank();
		void setRank(int rank);
		void addElapsedExclusive(uint32_t cpuTime, uint32_t realTime);
		void addElapsedInclusive(uint32_t cpuTime, uint32_t realTime, bool isRecursive, int parent, CallList* callList);

	private:
		ProfileDataMap* updateInclusive(uint32_t cpuTime, uint32_t realTime, MethodData* contextMethod, MethodData* elementMethod, bool elementIsParent, ProfileDataMap* map);

	public:
		void analyzeData(TimeBase* timeBase);

	private:
		PDataPtrList* sortProfileData(ProfileDataMap* map, TimeBase* timeBase);
		PDataPtrList* addSelf(PDataPtrList* children);

	public:
		void addTopExclusive(uint32_t cpuTime, uint32_t realTime);
		uint32_t getTopExclusiveCpuTime() const;
		uint32_t getTopExclusiveRealTime() const;
		id_type getId();

	private:
		void computeName();

	public:
		const char* getName() const;
		const char* getClassName() const;
		const char* getMethodName() const;
		const char* getProfileName() const;
		const char* getSignature() const;
		void computeProfileName();
		const char* getCalls(String& outString);
		int getTotalCalls();
		COLOR getColor();
		void setColor(COLOR color);

		uint32_t getElapsedExclusiveCpuTime() const;
		uint32_t getElapsedExclusiveRealTime() const;
		uint32_t getElapsedInclusiveCpuTime() const;
		uint32_t getElapsedInclusiveRealTime() const;

		void setFadedColor(COLOR fadedColor);
		COLOR getFadedColor();
		void setPathname(const char*pathname);
		const char* getPathname();
		void setLineNumber(int lineNumber);
		int getLineNumber();
		PNodePtrList* getProfileNodes();

		MethodData();
		MethodData(id_type id, const char* className);
		MethodData(id_type id, const char* className, const char* methodName, const char* signature, const char* pathname, int lineNumber);
		~MethodData();

		struct Greater : public std::binary_function<ThreadData*, ThreadData*, bool> {
			TimeBase* timeBase;

            Greater(TimeBase* timeBase) {
				this->timeBase = timeBase;
			}

			bool operator() (const MethodData* _Left, const MethodData* _Right) const {
				if (timeBase->getElapsedInclusiveTime(_Left) == timeBase->getElapsedInclusiveTime(_Right)) {
					return _Left->getName() > _Right->getName();
				}
				return timeBase->getElapsedInclusiveTime(_Left) > timeBase->getElapsedInclusiveTime(_Right);
			}
		};
	};

};
