
#include "MethodData.hpp"
#include "ProfileSelf.hpp"

namespace Android {

	MethodData::MethodData()
		: MethodData(-100, "Not inited")
	{
	}

	MethodData::MethodData(id_type id, const char* className)
		: MethodData(id, className, "", "", "", -1)
	{
	}

	MethodData::MethodData(id_type id, const char*  className, const char*  methodName, const char*  signature, const char*  pathname, int lineNumber)
		: mRank(-1)
		, mClassName(className)
		, mMethodName(methodName)
		, mSignature(signature)
		, mPathname(pathname)
	{
		mId = id;
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
		for (auto it = mProfileNodes.begin(); it != mProfileNodes.end(); it++) {
			ProfileNode* node = *it;
			delete node;
		}
	}

	MethodData::ProfileDataMap::~ProfileDataMap()
	{
		for (auto it = begin(); it != end(); it++) {
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

	void MethodData::addElapsedExclusive(uint32_t cpuTime, uint32_t realTime)
	{
		mElapsedExclusiveCpuTime += cpuTime;
		mElapsedExclusiveRealTime += realTime;
	}

	void MethodData::addElapsedInclusive(uint32_t cpuTime, uint32_t realTime, bool isRecursive, int parent, CallList* callList)
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

	MethodData::ProfileDataMap* MethodData::updateInclusive(uint32_t cpuTime, uint32_t realTime, MethodData* contextMethod, MethodData* elementMethod, bool elementIsParent, MethodData::ProfileDataMap* map)
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
		PDataPtrList* sortedParents = sortProfileData(mParents, timeBase);
		PDataPtrList* sortedChildren = sortProfileData(mChildren, timeBase);
		PDataPtrList* sortedRecursiveParents = sortProfileData(mRecursiveParents, timeBase);
		PDataPtrList* sortedRecursiveChildren = sortProfileData(mRecursiveChildren, timeBase);

		PNodePtrList* nodes = &mProfileNodes;
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

	PDataPtrList* MethodData::sortProfileData(ProfileDataMap* map, TimeBase* timeBase)
	{
		if (map == nullptr) {
			return nullptr;
		}

		Vector<ProfileData*> *sorted = map->value_vector();
		std::sort(sorted->begin(), sorted->end(), ProfileData::Less(timeBase));
		return sorted;
	}

	PDataPtrList* MethodData::addSelf(PDataPtrList* children)
	{
		if (children == nullptr) {
			children = new Vector<ProfileData*>;
		}
		children->insert(children->begin(), new ProfileSelf(this));
		return children;
	}

	void MethodData::addTopExclusive(uint32_t cpuTime, uint32_t realTime)
	{
		mTopExclusiveCpuTime += cpuTime;
		mTopExclusiveRealTime += realTime;
	}

	uint32_t MethodData::getTopExclusiveCpuTime() const
	{
		return mTopExclusiveCpuTime;
	}

	uint32_t MethodData::getTopExclusiveRealTime() const
	{
		return mTopExclusiveRealTime;
	}

	id_type MethodData::getId()
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

	const char* MethodData::getName() const
	{
		return mName.c_str();
	}

	const char* MethodData::getClassName() const
	{
		return mClassName.c_str();
	}

	const char* MethodData::getMethodName() const
	{
		return mMethodName.c_str();
	}

	const char* MethodData::getProfileName() const
	{
		return mProfileName.c_str();
	}

	const char* MethodData::getSignature() const
	{
		return mSignature.c_str();
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

	const char* MethodData::getCalls(String& outString)
	{
		std::stringstream ss;
		ss << mNumCalls[0] << "+" << mNumCalls[1];
		outString = ss.str();
		return outString.c_str();
	}

	int MethodData::getTotalCalls()
	{
		return mNumCalls[0] + mNumCalls[1];
	}

	COLOR MethodData::getColor()
	{
		return mColor;
	}

	void MethodData::setColor(COLOR color)
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

	uint32_t MethodData::getElapsedExclusiveCpuTime() const
	{
		return mElapsedExclusiveCpuTime;
	}

	uint32_t MethodData::getElapsedExclusiveRealTime() const
	{
		return mElapsedExclusiveRealTime;
	}

	uint32_t MethodData::getElapsedInclusiveCpuTime() const
	{
		return mElapsedInclusiveCpuTime;
	}

	uint32_t MethodData::getElapsedInclusiveRealTime() const
	{
		return mElapsedInclusiveRealTime;
	}

	void MethodData::setFadedColor(COLOR fadedColor)
	{
		mFadedColor = fadedColor;
	}

	COLOR MethodData::getFadedColor()
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

	void MethodData::setPathname(const char* pathname)
	{
		mPathname = pathname;
	}

	const char* MethodData::getPathname()
	{
		return mPathname.c_str();
	}

	void MethodData::setLineNumber(int lineNumber)
	{
		mLineNumber = lineNumber;
	}

	int MethodData::getLineNumber()
	{
		return mLineNumber;
	}

	PNodePtrList* MethodData::getProfileNodes()
	{
		return &mProfileNodes;
	}


};
