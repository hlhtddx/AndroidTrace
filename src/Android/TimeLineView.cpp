#include "TimeLineView.hpp"
#include "Selection.hpp"
#include "TraceUnits.hpp"

namespace Android {

	Range::Range(int xStart, int width, int y, COLOR color)
	{
		mXdim.x = xStart;
		mXdim.y = width;
		mY = y;
		mColor = color;
	}

	Strip::Strip(int x, int y, int width, int height, RowData* rowData, Segment* segment, COLOR color)
	{
		mX = x;
		mY = y;
		mWidth = width;
		mHeight = height;
		mRowData = rowData;
		mSegment = segment;
		mColor = color;
	}

	RowData::RowData(ThreadData* row)
	{
		mName = row->getName();
	}

	void RowData::push(int index)
	{
		mStack.push_back(index);
	}

	int RowData::top()
	{
		if (mStack.size() == 0) {
			return -1;
		}
		return mStack.back();
	}

	void RowData::pop()
	{
		if (mStack.size() == 0) {
			return;
		}

		mStack.pop_back();
	}

	void TickScaler::computeTicks(bool useGivenEndPoints)
	{
		int numTicks = mNumPixels / mPixelsPerTick;
		mRangeVal = (mMaxVal - mMinVal);
		mTickIncrement = (mRangeVal / numTicks);
		double dlogTickIncrement = log10(mTickIncrement);
		double logTickIncrement = floor(dlogTickIncrement);
		double scale = pow(10.0, logTickIncrement);
		double scaledTickIncr = mTickIncrement / scale;

		if (scaledTickIncr > 5.0) {
			scaledTickIncr = 10.0;
		}
		else if (scaledTickIncr > 2.0) {
			scaledTickIncr = 5.0;
		}
		else if (scaledTickIncr > 1.0) {
			scaledTickIncr = 2.0;
		}
		else
			scaledTickIncr = 1.0;

		mTickIncrement = (scaledTickIncr * scale);

		if (!useGivenEndPoints) {
			double minorTickIncrement = mTickIncrement / 5.0;
			double dval = mMaxVal / minorTickIncrement;
			int ival = (int)dval;
			if (ival != dval) {
				mMaxVal = ((ival + 1) * minorTickIncrement);
			}
			ival = static_cast<int>((mMinVal / mTickIncrement));
			mMinVal = (ival * mTickIncrement);
			mMinMajorTick = mMinVal;
		}
		else {
			auto ival = static_cast<int>((mMinVal / mTickIncrement));
			mMinMajorTick = (ival * mTickIncrement);
			if (mMinMajorTick < mMinVal) {
				mMinMajorTick += mTickIncrement;
			}
		}
		mRangeVal = (mMaxVal - mMinVal);
		mPixelsPerRange = (mNumPixels / mRangeVal);
	}

	Pixel::Pixel()
	{
		mStart = -2;
	}

	void Pixel::setFields(int start, double weight, Segment* segment, COLOR color, RowData* rowData)
	{
		mStart = start;
		mMaxWeight = weight;
		mSegment = segment;
		mColor = color;
		mRowData = rowData;
	}

	void Canvas::mouseMove(Point& pt, int flags)
	{

	}
	
	void Canvas::mouseDown(Point& pt, int flags)
	{

	}

	void Canvas::mouseUp(Point& pt, int flags)
	{

	}

	void Canvas::mouseScrolled(Point& pt, int flags)
	{

	}

	void Canvas::mouseDoubleClick(Point& pt, int flags)
	{

	}

	int Surface::highlightHeights[] = { 0, 2, 4, 5, 6, 5, 4, 2, 4, 5, 6 };

	Surface::Surface(TimeLineView *parent)
		: mParent(parent)
	{
		mGraphicsState = Normal;
		mMouse.x = 10;
		mMouse.y = 0;
		mDebug = false;
		mMinStripHeight = 2;
		mHighlightHeight = 4;
		//		HIGHLIGHT_STEPS = 0;
		mMouseSelect.x = mMouseSelect.y = 0;
		initZoomFractionsWithExp();
	}

	void Surface::initZoomFractionsWithExp()
	{
		auto next = 0;
		for (auto ii = 0; ii < 4; next++) {
			mZoomFractions[next] = ((1 << ii) / 16.0);
			ii++;
		}
		for (auto ii = 2; ii < 6; next++) {
			mZoomFractions[next] = (((1 << ii) - 1) / (1 << ii));
			ii++;
		}
	}

	void Surface::initZoomFractionsWithSinWave()
	{
		for (auto ii = 0; ii < 8; ii++) {
			auto offset = 3.141592653589793 * ii / 8.0;
			mZoomFractions[ii] = ((sin(4.71238898038469 + offset) + 1.0) / 2.0);
		}
	}

	void Surface::setRange(double minVal, double maxVal)
	{
		mMinDataVal = minVal;
		mMaxDataVal = maxVal;
		mParent->getScaleInfo().setMinVal(minVal);
		mParent->getScaleInfo().setMaxVal(maxVal);
	}

