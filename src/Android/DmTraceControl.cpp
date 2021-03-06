#include "DmTraceControl.hpp"
#include "TraceUnits.hpp"
#include <assert.h>
#include <iostream>

#ifdef _MSC_VER
#include <Windows.h>
#undef min
#undef max
#else
#include <mach/mach_time.h>
#endif

namespace Android {
    const double Pixel::qualifiedWeight = 0.5;

    Range::Range(int xStart, int width, int y, COLOR color)
    {
        mXdim.x = xStart;
        mXdim.y = width;
        mY = y;
        mColor = color;
    }

    void Pixel::setFields(int start, double weight, Call* call, COLOR color, ThreadData* thread)
    {
        mStart = start;
        mMaxWeight = weight;
        mCall = call;
        mColor = color;
        mThread = thread;
    }

    /* Compute the scale ticks for following parameters:
    1. mMinVal, mMaxVal ==> The visible range of min/max timeline in us(If useGivenEndPoints is true, they are aligned with mTickIncrement)
    mRangeVal          = mMaxVal - mMinVal

    2. mNumPixels       ==> How many pixels is visible width range

    3. mTickIncrement   ==> Scaling unit (1, 2, 5 * 10^N) in us

    4. mMinMajorTick    ==> Ceil of mMinVal aligned with mTickIncrement
    */
    void TickScaler::computeTicks(bool useGivenEndPoints)
    {
        int numTicks = mNumPixels / mPixelsPerTick;
        mRangeVal = (mMaxVal - mMinVal);
        mTickIncrement = int(mRangeVal / numTicks);

        uint32_t nTickIncr;
        for (nTickIncr = 100; nTickIncr < mTickIncrement; nTickIncr *= 10) {
            if (nTickIncr * 2 > mTickIncrement) {
                nTickIncr = nTickIncr * 2;
                break;
            }
            if (nTickIncr * 5 > mTickIncrement) {
                nTickIncr = nTickIncr * 5;
                break;
            }
        }
        mTickIncrement = nTickIncr;

        if (!useGivenEndPoints) {
            //Align mMinVal and mMaxVal to minor Tick Increment scale
            uint32_t minorTickIncrement = mTickIncrement / 5;
            uint32_t imax = mMaxVal / minorTickIncrement * minorTickIncrement;
            if (imax < mMaxVal) {
                mMaxVal = imax;
            }
            mMinVal = mMinVal / mTickIncrement * mTickIncrement;
            mMinMajorTick = mMinVal;
        }
        else {
            // Only mMinMajorTick should be aligned
            mMinMajorTick = mMinVal / mTickIncrement * mTickIncrement;
            if (mMinMajorTick < mMinVal) {
                mMinMajorTick += mTickIncrement;
            }
        }
        mRangeVal = (mMaxVal - mMinVal);
    }

    int TimeLineView::highlightHeights[] = { 0, 2, 4, 5, 6, 5, 4, 2, 4, 5, 6 };

    TimeLineView::TimeLineView(DmTraceControl *parent)
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

    void TimeLineView::initZoomFractionsWithExp()
    {
        int next = 0;
        for (auto ii = 0; ii < 4; next++) {
            mZoomFractions[next] = ((1 << ii) / 16.0);
            ii++;
        }
        for (auto ii = 2; ii < 6; next++) {
            mZoomFractions[next] = (((1 << ii) - 1) / (1 << ii));
            ii++;
        }
    }

    void TimeLineView::initZoomFractionsWithSinWave()
    {
        for (auto ii = 0; ii < 8; ii++) {
            double offset = 3.141592653589793 * ii / 8.0;
            mZoomFractions[ii] = ((sin(4.71238898038469 + offset) + 1.0) / 2.0);
        }
    }

    void TimeLineView::setRange(uint32_t minVal, uint32_t maxVal)
    {
        mMinDataVal = minVal;
        mMaxDataVal = maxVal;
        mParent->getScaleInfo().setMinVal(minVal);
        mParent->getScaleInfo().setMaxVal(maxVal);
    }

    void TimeLineView::setLimitRange(uint32_t minVal, uint32_t maxVal)
    {
        mLimitMinVal = minVal;
        mLimitMaxVal = maxVal;
    }

