
#include "MethodData.hpp"
#include "ProfileSelf.hpp"

namespace Android {

	MethodData::MethodData()
		: MethodData(-100, "Not inited")
	{
	}

	MethodData::MethodData(int64_t id, String className)
		: MethodData(id, className, "", "", "", -1)
	{
	}

	MethodData::MethodData(int64_t id, String className, String methodName, String signature, String pathname, int lineNumber)
		: mRank(-1)
	{
		mId = id;
		mClassName = className;
		mMethodName = methodName;
		mSignature = signature;
		mPathname = pathname;
		mLineNumber = -1;
		mParents = NULL;
		mChildren = NULL;
		mRecursiveParents = NULL;
		mRecursiveChildren = NULL;
		computeName();
		computeProfileName();
	}

	MethodData::~MethodData()
	{
		if (mParents) {
			delete mParents;
		}
		if (mChildren) {
			delete mChildren;
		}
		if (mRecursiveParents) {
			delete mRecursiveParents;
		}
		if (mRecursiveChildren) {
			delete mRecursiveChildren;
		}
		for (ProfileNodeList::iterator it = mProfileNodes.begin(); it != mProfileNodes.end(); it++) {
			ProfileNode* node = *it;
			delete node;
		}
	}

	MethodData::ProfileDataMap::~ProfileDataMap()
	{
		for (iterator it = begin(); it != end(); it++) {
			delete it->second;
		}
	}

	double MethodData::addWeight(int x, int y, double weight)
	{
		if ((mX == x) && (mY == y)) {
			mWeight += weight;
		}
		else {
			mX = x;
			mY = y;
			mWeight = weight;
		}
		return mWeight;
	}

	void MethodData::clearWeight()
	{
		mWeight = 0.0;
	}

	int MethodData::getRank()
	{
		return mRank;
	}

	void MethodData::setRank(int rank)
	{
		mRank = rank;
		computeProfileName();
	}

	void MethodData::addElapsedExclusive(int64_t cpuTime, int64_t realTime)
	{
		mElapsedExclusiveCpuTime += cpuTime;
		mElapsedExclusiveRealTime += realTime;
	}

	void MethodData::addElapsedInclusive(int64_t cpuTime, int64_t realTime, bool isRecursive, int parent, Call::CallList* callList)
	{
		if (!isRecursive) {
			mElapsedInclusiveCpuTime += cpuTime;
			mElapsedInclusiveRealTime += realTime;
			mNumCalls[0] += 1;
		}
		else {
			mNumCalls[1] += 1;
		}

		if (parent == -1) {
			return;
		}

		Call* callParent = callList->get(parent);
		MethodData* parentMethod = callParent->getMethodData();
		if (callParent->isRecursive()) {
			parentMethod->mRecursiveChildren = updateInclusive(cpuTime, realTime, parentMethod, this, false, parentMethod->mRecursiveChildren);
		}
		else {
			parentMethod->mChildren = updateInclusive(cpuTime, realTime, parentMethod, this, false, parentMethod->mChildren);
		}
		if (isRecursive) {
			mRecursiveParents = updateInclusive(cpuTime, realTime, this, parentMethod, true, mRecursiveParents);
		}
		else {
			mParents = updateInclusive(cpuTime, realTime, this, parentMethod, true, mParents);
		}
	}

	MethodData::ProfileDataMap* MethodData::updateInclusive(int64_t cpuTime, int64_t realTime, MethodData* contextMethod, MethodData* elementMethod, bool elementIsParent, MethodData::ProfileDataMap* map)
	{
		if (map == nullptr) {
			map = new MethodData::ProfileDataMap;
		}
		else {
			if (map->find(elementMethod->mId) != map->end()) {
				ProfileData* profileData = map->at(elementMethod->mId);
				profileData->addElapsedInclusive(cpuTime, realTime);
				return map;
			}
		}
		ProfileData* elementData = new ProfileData(contextMethod, elementMethod, elementIsParent);
		map->add(elementMethod->mId, elementData);
		elementData->setElapsedInclusive(cpuTime, realTime);
		elementData->setNumCalls(1);
		return map;
	}