	void Surface::setLimitRange(double minVal, double maxVal)
	{
		mLimitMinVal = minVal;
		mLimitMaxVal = maxVal;
	}

	void Surface::resetScale()
	{
		mParent->getScaleInfo().setMinVal(mLimitMinVal);
		mParent->getScaleInfo().setMaxVal(mLimitMaxVal);
	}

	void Surface::setScaleFromHorizontalScrollBar(int selection)
	{
		auto minVal = mParent->getScaleInfo().getMinVal();
		auto maxVal = mParent->getScaleInfo().getMaxVal();
		auto visibleRange = maxVal - minVal;
		minVal = mLimitMinVal + selection;
		maxVal = minVal + visibleRange;
		if (maxVal > mLimitMaxVal) {
			maxVal = mLimitMaxVal;
			minVal = maxVal - visibleRange;
		}
		mParent->getScaleInfo().setMinVal(minVal);
		mParent->getScaleInfo().setMaxVal(maxVal);
		mGraphicsState = Scrolling;
	}
	/*
		void Surface::updateHorizontalScrollBar()
		{
		auto minVal = mParent->getScaleInfo().getMinVal();
		auto maxVal = mParent->getScaleInfo().getMaxVal();
		auto visibleRange = maxVal - minVal;
		auto fullRange = mLimitMaxVal - mLimitMinVal;
		auto hBar = getHorizontalBar();
		if (fullRange > visibleRange) {
		hBar->setVisible(true);
		hBar->setMinimum(0);
		hBar->setMaximum(static_cast<int>(::java::lang::Math::ceil(fullRange)));
		hBar->setThumb(static_cast<int>(::java::lang::Math::ceil(visibleRange)));
		hBar->setSelection(static_cast<int>(::java::lang::Math::floor(minVal - mLimitMinVal)));
		}
		else {
		hBar->setVisible(false);
		}
		}*/