    void TimeLineView::resetScale()
    {
        mParent->getScaleInfo().setMinVal(mLimitMinVal);
        mParent->getScaleInfo().setMaxVal(mLimitMaxVal);
    }

    void TimeLineView::setScaleFromHorizontalScrollBar(int selection)
    {
        uint32_t minVal = mParent->getScaleInfo().getMinVal();
        uint32_t maxVal = mParent->getScaleInfo().getMaxVal();
        uint32_t visibleRange = maxVal - minVal;
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
        void TimeLineView::updateHorizontalScrollBar()
        {
            double minVal = mParent->getScaleInfo().getMinVal();
            double maxVal = mParent->getScaleInfo().getMaxVal();
            double visibleRange = maxVal - minVal;
            double fullRange = mLimitMaxVal - mLimitMinVal;
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
        }
     */

    void TimeLineView::draw(void* context)
    {
        if (mGraphicsState == Scaling) {
            int diff = mMouse.x - mMouseMarkStartX;
            if (diff > 0) {
                uint32_t newMinVal = mScaleMinVal - diff / mScalePixelsPerRange;
                if (newMinVal < mLimitMinVal)
                    newMinVal = mLimitMinVal;

                mParent->getScaleInfo().setMinVal(newMinVal);
            }
            else if (diff < 0) {
                uint32_t newMaxVal = mScaleMaxVal - diff / mScalePixelsPerRange;
                if (newMaxVal > mLimitMaxVal)
                    newMaxVal = mLimitMaxVal;

                mParent->getScaleInfo().setMaxVal(newMaxVal);
            }
        }

        Size dim = getSize();

        if ((mParent->mStartRow != mCachedStartRow) || (mParent->mEndRow != mCachedEndRow) || (mParent->getScaleInfo().getMinVal() != mCachedMinVal) || (mParent->getScaleInfo().getMaxVal() != mCachedMaxVal)) {
            mCachedStartRow = mParent->mStartRow;
            mCachedEndRow = mParent->mEndRow;
            int xdim = dim.cx - 70;
            mParent->getScaleInfo().setNumPixels(xdim);
            bool forceEndPoints = (mGraphicsState == Scaling) || (mGraphicsState == Animating) || (mGraphicsState == Scrolling);
            mParent->getScaleInfo().computeTicks(forceEndPoints);
            mCachedMinVal = mParent->getScaleInfo().getMinVal();
            mCachedMaxVal = mParent->getScaleInfo().getMaxVal();
            if (mLimitMinVal > mParent->getScaleInfo().getMinVal())
                mLimitMinVal = mParent->getScaleInfo().getMinVal();

            if (mLimitMaxVal < mParent->getScaleInfo().getMaxVal()) {
                mLimitMaxVal = mParent->getScaleInfo().getMaxVal();
            }
            //computeStrips();
        }

        if (mParent->mNumRows > 2) {
            //			gcImage->setBackground(mParent->mColorRowBack);
            for (auto ii = 1; ii < mParent->mNumRows; ii += 2) {
                //ThreadData* rd = mParent->mRows)[ii];
                //int y1 = rd->mRank * 32 - mParent->mScrollOffsetY;
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
            printf("dim.x %d pixels %d minVal %d, maxVal %d pixels %d tickIncr %d\n",
                dim.cx, dim.cx - 70,
                mParent->getScaleInfo().getMinVal(), mParent->getScaleInfo().getMaxVal(),
                mParent->getScaleInfo().getNumPixels(), mParent->getScaleInfo().getTickIncrement());
        }
        Call* selectBlock = nullptr;
#if 0
        for (auto it = 0; it < mStripList.size(); it++) {
            Strip* strip = mStripList.get(it);
            if (strip->mColor != 0) {

                if (mParent->mMouseRow == strip->mCall->getThreadData()->mRank) {
                    if ((mMouse.x >= strip->mX) && (mMouse.x < strip->mX + strip->mWidth)) {
                        Call* block = strip->mCall;
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
                                String outString;
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
                        selectBlock = strip->mCall;
                    }
                }
            }
        }
#endif
        mMouseSelect.x = 0;
        mMouseSelect.y = 0;
        if (selectBlock != nullptr) {
            //			Vector<Selection*> selections;
            //			Vector<RowData*>& rd = mParent->mRows[mParent->mMouseRow];
            //			selections.push_back(Selection::highlight("Thread", rd));
            //			selections.push_back(Selection::highlight("Call", selectBlock));
            auto mouseX = mMouse.x - 10;
            auto mouseXval = mParent->getScaleInfo().pixelToValue(mouseX);
            //selections.push_back(Selection::highlight("Time", reinterpret_cast<Object*>(mouseXval)));
            //mParent->mSelectionController->change(selections, "DmTraceControl");
            mParent->mHighlightMethodData = nullptr;
            mParent->mHighlightCall = selectBlock;
            mParent->startHighlighting();
        }
        if ((mParent->mMouseRow >= 0) && (mParent->mMouseRow < mParent->mNumRows) && (mHighlightStep == 0)) {
            int y1 = mParent->mMouseRow * 32 - mParent->mScrollOffsetY;
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

    void TimeLineView::drawHighlights(Point dim)
    {
        auto height = mHighlightHeight;
        if (height <= 0)
            return;
/*
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
*/
    }

    bool TimeLineView::drawingSelection()
    {
        return (mGraphicsState == Marking) || (mGraphicsState == Animating);
    }

    void TimeLineView::mouseMove(Point& pt, int flags)
    {
        Size dim = getSize();
        int x = pt.x;
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
            //mParent->mTimeLine->setCursor(mNormalCursor);
        }
        else if (mGraphicsState == Marking) {
            if (mMouse.x >= mMouseMarkStartX) {
                //mParent->mTimeLine->setCursor(mIncreasingCursor);
            }
            else {
                //mParent->mTimeLine->setCursor(mDecreasingCursor);
            }
        }
        int rownum = (mMouse.y + mParent->mScrollOffsetY) / 32;
        if ((pt.y < 0) || (pt.y >= dim.cy)) {
            rownum = -1;
        }
        if (mParent->mMouseRow != rownum) {
            mParent->mMouseRow = rownum;
            //mParent->mThreadLabel->redraw();
        }
        //redraw();
    }

    void TimeLineView::mouseDown(Point& pt, int flags)
    {
        Size dim = getSize();
        int x = pt.x;
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

    void TimeLineView::mouseUp(Point& pt, int flags)
    {
        if (mGraphicsState != Marking) {
            mGraphicsState = Normal;
            return;
        }
        mGraphicsState = Animating;
        Size dim = getSize();
        if ((pt.y <= 0) || (pt.y >= dim.cy)) {
            mGraphicsState = Normal;
            redraw();
            return;
        }
        int x = pt.x;
        if (x < 10)
            x = 10;

        if (x > dim.cx - 60)
            x = dim.cx - 60;

        mMouseMarkEndX = x;
        int dist = mMouseMarkEndX - mMouseMarkStartX;
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
            int temp = mMouseMarkEndX;
            mMouseMarkEndX = mMouseMarkStartX;
            mMouseMarkStartX = temp;
        }
        if ((mMouseMarkStartX <= 20) && (mMouseMarkEndX >= dim.cx - 60 - 10)) {
            mGraphicsState = Normal;
            redraw();
            return;
        }
        uint32_t minVal = mParent->getScaleInfo().getMinVal();
        uint32_t maxVal = mParent->getScaleInfo().getMaxVal();
        uint32_t ppr = 1;
        mZoomMin = (minVal + (mMouseMarkStartX - 10) / ppr);
        mZoomMax = (minVal + (mMouseMarkEndX - 10) / ppr);
        if (mZoomMin < mMinDataVal)
            mZoomMin = mMinDataVal;

        if (mZoomMax > mMaxDataVal) {
            mZoomMax = mMaxDataVal;
        }
        int xdim = dim.cx - 70;
        TickScaler scaler(mZoomMin, mZoomMax, xdim, 50);
        scaler.computeTicks(false);
        mZoomMin = scaler.getMinVal();
        mZoomMax = scaler.getMaxVal();
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

    void TimeLineView::mouseScrolled(Point& pt, int flags)
    {
        mGraphicsState = Scrolling;
        double tMin = mParent->getScaleInfo().getMinVal();
        double tMax = mParent->getScaleInfo().getMaxVal();
        double zoomFactor = 2.0;
        double tMinRef = mLimitMinVal;
        double tMaxRef = mLimitMaxVal;
        double tMaxNew;
        double tMinNew;
        double t;
        if (pt.x > 0) {
            Size dim = getSize();
            int x = pt.x;
            if (x < 10)
                x = 10;

            if (x > dim.cx - 60)
                x = dim.cx - 60;

            double ppr = 1.0;// mParent->getScaleInfo().getPixelsPerRange();
            t = tMin + (x - 10) / ppr;
            tMinNew = std::max(tMinRef, t - (t - tMin) / zoomFactor);
            tMaxNew = std::min(tMaxRef, t + (tMax - t) / zoomFactor);
        }
        else {
            double factor = (tMax - tMin) / (tMaxRef - tMinRef);
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
        redraw();
    }

    void TimeLineView::mouseDoubleClick(Point& pt, int flags)
    {
    }

    void TimeLineView::startScaling(int mouseX)
    {
        Size dim = getSize();
        int x = mouseX;
        if (x < 10)
            x = 10;

        if (x > dim.cx - 60)
            x = dim.cx - 60;

        mMouseMarkStartX = x;
        mGraphicsState = Scaling;
        mScalePixelsPerRange = 1.0;// mParent->getScaleInfo().getPixelsPerRange();
        mScaleMinVal = mParent->getScaleInfo().getMinVal();
        mScaleMaxVal = mParent->getScaleInfo().getMaxVal();
    }

    void TimeLineView::stopScaling(int mouseX)
    {
        mGraphicsState = Normal;
    }

    void TimeLineView::animateHighlight()
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

    void TimeLineView::clearHighlights()
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

    void TimeLineView::animateZoom()
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
            Size dim = getSize();
            mMouseMarkEndX = (dim.cx - 60);
            //mParent->mTimescale->setMarkStart(mMouseMarkStartX);
            //mParent->mTimescale->setMarkEnd(mMouseMarkEndX);
        }
        else {
            double fraction = mZoomFractions[mZoomStep];
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
            double newMin = mZoomFixed - mFixedPixelStartDistance / ppr;
            double newMax = mZoomFixed + mFixedPixelEndDistance / ppr;
            mParent->getScaleInfo().setMinVal(newMin);
            mParent->getScaleInfo().setMaxVal(newMax);
        }
        redraw();
    }

    TimeScaleView::TimeScaleView(DmTraceControl *parent)
        : mParent(parent)
    {
    }

    ThreadLabelView::ThreadLabelView(DmTraceControl *parent)
        : mParent(parent)
    {
        mBoundRect.size.cx = 250;
    }

    void ThreadLabelView::mouseMove(Point& pt, int flags)
    {
        int rownum = (pt.y + mParent->mScrollOffsetY) / 32;
        if (mParent->mMouseRow != rownum) {
            mParent->mMouseRow = rownum;
            redraw();
            redraw();
        }
    }

    DmTraceControl::DmTraceControl()
        : mScaleInfo(0, 0, 0, 50)
        , mMouseRow(-1)
        , mStartRow(0)
        , mEndRow(0)
        , mNumRows(0)
        , mScrollOffsetY(0)
        , mTimescale(nullptr)
        , mTimeLine(nullptr)
        , mThreadLabel(nullptr)
    {
        mDmTraceData = nullptr;
        mHighlightMethodData = nullptr;
        mHighlightCall = nullptr;
    }

    void DmTraceControl::setData(DmTraceModel* traceData)
    {
        mDmTraceData = traceData;
        mHighlightMethodData = nullptr;
        mHighlightCall = nullptr;
        //mUnits = reader->getTraceUnits();
        mClockSource = traceData->getClockSource();
        mHaveCpuTime = traceData->haveCpuTime();
        mHaveRealTime = traceData->haveRealTime();
        mNumRows = (int)traceData->getThreads()->size();

        if (mDmTraceData->isRegression()) {
            uint32_t maxVal = traceData->getMaxTime();
            uint32_t minVal = traceData->getMinTime();
            mScaleInfo.setMaxVal(maxVal);
            mScaleInfo.setMinVal(minVal);
            mScaleInfo.setNumPixels(1000);
            mScaleInfo.computeTicks(false);
            computeVisibleRows(10000);

            mTimeLine->setRange(minVal, maxVal);
            mTimeLine->setLimitRange(minVal, maxVal);

#ifdef _MSC_VER
            DWORD s = GetTickCount();
#else
            uint64_t s = mach_absolute_time();
#endif
            std::cerr << "Begin to compute strips" << std::endl;

            computeStrips();

#ifdef _MSC_VER
            DWORD t = GetTickCount() - s;
#else
            uint64_t t = mach_absolute_time() - s;
#endif
            std::cerr << "It took " << t << " ticks to compute strips" << std::endl;
        }
    }

    int DmTraceControl::computeVisibleRows(int ydim)
    {
        int offsetY = mScrollOffsetY;
        int spaceNeeded = mNumRows * rowYSpace;
        if (offsetY + ydim > spaceNeeded) {
            offsetY = spaceNeeded - ydim;
            if (offsetY < 0) {
                offsetY = 0;
            }
        }
        mStartRow = (offsetY / rowYSpace);
        mEndRow = ((offsetY + ydim) / rowYSpace);
        if (mEndRow >= mNumRows) {
            mEndRow = (mNumRows - 1);
        }
        return offsetY;
    }

    void DmTraceControl::startHighlighting()
    {
        mTimeLine->mHighlightStep = 0;
        mTimeLine->mFadeColors = true;
        mTimeLine->mCachedEndRow = -1;
    }

    void DmTraceControl::computeStrips()
    {
        uint32_t minVal = mScaleInfo.getMinVal();
        uint32_t maxVal = mScaleInfo.getMaxVal();

        mHighlightExclusive.clear();
        mHighlightInclusive.clear();

        MethodData* callMethod = nullptr;
        ThreadData* callRowData = nullptr;

        uint32_t callStart = 0;
        uint32_t callEnd = UINT32_MAX;

        // if a Call in timeline view is selected, it will be highlighted
        if (mHighlightCall != nullptr) {

            int callPixelStart = -1;
            int callPixelEnd = -1;
            Call* highlightCall = mHighlightCall;

            callStart = highlightCall->getStartTime();
            callEnd = highlightCall->getEndTime();

            if (callStart >= minVal) {
                callPixelStart = mScaleInfo.valueToPixel(callStart);
            }

            if (callEnd <= maxVal) {
                callPixelEnd = mScaleInfo.valueToPixel(callEnd);
            }

            //Compute the Y axis by (thread->row)
            id_type threadId = highlightCall->getThreadId();
            callRowData = mDmTraceData->getThreadData(threadId);
            int y1 = callRowData->mRank * 32 + 32 - 6;

            //Get color from method data
            callMethod = highlightCall->getMethodData();
            COLOR color = callMethod->getColor();
            mHighlightInclusive.push_back(Range(callPixelStart + 10, callPixelEnd + 10, y1, color));
        }

        ThreadPtrList* sortedThreads = mDmTraceData->getThreads();

        for (auto _ti = sortedThreads->begin(); _ti != sortedThreads->end(); _ti++) {
            ThreadData* thread = *_ti;
            StripList& stripList = thread->getStripList();
            stripList.clear();

            if (thread->mRank > mEndRow) {
                break;
            }

            if (thread->isEmpty()) {
                continue;
            }

            CallList* callList = thread->getCallList();

            int y1 = thread->mRank * 32 + 32 - 6;
            Pixel pixel;

            CallStack stack("computeStrip");

            for (int callIndex = 0; callIndex < callList->size(); callIndex++) {
                Call* call = callList->get(callIndex);
                uint32_t blockStartTime = call->getStartTime();
                uint32_t blockEndTime = call->getEndTime();
                uint32_t currentStartTime = mScaleInfo.pixelToValue(pixel.mStart);

                //Check if segment is out of visible range
                if (blockEndTime <= currentStartTime)
                {
                    callIndex = call->getEnd();
                    continue;
                }

                if (blockStartTime >= maxVal)
                {
                    break;
                }

                if (call->isIgnoredBlock(callList)) {
                    continue;
                }

                COLOR color;
                if (call->isContextSwitch()) {
                    color = call->getParentBlock(callList)->getColor();
                }
                else {
                    color = call->getColor();
                }

                // Clip block to range of minVal~maxVal
                uint32_t recordStart = std::max(blockStartTime, minVal);
                uint32_t recordEnd = std::min(blockEndTime, maxVal);

                // if range of the Call is empty, skip to next call
                if (recordStart == recordEnd) {
                    callIndex = call->getEnd();
                    continue;
                }

                bool isContextSwitch = call->isContextSwitch();

                int top = stack.top();
                if (top == -1) {
                    stack.push(call->getIndex());
                    continue;
                }
                Call* topCall = callList->get(top);
                uint32_t topStartTime = topCall->getStartTime();
                uint32_t topEndTime = topCall->getEndTime();
                bool skipChild = false;

                // topEndTime > blockStartTime ==> current block is inside top block
                // topEndTime = blockStartTime ==> current block is just following the top block
                // topEndTime < blockStartTime ==> current block is behind top block
                if (topEndTime >= blockStartTime) {

                    if (topStartTime < blockStartTime) {
                        //Next call(child or siblings) belongs to the next pixel
                        createStrip(topStartTime, recordStart, y1, topCall, isContextSwitch, pixel);
                    }

                    if (topEndTime == blockStartTime)
                        stack.pop();

                    stack.push(call->getIndex());
                }
                else {
                    popFrames(stack, callList, topCall, blockStartTime, pixel, &stripList);
                    stack.push(call->getIndex());
                }

                if (skipChild) {
                    callIndex = call->getEnd();
                }
            }

            int top = stack.top();
            if (top != -1)
            {
                popFrames(stack, callList, callList->get(top), INT_MAX, pixel, &stripList);
            }
        }

        if (mDmTraceData->isRegression()) {
            dumpStrips();
        }
    }

    void DmTraceControl::createStrip(uint32_t recordStart, uint32_t recordEnd, uint32_t baseline, Call* call, bool isContextSwitch, Pixel& pixel)
    {
        assert(call != nullptr);
        StripList& stripList = call->getThreadData()->getStripList();

        double mStartFraction = mScaleInfo.pixelToValue(pixel.mStart);
        double mPixelFraction = mScaleInfo.pixelToValue(pixel.mStart + 1);
        double mValuePerPixel = mPixelFraction - mStartFraction;

        int pixelStart = std::max(pixel.mStart, mScaleInfo.valueToPixel(recordStart));
        int pixelEnd = mScaleInfo.valueToPixel(recordEnd);

        int pixelWidth = pixelEnd - pixelStart;

        if (pixelWidth <= 0) {
            return;
        }

        int stripHeight = 20;
        if (isContextSwitch) {
            stripHeight = 1;
        }

        if (mDmTraceData->isRegression()) {
            printf("begin createStrip: thread=%d, mStart=%d\tstart=%d\tend=%d\tpixelStart=%d\tpixelEnd=%d\tpixelWidth=%d\n",
                call->getThreadId(), pixel.mStart, recordStart, recordEnd, pixelStart, pixelEnd, pixelWidth);
        }

        uint32_t width = recordEnd - recordStart;

        if (pixelWidth == 0) {
            //Emit a single pixel 
            // Skip the block which is too short(shorted than 0.5 pixel)
            if (width * 2 >= mValuePerPixel) {
                Strip* strip = stripList.addNull2();
                strip->init(pixelStart, baseline - stripHeight, 1, stripHeight, nullptr, call, call->getColor());
                pixel.mStart++;
            }
        }
        else {
            double remainder = width - mValuePerPixel * pixelWidth;
            printf("remainder=%f, valuePerpixel=%f\n", remainder * 2.0, mValuePerPixel);
            if (remainder * 2.0 >= mValuePerPixel) {
                pixelWidth++;
            }
            Strip* strip = stripList.addNull2();
            strip->init(pixelStart, baseline - stripHeight, pixelWidth, stripHeight, nullptr, call, call->getColor());
            pixel.mStart += pixelWidth;
        }
        if (mDmTraceData->isRegression()) {
            printf("End createStrip: mStart=%d, pixelWidth=%d\n\n", pixel.mStart, pixelWidth);
            fflush(stdout);
        }
    }

    void DmTraceControl::popFrames(CallStack& stack, CallList* callList, Call* top, uint32_t startTime, Pixel& pixel, FastArray<Strip>* stripList)
    {
        uint32_t topEndTime = top->getEndTime();
        uint32_t lastEndTime = top->getStartTime();
        int y1 = top->getThreadData()->mRank * 32 + 32 - 6;

        while (topEndTime <= startTime) {
            if (topEndTime > lastEndTime) {
                createStrip(lastEndTime, topEndTime, y1, top, top->isContextSwitch(), pixel);
                lastEndTime = topEndTime;
            }
            stack.pop();

            int topIndex = stack.top();
            if (topIndex == -1)
                return;
            top = callList->get(topIndex);

            topEndTime = top->getEndTime();
        }

        if (lastEndTime < startTime) {
            createStrip(lastEndTime, startTime, y1, top, top->isContextSwitch(), pixel);
        }
    }

    void DmTraceControl::dumpStrips()
    {
        printf("Strip list\n");
        printf("   Tid  \t   x   \t  width \t   y   \t height \t color  \tmethod\n");
        ThreadPtrList* sortedThreads = mDmTraceData->getThreads();

        for (auto _ti = sortedThreads->begin(); _ti != sortedThreads->end(); _ti++) {
            ThreadData* threadData = *_ti;
            StripList& stripList = threadData->getStripList();

            for (auto i = 0; i < stripList.size(); i++) {
                Strip* strip = stripList.get(i);
                Call* call = strip->mCall;
                printf("%8d\t%8d\t%8d\t%8d\t%8d\t%06x\t%s\n", call->getThreadId(), strip->mX, strip->mWidth, strip->mY, strip->mHeight, strip->mColor, call->getName());
            }
        }
    }

    void DmTraceControl::setBoundary(Rectangle* rcBound)
    {
        Canvas::setBoundary(rcBound);

        Rectangle rcThreadLabel;
        Rectangle rcTimescale;
        Rectangle rcTimeLine;

        rcThreadLabel.topleft.x = rcBound->topleft.x;
        rcThreadLabel.topleft.y = rcBound->topleft.y + topMargin;
        rcThreadLabel.size.cx = 250;  //No changed unless user change it manually
        rcThreadLabel.size.cy = std::max(rcBound->size.cy - topMargin, 0);

        rcTimescale.topleft.x = rcBound->topleft.x + rcThreadLabel.size.cx;
        rcTimescale.topleft.y = 0;
        rcTimescale.size.cx = std::max(0, rcBound->size.cx - rcThreadLabel.size.cx);
        rcTimescale.size.cy = topMargin;

        rcTimeLine.topleft.x = rcTimescale.topleft.x;
        rcTimeLine.topleft.y = rcBound->topleft.y + topMargin;
        rcTimeLine.size.cx = std::max(0, rcBound->size.cx - rcThreadLabel.size.cx);
        rcTimeLine.size.cy = std::max(rcBound->size.cy - topMargin, 0);

        if (mDmTraceData == nullptr) {
            return;
        }

        mThreadLabel->setBoundary(&rcThreadLabel);
        mTimescale->setBoundary(&rcTimescale);
        mTimeLine->setBoundary(&rcTimeLine);
    }

    void TimeLineView::setBoundary(Rectangle* rcBound)
    {
        Canvas::setBoundary(rcBound);
        TickScaler& scaleInfo = mParent->getScaleInfo();
        scaleInfo.setNumPixels(rcBound->size.cx);
        scaleInfo.computeTicks(true);
        mParent->computeVisibleRows(rcBound->size.cy);
        mParent->computeStrips();
    }

    void TimeScaleView::setBoundary(Rectangle* rcBound)
    {
        Canvas::setBoundary(rcBound);
    }

    void ThreadLabelView::setBoundary(Rectangle* rcBound)
    {
        Canvas::setBoundary(rcBound);
    }
};
