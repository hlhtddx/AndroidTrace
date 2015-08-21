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
		int64_t mId;
		int mRank;
		String mClassName;
		String mMethodName;
		String mSignature;
		String mName;
		String mProfileName;
		String mPathname;
		int mLineNumber;
		int64_t mElapsedExclusiveCpuTime;
		int64_t mElapsedInclusiveCpuTime;
		int64_t mTopExclusiveCpuTime;
		int64_t mElapsedExclusiveRealTime;
		int64_t mElapsedInclusiveRealTime;
		int64_t mTopExclusiveRealTime;
		int mNumCalls[2];
		int mColor;
		int mFadedColor;
		//    ::org::eclipse::swt::graphics::Image* mImage;
		//    ::org::eclipse::swt::graphics::Image* mFadedImage;
		class ProfileDataMap : public HashMap<int64_t, ProfileData*>
		{
		public:
			~ProfileDataMap();
		};
		typedef Vector<ProfileNode*> ProfileNodeList;
		typedef Vector<ProfileData*> ProfileDataList;

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
		void addElapsedExclusive(int64_t cpuTime, int64_t realTime);
		void addElapsedInclusive(int64_t cpuTime, int64_t realTime, bool isRecursive, int parent, Call::CallList* callList);

	private:
		ProfileDataMap* updateInclusive(int64_t cpuTime, int64_t realTime, MethodData* contextMethod, MethodData* elementMethod, bool elementIsParent, ProfileDataMap* map);

	public:
		void analyzeData(TimeBase* timeBase);

	private:
		ProfileDataList* sortProfileData(ProfileDataMap* map, TimeBase* timeBase);
		ProfileDataList* addSelf(ProfileDataList* children);

	public:
		void addTopExclusive(int64_t cpuTime, int64_t realTime);
		int64_t getTopExclusiveCpuTime() const;
		int64_t getTopExclusiveRealTime() const;
		int64_t getId();

	private:
		void computeName();

	public:
		const String& getName() const;
		const String& getClassName() const;
		const String& getMethodName() const;
		const String& getProfileName() const;
		const String& getSignature() const;
		void computeProfileName();
		void getCalls(String& outstring);
		int getTotalCalls();
		int getColor();
		void setColor(int color);
		//void setImage(::org::eclipse::swt::graphics::Image* image);
		//::org::eclipse::swt::graphics::Image* getImage();
		String toString();
		int64_t getElapsedExclusiveCpuTime() const;
		int64_t getElapsedExclusiveRealTime() const;
		int64_t getElapsedInclusiveCpuTime() const;
		int64_t getElapsedInclusiveRealTime() const;
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
		MethodData(int64_t id, String className);
		MethodData(int64_t id, String className, String methodName, String signature, String pathname, int lineNumber);
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