	void Surface::draw()
	{
		if (mParent->mSegments.size() == 0) {
			return;
		}

		if (mGraphicsState == Scaling) {
			double diff = mMouse.x - mMouseMarkStartX;
			if (diff > 0.0) {
				auto newMinVal = mScaleMinVal - diff / mScalePixelsPerRange;
				if (newMinVal < mLimitMinVal)
					newMinVal = mLimitMinVal;

				mParent->getScaleInfo().setMinVal(newMinVal);
			}
			else if (diff < 0.0) {
				auto newMaxVal = mScaleMaxVal - diff / mScalePixelsPerRange;
				if (newMaxVal > mLimitMaxVal)
					newMaxVal = mLimitMaxVal;

				mParent->getScaleInfo().setMaxVal(newMaxVal);
			}
		}

		Size dim = getSize();

		if ((mParent->mStartRow != mCachedStartRow) || (mParent->mEndRow != mCachedEndRow) || (mParent->getScaleInfo().getMinVal() != mCachedMinVal) || (mParent->getScaleInfo().getMaxVal() != mCachedMaxVal)) {
			mCachedStartRow = mParent->mStartRow;
			mCachedEndRow = mParent->mEndRow;
			auto xdim = dim.cx - 70;
			mParent->getScaleInfo().setNumPixels(xdim);
			auto forceEndPoints = (mGraphicsState == Scaling) || (mGraphicsState == Animating) || (mGraphicsState == Scrolling);
			mParent->getScaleInfo().computeTicks(forceEndPoints);
			mCachedMinVal = mParent->getScaleInfo().getMinVal();
			mCachedMaxVal = mParent->getScaleInfo().getMaxVal();
			if (mLimitMinVal > mParent->getScaleInfo().getMinVal())
				mLimitMinVal = mParent->getScaleInfo().getMinVal();

			if (mLimitMaxVal < mParent->getScaleInfo().getMaxVal()) {
				mLimitMaxVal = mParent->getScaleInfo().getMaxVal();
			}
			computeStrips();
		}
		if (mParent->mNumRows > 2) {
//			gcImage->setBackground(mParent->mColorRowBack);
			for (auto ii = 1; ii < mParent->mNumRows; ii += 2) {
				auto rd = (*mParent->mRows)[ii];
				auto y1 = rd->mRank * 32 - mParent->mScrollOffsetY;
//				gcImage->fillRectangle(0, y1, dim.cx, 32);
			}
		}
		if (drawingSelection()) {
			//drawSelection();
		}

		String blockName;
		COLOR blockColor = 0;
		String blockDetails;
		if (mDebug) {
			auto pixelsPerRange = mParent->getScaleInfo().getPixelsPerRange();
			printf("dim.x %d pixels %d minVal %f, maxVal %f ppr %f rpp %f\n",
				dim.cx, dim.cx - 70,
				mParent->getScaleInfo().getMinVal(), mParent->getScaleInfo().getMaxVal(),
				pixelsPerRange, 1.0 / pixelsPerRange);
		}
		Call* selectBlock = nullptr;
		for (auto it = mStripList.begin(); it != mStripList.end(); it++) {
			Strip* strip = *it;
			if (strip->mColor != 0) {

				if (mParent->mMouseRow == strip->mRowData->mRank) {
					if ((mMouse.x >= strip->mX) && (mMouse.x < strip->mX + strip->mWidth)) {
						auto block = mParent->mCallList->get(strip->mSegment->mBlock);
						blockName = block->getName();
						blockColor = strip->mColor;

						std::stringstream ss(blockDetails);
						String outString1;
						String outString2;
						String outString3;
						String outString4;

						if (mParent->mHaveCpuTime) {
							ss << "excl cpu " << mParent->mUnits->labelledString(block->getExclusiveCpuTime(), outString1)
								<< ", incl cpu " << mParent->mUnits->labelledString(block->getInclusiveCpuTime(), outString2);
							if (mParent->mHaveRealTime) {
								String out_string;
								ss << ", excl real " << mParent->mUnits->labelledString(block->getExclusiveRealTime(), outString3)
									<< ", incl real " << mParent->mUnits->labelledString(block->getInclusiveRealTime(), outString4);
							}
						}
						else {
							ss << ", excl real " << mParent->mUnits->labelledString(block->getExclusiveRealTime(), outString1)
								<< ", incl real " << mParent->mUnits->labelledString(block->getInclusiveRealTime(), outString2);
						}
					}
					if ((mMouseSelect.x >= strip->mX) && (mMouseSelect.x < strip->mX + strip->mWidth)) {
						selectBlock = mParent->mCallList->get(strip->mSegment->mBlock);
					}
				}
			}
		}
		mMouseSelect.x = 0;
		mMouseSelect.y = 0;
		if (selectBlock != nullptr) {
			Vector<Selection*> selections;
			auto rd = mParent->mRows[mParent->mMouseRow];
			//selections.push_back(Selection::highlight("Thread", rd));
			//selections.push_back(Selection::highlight("Call", selectBlock));
			auto mouseX = mMouse.x - 10;
			auto mouseXval = mParent->getScaleInfo().pixelToValue(mouseX);
			//selections.push_back(Selection::highlight("Time", reinterpret_cast<Object*>(mouseXval)));
			//mParent->mSelectionController->change(selections, "TimeLineView");
			mParent->mHighlightMethodData = nullptr;
			mParent->mHighlightCall = selectBlock;
			mParent->startHighlighting();
		}
		if ((mParent->mMouseRow >= 0) && (mParent->mMouseRow < mParent->mNumRows) && (mHighlightStep == 0)) {
			auto y1 = mParent->mMouseRow * 32 - mParent->mScrollOffsetY;
		}
//		drawHighlights();
		auto lineEnd = std::min(dim.cy, mParent->mNumRows * 32);

		if (!blockName.empty()) {
			//mParent->mTimescale->setMethodName(blockName);
			//mParent->mTimescale->setMethodColor(blockColor);
			//mParent->mTimescale->setDetails(blockDetails);
			mShowHighlightName = false;
		}
		else if (mShowHighlightName) {
			auto md = mParent->mHighlightMethodData;
			if ((md == nullptr) && (mParent->mHighlightCall != nullptr))
				md = mParent->mHighlightCall->getMethodData();

			if (md == nullptr) {
				printf("null highlight\n");
			}

			if (md != nullptr) {
				//mParent->mTimescale->setMethodName(md->getProfileName());
				//mParent->mTimescale->setMethodColor(md->getColor());
				//mParent->mTimescale->setDetails(nullptr);
			}
		}
		else {
			//mParent->mTimescale->setMethodName(nullptr);
			//mParent->mTimescale->setMethodColor(0);
			//mParent->mTimescale->setDetails(nullptr);
		}
		//mParent->mTimescale->redraw();
		//gc->drawImage(image, 0, 0);
		//image->dispose();
		//gcImage->dispose();
	}

	void Surface::drawHighlights(Point dim)
	{
		auto height = mHighlightHeight;
		if (height <= 0)
			return;

		for (auto _i = mHighlightExclusive.begin(); _i != mHighlightExclusive.end(); _i++) {
			Range* range = &(*_i);
			int xStart = range->mXdim.x;
			int width = range->mXdim.y;
		}
		height--;
		if (height <= 0) {
			height = 1;
		}

		for (auto _i = mHighlightInclusive.begin(); _i != mHighlightInclusive.end(); _i++) {
			Range* range = &(*_i);
			auto x1 = range->mXdim.x;
			auto x2 = range->mXdim.y;
			auto drawLeftEnd = false;
			auto drawRightEnd = false;
			if (x1 >= 10) {
				drawLeftEnd = true;
			}
			else
				x1 = 10;
			if (x2 >= 10) {
				drawRightEnd = true;
			}
			else
				x2 = dim.x - 60;
			auto y1 = range->mY + 20 + 2 - mParent->mScrollOffsetY;
			if (x2 - x1 < 3) {
				auto width = x2 - x1;
				if (width < 2)
					width = 2;
			}
			else {
				if (drawLeftEnd) {
					if (drawRightEnd) {
					}
					else {
					}
				}
				else if (drawRightEnd) {
				}
				else {
				}
				if (!drawLeftEnd) {
				}
				if (!drawRightEnd) {
				}
			}
		}
	}