	void MethodData::analyzeData(TimeBase* timeBase)
	{
		ProfileDataList* sortedParents = sortProfileData(mParents, timeBase);
		ProfileDataList* sortedChildren = sortProfileData(mChildren, timeBase);
		ProfileDataList* sortedRecursiveParents = sortProfileData(mRecursiveParents, timeBase);
		ProfileDataList* sortedRecursiveChildren = sortProfileData(mRecursiveChildren, timeBase);

		ProfileNodeList* nodes = &mProfileNodes;
		nodes->clear();

		if (mParents != nullptr) {
			nodes->push_back(new ProfileNode("Parents", this, sortedParents, true, false));
		}
		if (mChildren != nullptr) {
			sortedChildren = addSelf(sortedChildren);
			nodes->push_back(new ProfileNode("Children", this, sortedChildren, false, false));
		}
		if (mRecursiveParents != nullptr) {
			nodes->push_back(new ProfileNode("Parents while recursive", this, sortedRecursiveParents, true, true));
		}
		if (mRecursiveChildren != nullptr) {
			nodes->push_back(new ProfileNode("Children while recursive", this, sortedRecursiveChildren, false, true));
		}
	}

	MethodData::ProfileDataList* MethodData::sortProfileData(ProfileDataMap* map, TimeBase* timeBase)
	{
		if (map == nullptr) {
			return nullptr;
		}

		Vector<ProfileData*> *sorted = map->value_vector();
		std::sort(sorted->begin(), sorted->end(), ProfileData::Less(timeBase));
		return sorted;
	}

	MethodData::ProfileDataList* MethodData::addSelf(MethodData::ProfileDataList* children)
	{
		if (children == nullptr) {
			children = new Vector<ProfileData*>;
		}
		children->insert(children->begin(), new ProfileSelf(this));
		return children;
	}

	void MethodData::addTopExclusive(int64_t cpuTime, int64_t realTime)
	{
		mTopExclusiveCpuTime += cpuTime;
		mTopExclusiveRealTime += realTime;
	}

	int64_t MethodData::getTopExclusiveCpuTime() const
	{
		return mTopExclusiveCpuTime;
	}

	int64_t MethodData::getTopExclusiveRealTime() const
	{
		return mTopExclusiveRealTime;
	}

	int64_t MethodData::getId()
	{
		return mId;
	}

	void MethodData::computeName()
	{
		if (mMethodName.empty()) {
			mName = mClassName;
			return;
		}

		mName = mClassName + '.' + mMethodName + " " + mSignature;
	}

	const String& MethodData::getName() const
	{
		return mName;
	}

	const String& MethodData::getClassName() const
	{
		return mClassName;
	}

	const String& MethodData::getMethodName() const
	{
		return mMethodName;
	}

	const String& MethodData::getProfileName() const
	{
		return mProfileName;
	}

	const String& MethodData::getSignature() const
	{
		return mSignature;
	}

	void MethodData::computeProfileName()
	{
		if (mRank == -1) {
			mProfileName = mName;
			return;
		}

		std::stringstream ss;
		ss << mRank << " " << getName();
		mProfileName = ss.str();
	}

	void MethodData::getCalls(String& outstring)
	{
		std::stringstream ss;
		ss << mNumCalls[0] << "+" << mNumCalls[1];
		outstring = ss.str();
	}

	int MethodData::getTotalCalls()
	{
		return mNumCalls[0] + mNumCalls[1];
	}

	int MethodData::getColor()
	{
		return mColor;
	}

	void MethodData::setColor(int color)
	{
		mColor = color;
	}

#if 0
	void MethodData::setImage(::org::eclipse::swt::graphics::Image* image)
	{
		mImage = image;
	}

	org::eclipse::swt::graphics::Image* MethodData::getImage()
	{
		return mImage;
	}
#endif

	int64_t MethodData::getElapsedExclusiveCpuTime() const
	{
		return mElapsedExclusiveCpuTime;
	}

	int64_t MethodData::getElapsedExclusiveRealTime() const
	{
		return mElapsedExclusiveRealTime;
	}

	int64_t MethodData::getElapsedInclusiveCpuTime() const
	{
		return mElapsedInclusiveCpuTime;
	}

	int64_t MethodData::getElapsedInclusiveRealTime() const
	{
		return mElapsedInclusiveRealTime;
	}

	void MethodData::setFadedColor(int fadedColor)
	{
		mFadedColor = fadedColor;
	}

	int MethodData::getFadedColor()
	{
		return mFadedColor;
	}

#if 0
	void MethodData::setFadedImage(::org::eclipse::swt::graphics::Image* fadedImage)
	{
		mFadedImage = fadedImage;
	}

	org::eclipse::swt::graphics::Image* MethodData::getFadedImage()
	{
		return mFadedImage;
	}
#endif

	void MethodData::setPathname(const String &pathname)
	{
		mPathname = pathname;
	}

	const String & MethodData::getPathname()
	{
		return mPathname;
	}

	void MethodData::setLineNumber(int lineNumber)
	{
		mLineNumber = lineNumber;
	}

	int MethodData::getLineNumber()
	{
		return mLineNumber;
	}

	MethodData::ProfileNodeList* MethodData::getProfileNodes()
	{
		return &mProfileNodes;
	}


};
