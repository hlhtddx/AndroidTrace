#pragma once

#include "Common.hpp"
#include "Call.hpp"
#include "ProfileData.hpp"
#include "ProfileNode.hpp"
#include "TimeBase.hpp"

namespace Android {

	typedef Vector<ProfileNode*> ProfileNodeList;
	typedef Vector<ProfileData*> ProfileDataList;

	class MethodData : public Object
	{
	private:
		uint32_t mId;
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
		int mColor;
		int mFadedColor;

		class ProfileDataMap : public HashMap<uint32_t, ProfileData*>
		{
		public:
			~ProfileDataMap();
		};

		ProfileDataMap* mParents;
		ProfileDataMap* mChildren;
		ProfileDataMap* mRecursiveParents;
		ProfileDataMap* mRecursiveChildren;
		ProfileNodeList mProfileNodes;

		int mX;
		int mY;
		double mWeight;

	public:
		double addWeight(int x, int y, double weight);
		void clearWeight();
		int getRank();
		void setRank(int rank);
		void addElapsedExclusive(uint32_t cpuTime, uint32_t realTime);
		void addElapsedInclusive(uint32_t cpuTime, uint32_t realTime, bool isRecursive, int parent, Call::CallList* callList);

	private:
		ProfileDataMap* updateInclusive(uint32_t cpuTime, uint32_t realTime, MethodData* contextMethod, MethodData* elementMethod, bool elementIsParent, ProfileDataMap* map);

	public:
		void analyzeData(TimeBase* timeBase);

	private:
		ProfileDataList* sortProfileData(ProfileDataMap* map, TimeBase* timeBase);
		ProfileDataList* addSelf(ProfileDataList* children);

	public:
		void addTopExclusive(uint32_t cpuTime, uint32_t realTime);
		uint32_t getTopExclusiveCpuTime() const;
		uint32_t getTopExclusiveRealTime() const;
		uint32_t getId();

	private:
		void computeName();

	public:
		const char* getName() const;
		const char* getClassName() const;
		const char* getMethodName() const;
		const char* getProfileName() const;
		const char* getSignature() const;
		void computeProfileName();
		const char* getCalls(String& outstring);
		int getTotalCalls();
		uint32_t getColor();
		void setColor(uint32_t color);
		String toString();

		uint32_t getElapsedExclusiveCpuTime() const;
		uint32_t getElapsedExclusiveRealTime() const;
		uint32_t getElapsedInclusiveCpuTime() const;
		uint32_t getElapsedInclusiveRealTime() const;

		void setFadedColor(int fadedColor);
		int getFadedColor();
		//	void setFadedImage(::org::eclipse::swt::graphics::Image* fadedImage);
		//	::org::eclipse::swt::graphics::Image* getFadedImage();
		void setPathname(const String & pathname);
		const String & getPathname();
		void setLineNumber(int lineNumber);
		int getLineNumber();
		ProfileNodeList* getProfileNodes();
		// Generated
		MethodData();
		MethodData(uint32_t id, String className);
		MethodData(uint32_t id, String className, String methodName, String signature, String pathname, int lineNumber);
		~MethodData();
		struct Less : public std::binary_function<ThreadData*, ThreadData*, bool> {
			TimeBase* timeBase;

			Less(TimeBase* timeBase) {
				this->timeBase = timeBase;
			}

			bool operator() (const MethodData* _Left, const MethodData* _Right) const {
				if (timeBase->getElapsedInclusiveTime(_Left) == timeBase->getElapsedInclusiveTime(_Right)) {
					return _Left->getName() < _Right->getName();
				}
				return timeBase->getElapsedInclusiveTime(_Left) < timeBase->getElapsedInclusiveTime(_Right);
			}
		};
	};

};