	bool Surface::drawingSelection()
	{
		return (mGraphicsState == Marking) || (mGraphicsState == Animating);
	}

	void Surface::computeStrips()
	{
		auto minVal = mParent->getScaleInfo().getMinVal();
		auto maxVal = mParent->getScaleInfo().getMaxVal();
		auto pixels = new Pixel[mParent->mNumRows];
		for (auto ii = 0; ii < mParent->mSegments.size(); ii++) {
			mParent->mCallList->get(mParent->mSegments[ii].mBlock)->clearWeight();
		}

		mStripList.clear();
		mHighlightExclusive.clear();
		mHighlightInclusive.clear();
		MethodData* callMethod = nullptr;
		uint32_t callStart = 0;
		uint32_t callEnd = UINT32_MAX;
		RowData* callRowData = nullptr;
		uint32_t prevMethodStart = UINT32_MAX;
		uint32_t prevMethodEnd = UINT32_MAX;
		uint32_t prevCallStart = UINT32_MAX;
		uint32_t prevCallEnd = UINT32_MAX;
		if (mParent->mHighlightCall != nullptr) {

			uint32_t callPixelStart = UINT32_MAX;
			uint32_t callPixelEnd = UINT32_MAX;

			callStart = mParent->mHighlightCall->getStartTime();
			callEnd = mParent->mHighlightCall->getEndTime();
			callMethod = mParent->mHighlightCall->getMethodData();

			if (callStart >= minVal) {
				callPixelStart = mParent->getScaleInfo().valueToPixel(callStart);
			}

			if (callEnd <= maxVal) {
				callPixelEnd = mParent->getScaleInfo().valueToPixel(callEnd);
			}

			uint32_t threadId = mParent->mHighlightCall->getThreadId();
			const String& threadName = mParent->mThreadLabels[threadId].c_str();
			callRowData = mParent->mRowByName[threadName];
			auto y1 = callRowData->mRank * 32 + 6;
			COLOR color = callMethod->getColor();
			mHighlightInclusive.push_back(Range(callPixelStart + 10, callPixelEnd + 10, y1, color));
		}

		for (auto ii = 0; ii < mParent->mSegments.size(); ii++) {
			Segment* segment = mParent->mSegments.get(ii);
			if (segment->mEndTime > minVal && segment->mStartTime < maxVal) {
				auto block = mParent->mCallList->get(segment->mBlock);
				COLOR color = block->getColor();
				if (color != 0) {
					auto recordStart = std::max(static_cast<double>(segment->mStartTime), minVal);
					auto recordEnd = std::min(static_cast<double>(segment->mEndTime), maxVal);

					if (recordStart != recordEnd) {
						auto pixelStart = mParent->getScaleInfo().valueToPixel(recordStart);
						auto pixelEnd = mParent->getScaleInfo().valueToPixel(recordEnd);
						auto width = pixelEnd - pixelStart;
						auto isContextSwitch = segment->mIsContextSwitch;
						auto rd = segment->mRowData;
						auto md = block->getMethodData();
						auto y1 = rd->mRank * 32 + 6;
						if (rd->mRank > mParent->mEndRow) {
							break;
						}
						if (mParent->mHighlightMethodData != nullptr) {
							if (mParent->mHighlightMethodData == md) {
								if ((prevMethodStart != pixelStart) || (prevMethodEnd != pixelEnd)) {
									prevMethodStart = pixelStart;
									prevMethodEnd = pixelEnd;
									auto rangeWidth = width;
									if (rangeWidth == 0)
										rangeWidth = 1;

									mHighlightExclusive.push_back(Range(pixelStart + 10, rangeWidth, y1, color));
									callStart = block->getStartTime();
									auto callPixelStart = -1;
									if (callStart >= minVal)
										callPixelStart = mParent->getScaleInfo().valueToPixel(callStart);

									auto callPixelEnd = -1;
									callEnd = block->getEndTime();
									if (callEnd <= maxVal)
										callPixelEnd = mParent->getScaleInfo().valueToPixel(callEnd);

									if ((prevCallStart != callPixelStart) || (prevCallEnd != callPixelEnd)) {
										prevCallStart = callPixelStart;
										prevCallEnd = callPixelEnd;
										mHighlightInclusive.push_back(Range(callPixelStart + 10, callPixelEnd + 10, y1, color));
									}
								}
							}
							else if (mFadeColors) {
								color = md->getFadedColor();
							}
						}
						else if (mParent->mHighlightCall != nullptr) {
							if ((segment->mStartTime >= callStart) && (segment->mEndTime <= callEnd) && (callMethod == md) && (callRowData == rd)) {
								if ((prevMethodStart != pixelStart) || (prevMethodEnd != pixelEnd)) {
									prevMethodStart = pixelStart;
									prevMethodEnd = pixelEnd;
									auto rangeWidth = width;
									if (rangeWidth == 0)
										rangeWidth = 1;

									mHighlightExclusive.push_back(Range(pixelStart + 10, rangeWidth, y1, color));
								}
							}
							else if (mFadeColors) {
								color = md->getFadedColor();
							}
						}
						auto pix = &pixels[rd->mRank];
						if (pix->mStart != pixelStart) {
							if (pix->mSegment != nullptr) {
								emitPixelStrip(rd, y1, pix);
							}
							if (width == 0) {
								auto weight = computeWeight(recordStart, recordEnd, isContextSwitch, pixelStart);
								weight = block->addWeight(pixelStart, rd->mRank, weight);
								if (weight > pix->mMaxWeight) {
									pix->setFields(pixelStart, weight, segment, color, rd);
								}
							}
							else {
								auto x1 = pixelStart + 10;
								auto strip = new Strip(x1, isContextSwitch ? y1 + 20 - 1 : y1, width, isContextSwitch ? 1 : 20, rd, segment, color);
								mStripList.push_back(strip);
							}
						}
						else {
							auto weight = computeWeight(recordStart, recordEnd, isContextSwitch, pixelStart);
							weight = block->addWeight(pixelStart, rd->mRank, weight);
							if (weight > pix->mMaxWeight) {
								pix->setFields(pixelStart, weight, segment, color, rd);
							}
							if (width == 1) {
								emitPixelStrip(rd, y1, pix);
								pixelStart++;
								weight = computeWeight(recordStart, recordEnd, isContextSwitch, pixelStart);
								weight = block->addWeight(pixelStart, rd->mRank, weight);
								pix->setFields(pixelStart, weight, segment, color, rd);
							}
							else if (width > 1) {
								emitPixelStrip(rd, y1, pix);
								pixelStart++;
								width--;
								auto x1 = pixelStart + 10;
								auto strip = new Strip(x1, isContextSwitch ? y1 + 20 - 1 : y1, width, isContextSwitch ? 1 : 20, rd, segment, color);
								mStripList.push_back(strip);
							}
						}
					}
				}
			}
		}
		for (auto ii = 0; ii < mParent->mNumRows; ii++) {
			auto pix = &pixels[ii];
			if (pix->mSegment != nullptr) {
				auto rd = pix->mRowData;
				auto y1 = rd->mRank * 32 + 6;
				emitPixelStrip(rd, y1, pix);
			}
		}
	}

	double Surface::computeWeight(double start, double end, bool isContextSwitch, int pixel)
	{
		if (isContextSwitch) {
			return 0.0;
		}
		auto pixelStartFraction = mParent->getScaleInfo().valueToPixelFraction(start);
		auto pixelEndFraction = mParent->getScaleInfo().valueToPixelFraction(end);
		auto leftEndPoint = std::max(pixelStartFraction, pixel - 0.5);
		auto rightEndPoint = std::min(pixelEndFraction, pixel + 0.5);
		auto weight = rightEndPoint - leftEndPoint;
		return weight;
	}

	void Surface::emitPixelStrip(RowData* rd, int y, Pixel* pixel)
	{
		if (pixel->mSegment == nullptr) {
			return;
		}
		auto x = pixel->mStart + 10;
		auto height = static_cast<int>((pixel->mMaxWeight * 20.0 * 0.75));
		if (height < mMinStripHeight)
			height = mMinStripHeight;

		auto remainder = 20 - height;
		if (remainder > 0) {
			auto strip = new Strip(x, y, 1, remainder, rd, pixel->mSegment, mFadeColors ? 0 : 1); // TODO
			mStripList.push_back(strip);
		}
		auto strip = new Strip(x, y + remainder, 1, height, rd, pixel->mSegment, pixel->mColor);
		mStripList.push_back(strip);
		pixel->mSegment = nullptr;
		pixel->mMaxWeight = 0.0;
	}

	void Surface::mouseMove(Point& pt, int flags)
	{
		auto dim = mParent->mSurface->getSize();
		auto x = pt.x;
		if (x < 10)
			x = 10;

		if (x > dim.cx - 60)
			x = dim.cx - 60;

		mMouse.x = x;
		mMouse.y = pt.y;
		//mParent->mTimescale->setVbarPosition(x);
		if (mGraphicsState == Marking) {
			//mParent->mTimescale->setMarkEnd(x);
		}
		if (mGraphicsState == Normal) {
			//mParent->mSurface->setCursor(mNormalCursor);
		}
		else if (mGraphicsState == Marking) {
			if (mMouse.x >= mMouseMarkStartX) {
				//mParent->mSurface->setCursor(mIncreasingCursor);
			}
			else {
				//mParent->mSurface->setCursor(mDecreasingCursor);
			}
		}
		auto rownum = (mMouse.y + mParent->mScrollOffsetY) / 32;
		if ((pt.y < 0) || (pt.y >= dim.cy)) {
			rownum = -1;
		}
		if (mParent->mMouseRow != rownum) {
			mParent->mMouseRow = rownum;
			//mParent->mLabels->redraw();
		}
		//redraw();
	}

	void Surface::mouseDown(Point& pt, int flags)
	{
		auto dim = mParent->mSurface->getSize();
		auto x = pt.x;
		if (x < 10)
			x = 10;

		if (x > dim.cx - 60)
			x = dim.cx - 60;

		mMouseMarkStartX = x;
		mGraphicsState = Marking;
		//mParent->mTimescale->setMarkStart(mMouseMarkStartX);
		//mParent->mTimescale->setMarkEnd(mMouseMarkStartX);
		redraw();
	}

	void Surface::mouseUp(Point& pt, int flags)
	{
		if (mGraphicsState != Marking) {
			mGraphicsState = Normal;
			return;
		}
		mGraphicsState = Animating;
		auto dim = mParent->mSurface->getSize();
		if ((pt.y <= 0) || (pt.y >= dim.cy)) {
			mGraphicsState = Normal;
			redraw();
			return;
		}
		auto x = pt.x;
		if (x < 10)
			x = 10;

		if (x > dim.cx - 60)
			x = dim.cx - 60;

		mMouseMarkEndX = x;
		auto dist = mMouseMarkEndX - mMouseMarkStartX;
		if (dist < 0)
			dist = -dist;

		if (dist <= 2) {
			mGraphicsState = Normal;
			mMouseSelect.x = mMouseMarkStartX;
			mMouseSelect.y = pt.y;
			redraw();
			return;
		}
		if (mMouseMarkEndX < mMouseMarkStartX) {
			auto temp = mMouseMarkEndX;
			mMouseMarkEndX = mMouseMarkStartX;
			mMouseMarkStartX = temp;
		}
		if ((mMouseMarkStartX <= 20) && (mMouseMarkEndX >= dim.cx - 60 - 10)) {
			mGraphicsState = Normal;
			redraw();
			return;
		}
		auto minVal = mParent->getScaleInfo().getMinVal();
		auto maxVal = mParent->getScaleInfo().getMaxVal();
		auto ppr = mParent->getScaleInfo().getPixelsPerRange();
		mZoomMin = (minVal + (mMouseMarkStartX - 10) / ppr);
		mZoomMax = (minVal + (mMouseMarkEndX - 10) / ppr);
		if (mZoomMin < mMinDataVal)
			mZoomMin = mMinDataVal;

		if (mZoomMax > mMaxDataVal) {
			mZoomMax = mMaxDataVal;
		}
		auto xdim = dim.cx - 70;
		auto scaler = new TickScaler(mZoomMin, mZoomMax, xdim, 50);
		scaler->computeTicks(false);
		mZoomMin = scaler->getMinVal();
		mZoomMax = scaler->getMaxVal();
		mMouseMarkStartX = static_cast<int>(((mZoomMin - minVal) * ppr + 10.0));
		mMouseMarkEndX = static_cast<int>(((mZoomMax - minVal) * ppr + 10.0));
		//mParent->mTimescale->setMarkStart(mMouseMarkStartX);
		//mParent->mTimescale->setMarkEnd(mMouseMarkEndX);
		mMouseEndDistance = (dim.cx - 60 - mMouseMarkEndX);
		mMouseStartDistance = (mMouseMarkStartX - 10);
		mZoomMouseStart = mMouseMarkStartX;
		mZoomMouseEnd = mMouseMarkEndX;
		mZoomStep = 0;
		mMin2ZoomMin = (mZoomMin - minVal);
		mZoomMax2Max = (maxVal - mZoomMax);
		mZoomFixed = (mZoomMin + (mZoomMax - mZoomMin) * mMin2ZoomMin / (mMin2ZoomMin + mZoomMax2Max));
		mZoomFixedPixel = ((mZoomFixed - minVal) * ppr + 10.0);
		mFixedPixelStartDistance = (mZoomFixedPixel - 10.0);
		mFixedPixelEndDistance = (dim.cx - 60 - mZoomFixedPixel);
		mZoomMin2Fixed = (mZoomFixed - mZoomMin);
		mFixed2ZoomMax = (mZoomMax - mZoomFixed);
		redraw();
	}

	void Surface::mouseScrolled(Point& pt, int flags)
	{
		mGraphicsState = Scrolling;
		auto tMin = mParent->getScaleInfo().getMinVal();
		auto tMax = mParent->getScaleInfo().getMaxVal();
		auto zoomFactor = 2.0;
		auto tMinRef = mLimitMinVal;
		auto tMaxRef = mLimitMaxVal;
		double tMaxNew;
		double tMinNew;
		double t;
		if (pt.x > 0) {
			auto dim = mParent->mSurface->getSize();
			auto x = pt.x;
			if (x < 10)
				x = 10;

			if (x > dim.cx - 60)
				x = dim.cx - 60;

			auto ppr = mParent->getScaleInfo().getPixelsPerRange();
			t = tMin + (x - 10) / ppr;
			tMinNew = std::max(tMinRef, t - (t - tMin) / zoomFactor);
			tMaxNew = std::min(tMaxRef, t + (tMax - t) / zoomFactor);
		}
		else {
			auto factor = (tMax - tMin) / (tMaxRef - tMinRef);
			if (factor < 1.0) {
				t = (factor * tMinRef - tMin) / (factor - 1.0);
				tMinNew = std::max(tMinRef, t - zoomFactor * (t - tMin));
				tMaxNew = std::min(tMaxRef, t + zoomFactor * (tMax - t));
			}
			else {
				return;
			}
		}
		mParent->getScaleInfo().setMinVal(tMinNew);
		mParent->getScaleInfo().setMaxVal(tMaxNew);
		mParent->mSurface->redraw();
	}

	void Surface::mouseDoubleClick(Point& pt, int flags)
	{
	}

	void Surface::startScaling(int mouseX)
	{
		auto dim = mParent->mSurface->getSize();
		auto x = mouseX;
		if (x < 10)
			x = 10;

		if (x > dim.cx - 60)
			x = dim.cx - 60;

		mMouseMarkStartX = x;
		mGraphicsState = Scaling;
		mScalePixelsPerRange = mParent->getScaleInfo().getPixelsPerRange();
		mScaleMinVal = mParent->getScaleInfo().getMinVal();
		mScaleMaxVal = mParent->getScaleInfo().getMaxVal();
	}

	void Surface::stopScaling(int mouseX)
	{
		mGraphicsState = Normal;
	}

	void Surface::animateHighlight()
	{
		mHighlightStep += 1;
		if (mHighlightStep >= HIGHLIGHT_STEPS) {
			mFadeColors = false;
			mHighlightStep = 0;
			mCachedEndRow = -1;
		}
		else {
			mFadeColors = true;
			mShowHighlightName = true;
			mHighlightHeight = highlightHeights[mHighlightStep];
		}
		redraw();
	}

	void Surface::clearHighlights()
	{
		mShowHighlightName = false;
		mHighlightHeight = 0;
		mParent->mHighlightMethodData = nullptr;
		mParent->mHighlightCall = nullptr;
		mFadeColors = false;
		mHighlightStep = 0;
		mCachedEndRow = -1;
		redraw();
	}

	void Surface::animateZoom()
	{
		mZoomStep += 1;
		if (mZoomStep > 8) {
			mGraphicsState = Normal;
			mCachedMinVal = (mParent->getScaleInfo().getMinVal() + 1.0);
		}
		else if (mZoomStep == 8) {
			mParent->getScaleInfo().setMinVal(mZoomMin);
			mParent->getScaleInfo().setMaxVal(mZoomMax);
			mMouseMarkStartX = 10;
			auto dim = getSize();
			mMouseMarkEndX = (dim.cx - 60);
			//mParent->mTimescale->setMarkStart(mMouseMarkStartX);
			//mParent->mTimescale->setMarkEnd(mMouseMarkEndX);
		}
		else {
			auto fraction = mZoomFractions[mZoomStep];
			mMouseMarkStartX = static_cast<int>((mZoomMouseStart - fraction * mMouseStartDistance));
			mMouseMarkEndX = static_cast<int>((mZoomMouseEnd + fraction * mMouseEndDistance));
			//mParent->mTimescale->setMarkStart(mMouseMarkStartX);
			//mParent->mTimescale->setMarkEnd(mMouseMarkEndX);
			double ppr;
			if (mZoomMin2Fixed >= mFixed2ZoomMax) {
				ppr = (mZoomFixedPixel - mMouseMarkStartX) / mZoomMin2Fixed;
			}
			else
				ppr = (mMouseMarkEndX - mZoomFixedPixel) / mFixed2ZoomMax;
			auto newMin = mZoomFixed - mFixedPixelStartDistance / ppr;
			auto newMax = mZoomFixed + mFixedPixelEndDistance / ppr;
			mParent->getScaleInfo().setMinVal(newMin);
			mParent->getScaleInfo().setMaxVal(newMax);
		}
		redraw();
	}

	Timescale::Timescale(TimeLineView *parent)
		: mParent(parent)
	{
	}

	RowLabels::RowLabels(TimeLineView *parent)
		: mParent(parent)
	{
	}

	void RowLabels::mouseMove(Point& pt, int flags)
	{
		auto rownum = (pt.y + mParent->mScrollOffsetY) / 32;
		if (mParent->mMouseRow != rownum) {
			mParent->mMouseRow = rownum;
			redraw();
			mParent->mSurface->redraw();
		}
	}

	TimeLineView::TimeLineView(DmTraceReader* reader/*, SelectionController* selectionController*/)
		: mScaleInfo(0.0, 0.0, 0, 50)
		, mMouseRow(-1)
		, mSetFonts(false)
	{
		mCallList = nullptr;
		//mUnits = reader->getTraceUnits();
		mClockSource = reader->getClockSource();
		mHaveCpuTime = reader->haveCpuTime();
		mHaveRealTime = reader->haveRealTime();
//		mThreadLabels = reader->getThreadLabels();

		mTimescale = new Timescale(this);
		mSurface = new Surface(this);
		mLabels = new RowLabels(this);

		setData(reader->getThreadTimeRecords());
	}

	void TimeLineView::setData(CallList* callList)
	{
		if (mCallList) {
			throw GeneralException("setData already called");
		}

		mCallList = callList;
		double maxVal = 0.0;
		double minVal = 0.0;
		if (callList->size() > 0) {
			minVal = callList->get(0)->getStartTime();
		}

		for (size_t _i = 0; _i < callList->size(); _i++) {
			Call* block = callList->get(_i);
			ThreadData* row = block->getThreadData();
			if (!block->isIgnoredBlock(callList)) {
				auto rowName = row->getName();

				RowData* rd;
				HashMap<String, RowData*>::iterator it = mRowByName.find(rowName);
				if (it == mRowByName.end()) {
					rd = new RowData(row);
					mRowByName.add(rowName, rd);
				}
				else {
					rd = it->second;
				}

				auto blockStartTime = block->getStartTime();
				auto blockEndTime = block->getEndTime();
				if (blockEndTime > rd->mEndTime) {
					auto start = std::max(blockStartTime, rd->mEndTime);
					rd->mElapsed = blockEndTime - start;
					rd->mEndTime = blockEndTime;
				}
				if (blockEndTime > maxVal) {
					maxVal = blockEndTime;
				}
				auto top = rd->top();
				if (top == -1) {
					rd->push(block->getIndex());
				}
				else {
					Call* topCall = mCallList->get(top);
					auto topStartTime = topCall->getStartTime();
					auto topEndTime = topCall->getEndTime();
					if (topEndTime >= blockStartTime) {
						if (topStartTime < blockStartTime) {
							auto segment = mSegments.addNull2();
							segment->init(rd, callList, topCall->getIndex(), topStartTime, blockStartTime);
						}
						if (topEndTime == blockStartTime)
							rd->pop();

						rd->push(block->getIndex());
					}
					else {
						popFrames(rd, callList, topCall, blockStartTime, &mSegments);
						rd->push(block->getIndex());
					}
				}
			}
		}

		for (auto _i = mRowByName.begin(); _i != mRowByName.end(); _i++) {
			RowData* rd = _i->second;
			{
				auto top = rd->top();
				popFrames(rd, callList, callList->get(top), INT_MAX, &mSegments);
			}
		}

		mSurface->setRange(minVal, maxVal);
		mSurface->setLimitRange(minVal, maxVal);
		mRows = mRowByName.value_vector();
		std::sort(mRows->begin(), mRows->begin(), RowData::Less());

		int index = 0;
		for (auto ii = mRows->begin(); ii < mRows->end(); ii++) {
			(*ii)->mRank = index++;
		}

		mNumRows = 0;
		for (auto ii = mRows->begin(); ii < mRows->end(); ii++) {
			if ((*ii)->mElapsed == 0LL)
				break;

			mNumRows += 1;
		}
		//::qsort(mSegments, new TimeLineView_setData_12(this));
		dumpSegments();
	}

	void TimeLineView::popFrames(RowData* rd, CallList* callList, Call* top, uint32_t startTime, SegmentList* segmentList)
	{
		auto topEndTime = top->getEndTime();
		auto lastEndTime = top->getStartTime();
		while (topEndTime <= startTime) {
			if (topEndTime > lastEndTime) {
				auto segment = segmentList->addNull2();
				segment->init(rd, callList, top->getIndex(), lastEndTime, topEndTime);
				lastEndTime = topEndTime;
			}
			rd->pop();

			int topIndex = rd->top();
			if (topIndex == -1)
				return;
			top = callList->get(topIndex);

			topEndTime = top->getEndTime();
		}
		if (lastEndTime < startTime) {
			auto bd = segmentList->addNull2();
			bd->init(rd, callList, top->getIndex(), lastEndTime, startTime);
		}
	}

	int TimeLineView::computeVisibleRows(int ydim)
	{
		auto offsetY = mScrollOffsetY;
		auto spaceNeeded = mNumRows * 32;
		if (offsetY + ydim > spaceNeeded) {
			offsetY = spaceNeeded - ydim;
			if (offsetY < 0) {
				offsetY = 0;
			}
		}
		mStartRow = (offsetY / 32);
		mEndRow = ((offsetY + ydim) / 32);
		if (mEndRow >= mNumRows) {
			mEndRow = (mNumRows - 1);
		}
		return offsetY;
	}

	void TimeLineView::startHighlighting()
	{
		mSurface->mHighlightStep = 0;
		mSurface->mFadeColors = true;
		mSurface->mCachedEndRow = -1;
	}
	
	void TimeLineView::dumpSegments()
	{
		printf("\nSegments\n");
		printf("id\tt-start\tt-end\tg-start\tg-end\texcl.\tincl.\tmethod\n");
		for (auto _i = 0; _i < mSegments.size(); _i++) {
			Segment* segment = mSegments.get(_i);
			Call* call = mCallList->get(segment->mBlock);
			printf("%2d\t%s\t%4d\t%4d\t%s\n"
				   , call->getThreadId()
				   , segment->mRowData->mName.c_str()
				   , segment->mStartTime
				   , segment->mEndTime
				   , call->getMethodData()->getName()
				   );
		}
	}

};
