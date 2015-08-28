#include "TimeLineView.hpp"
#include "Selection.hpp"
#include "TraceUnits.hpp"

namespace Android {
    const double Pixel::qualifiedWeight = 0.5;

    Range::Range(int xStart, int width, int y, COLOR color)
    {
        mXdim.x = xStart;
        mXdim.y = width;
        mY = y;
        mColor = color;
    }

    Strip::Strip(int x, int y, int width, int height, ThreadData* thread, Segment* segment, COLOR color)
    {
        mX = x;
        mY = y;
        mWidth = width;
        mHeight = height;
        mThread = thread;
        mSegment = segment;
        mColor = color;
    }

    Strip::Strip(int x, int y, int width, int height, ThreadData* thread, Call* call, COLOR color)
    {
        mX = x;
        mY = y;
        mWidth = width;
        mHeight = height;
        mThread = thread;
        mCall = call;
        mColor = color;
    }

    void Pixel::setFields(int start, double weight, Segment* segment, COLOR color, ThreadData* thread)
    {
        mStart = start;
        mMaxWeight = weight;
        mSegment = segment;
        mColor = color;
        mThread = thread;
    }

    void Pixel::setFields(int start, double weight, Call* call, COLOR color, ThreadData* thread)
    {
        mStart = start;
        mMaxWeight = weight;
        mCall = call;
        mColor = color;
        mThread = thread;
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

    void Surface::initZoomFractionsWithSinWave()
    {
        for (auto ii = 0; ii < 8; ii++) {
            double offset = 3.141592653589793 * ii / 8.0;
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
        double minVal = mParent->getScaleInfo().getMinVal();
        double maxVal = mParent->getScaleInfo().getMaxVal();
        double visibleRange = maxVal - minVal;
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

    void Surface::draw()
    {
        if (mParent->mSegments.size() == 0) {
            return;
        }

        if (mGraphicsState == Scaling) {
            double diff = mMouse.x - mMouseMarkStartX;
            if (diff > 0.0) {
                double newMinVal = mScaleMinVal - diff / mScalePixelsPerRange;
                if (newMinVal < mLimitMinVal)
                    newMinVal = mLimitMinVal;

                mParent->getScaleInfo().setMinVal(newMinVal);
            }
            else if (diff < 0.0) {
                double newMaxVal = mScaleMaxVal - diff / mScalePixelsPerRange;
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
            computeStrips();
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
            double pixelsPerRange = mParent->getScaleInfo().getPixelsPerRange();
            printf("dim.x %d pixels %d minVal %f, maxVal %f ppr %f rpp %f\n",
                dim.cx, dim.cx - 70,
                mParent->getScaleInfo().getMinVal(), mParent->getScaleInfo().getMaxVal(),
                pixelsPerRange, 1.0 / pixelsPerRange);
        }

        Call* selectBlock = nullptr;
        for (auto it = mStripList.begin(); it != mStripList.end(); it++) {
            Strip* strip = *it;
            if (strip->mColor != 0) {

                if (mParent->mMouseRow == strip->mThread->mRank) {
                    if ((mMouse.x >= strip->mX) && (mMouse.x < strip->mX + strip->mWidth)) {
                        Call* block = strip->mSegment->mBlock;
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
                        selectBlock = strip->mSegment->mBlock;
                    }
                }
            }
        }

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
            //mParent->mSelectionController->change(selections, "TimeLineView");
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
        double minVal = mParent->getScaleInfo().getMinVal();
        double maxVal = mParent->getScaleInfo().getMaxVal();

        //for (auto ii = 0; ii < mParent->mSegments.size(); ii++) {
        //	mParent->mCallList->get(mParent->mSegments[ii].mBlock)->clearWeight();
        //}

        mStripList.clear();
        mHighlightExclusive.clear();
        mHighlightInclusive.clear();

        MethodData* callMethod = nullptr;
        ThreadData* callRowData = nullptr;

        uint32_t callStart = 0;
        uint32_t callEnd = UINT32_MAX;
        uint32_t prevMethodStart = UINT32_MAX;
        uint32_t prevMethodEnd = UINT32_MAX;
        uint32_t prevCallStart = UINT32_MAX;
        uint32_t prevCallEnd = UINT32_MAX;

        // if a Call in timeline view is selected, it will be highlighted
        if (mParent->mHighlightCall != nullptr) {

            int callPixelStart = -1;
            int callPixelEnd = -1;
            Call* highlightCall = mParent->mHighlightCall;

            callStart = highlightCall->getStartTime();
            callEnd = highlightCall->getEndTime();

            if (callStart >= minVal) {
                callPixelStart = mParent->getScaleInfo().valueToPixel(callStart);
            }

            if (callEnd <= maxVal) {
                callPixelEnd = mParent->getScaleInfo().valueToPixel(callEnd);
            }

            //Compute the Y axis by (thread->row)
            id_type threadId = highlightCall->getThreadId();
            callRowData = mParent->mTraceData->getThreadData(threadId);
            int y1 = callRowData->mRank * 32 + 6;

            //Get color from method data
            callMethod = highlightCall->getMethodData();
            COLOR color = callMethod->getColor();
            mHighlightInclusive.push_back(Range(callPixelStart + 10, callPixelEnd + 10, y1, color));
        }
#if 0
        for (auto ii = 0; ii < mParent->mSegments.size(); ii++) {
            Segment* segment = mParent->mSegments.get(ii);

            //Check if segment is out of visible range
            if (segment->mEndTime <= minVal || segment->mStartTime >= maxVal)
            {
                continue;
            }

            Call* block = segment->mBlock;
            COLOR color = block->getColor();
            if (color == 0) {
                continue;
            }

            // Clip block to range of minVal~maxVal
            double recordStart = std::max(static_cast<double>(segment->mStartTime), minVal);
            double recordEnd = std::min(static_cast<double>(segment->mEndTime), maxVal);

            // if range is empty
            if (recordStart == recordEnd) {
                continue;
            }

            int pixelStart = mParent->getScaleInfo().valueToPixel(recordStart);
            int pixelEnd = mParent->getScaleInfo().valueToPixel(recordEnd);
            int width = pixelEnd - pixelStart;
            bool isContextSwitch = segment->mIsContextSwitch;

            ThreadData* thread = segment->mThreadData;

            // if beyond the last row(thread), break the loop
            if (thread->mRank > mParent->mEndRow) {
                break;
            }
            int y1 = thread->mRank * 32 + 6;

            MethodData* md = block->getMethodData();

            if (mParent->mHighlightMethodData != nullptr) {
                if (mParent->mHighlightMethodData == md) {
                    if ((prevMethodStart != pixelStart) || (prevMethodEnd != pixelEnd)) {
                        prevMethodStart = pixelStart;
                        prevMethodEnd = pixelEnd;
                        int rangeWidth = width;
                        if (rangeWidth == 0)
                            rangeWidth = 1;

                        mHighlightExclusive.push_back(Range(pixelStart + 10, rangeWidth, y1, color));

                        int callPixelStart = -1;
                        callStart = block->getStartTime();
                        if (callStart >= minVal)
                            callPixelStart = mParent->getScaleInfo().valueToPixel(callStart);

                        int callPixelEnd = -1;
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
                if ((segment->mStartTime >= callStart) && (segment->mEndTime <= callEnd) && (callMethod == md) && (callRowData == thread)) {
                    if ((prevMethodStart != pixelStart) || (prevMethodEnd != pixelEnd)) {
                        prevMethodStart = pixelStart;
                        prevMethodEnd = pixelEnd;
                        int rangeWidth = width;
                        if (rangeWidth == 0)
                            rangeWidth = 1;

                        mHighlightExclusive.push_back(Range(pixelStart + 10, rangeWidth, y1, color));
                    }
                }
                else if (mFadeColors) {
                    color = md->getFadedColor();
                }
            }

            Pixel* pix = &pixels[thread->mRank];

            if (pix->mStart != pixelStart) {

                // Compare the current segment to previous pixel. If pixel shifted, the previous pixel should be emitted.
                if (pix->mSegment != nullptr) {

                    // There is segment recorded for the pixel. Create a strip for it.
                    emitPixelStrip(thread, y1, pix);
                }
                if (width == 0) {
                    // If the segment is within one pixel, the Call with most weight will be displayed.
                    // But at first, we record the pixel(with its weight and color) to Pixel* pix
                    double weight = computeWeight(recordStart, recordEnd, isContextSwitch, pixelStart);
                    weight = block->addWeight(pixelStart, thread->mRank, weight);
                    if (weight > pix->mMaxWeight) {
                        pix->setFields(pixelStart, weight, segment, color, thread);
                    }
                }
                else {
                    // if the segment larger than 1 pixel, create a multi-pixel strip
                    int x1 = pixelStart + 10;
                    Strip* strip = new Strip(x1, isContextSwitch ? y1 + 20 - 1 : y1, width, isContextSwitch ? 1 : 20, thread, segment, color);
                    mStripList.push_back(strip);
                }
            }
            else {

                // Still start with same pixel, need to compute weight for the first pixel
                double weight = computeWeight(recordStart, recordEnd, isContextSwitch, pixelStart);
                weight = block->addWeight(pixelStart, thread->mRank, weight);
                if (weight > pix->mMaxWeight) {
                    pix->setFields(pixelStart, weight, segment, color, thread);
                }

                if (width == 1) {
                    // For one new pixel, finish current pixel and start a new pixel weighting
                    emitPixelStrip(thread, y1, pix);
                    pixelStart++;
                    weight = computeWeight(recordStart, recordEnd, isContextSwitch, pixelStart);
                    weight = block->addWeight(pixelStart, thread->mRank, weight);
                    pix->setFields(pixelStart, weight, segment, color, thread);
                }
                else if (width > 1) {
                    // Finish current pixel and create new multi-pixel strip
                    emitPixelStrip(thread, y1, pix);
                    pixelStart++;
                    width--;
                    int x1 = pixelStart + 10;
                    Strip* strip = new Strip(x1, isContextSwitch ? y1 + 20 - 1 : y1, width, isContextSwitch ? 1 : 20, thread, segment, color);
                    mStripList.push_back(strip);
                }
            }
        }
#endif

#if 0
        for (auto ii = 0; ii < mParent->mNumRows; ii++) {
            Pixel* pix = &pixels[ii];
            if (pix->mSegment != nullptr) {
                ThreadData* thread = pix->mThread;
                int y1 = thread->mRank * 32 + 6;
                emitPixelStrip(thread, y1, pix);
            }
        }
#endif
        ThreadPtrList* sortedThreads = mParent->mTraceData->getThreads();

        for (auto _ti = sortedThreads->begin(); _ti != sortedThreads->end(); _ti++) {
            ThreadData* thread = *_ti;

            if (thread->mRank > mParent->mEndRow) {
                break;
            }

            if (thread->isEmpty()) {
                continue;
            }

            CallList* callList = thread->getCallList();

            int mStart = -2;
            double mStartFraction = mParent->getScaleInfo().pixelToValue(mStart);
            double mPixelFraction = mParent->getScaleInfo().pixelToValue(mStart + 1);

            int y1 = thread->mRank * 32 + 6;
            double mMaxWeight = 0.0;
            Call* mCall = nullptr;
            COLOR mColor = 0;
            Strip currStrip(0, 0, 0, 0, nullptr, (Call*)nullptr, 0);

            CallStack stack;

            for (int callIndex = 0; callIndex < callList->size(); callIndex++) {
                Call* call = callList->get(callIndex);
                COLOR color = call->getColor();
                uint32_t blockStartTime = call->getStartTime();
                uint32_t blockEndTime = call->getEndTime();

                //Check if segment is out of visible range
                if (blockEndTime <= minVal)
                {
                    callIndex = call->getEnd();
                    continue;
                }
                if (blockStartTime >= maxVal)
                {
                    break;
                }

                if (!call->isIgnoredBlock(callList)) {

                    // Clip block to range of minVal~maxVal
                    double recordStart = std::max(static_cast<double>(blockStartTime), minVal);
                    double recordEnd = std::min(static_cast<double>(blockEndTime), maxVal);

                    // if range of the Call is empty, skip to next call
                    if (recordStart == recordEnd) {
                        callIndex = call->getEnd();
                        continue;
                    }

                    int pixelStart = mParent->getScaleInfo().valueToPixel(recordStart);
                    int pixelEnd = mParent->getScaleInfo().valueToPixel(recordEnd);
                    int width = pixelEnd - pixelStart;
                    bool isContextSwitch = call->isContextSwitch();

                    int top = stack.top();
                    if (top == -1) {
                        stack.push(call->getIndex());
                    }
                    else {
                        Call* topCall = callList->get(top);
                        uint32_t topStartTime = topCall->getStartTime();
                        uint32_t topEndTime = topCall->getEndTime();

                        // topEndTime > blockStartTime ==> current block is inside top block
                        // topEndTime = blockStartTime ==> current block is behind top block
                        // topEndTime < blockStartTime ==> current block is behind top block
                        if (topEndTime >= blockStartTime) {
                            if (topStartTime < blockStartTime) {
                                Segment* segment = mSegments.addNull2();
                                segment->init(thread, callList, topCall->getIndex(), topStartTime, blockStartTime);
                            }
                            if (topEndTime == blockStartTime)
                                stack.pop();

                            stack.push(call->getIndex());
                        }
                        else {
                            popFrames(thread, callList, topCall, blockStartTime, &mSegments);
                            stack.push(call->getIndex());
                        }
                    }
                }

                int top = stack.top();
                if (top != -1)
                {
                    popFrames(thread, callList, callList->get(top), INT_MAX, &mSegments);
                }

            }
        }
    }

    double Surface::computeWeight(double start, double end, bool isContextSwitch, int pixel)
    {
//        if (isContextSwitch) {
//            return 0.0;
//        }
        double pixelStartFraction = mParent->getScaleInfo().valueToPixelFraction(start);
        double pixelEndFraction = mParent->getScaleInfo().valueToPixelFraction(end);
        double leftEndPoint = std::max(pixelStartFraction, pixel - 0.5);
        double rightEndPoint = std::min(pixelEndFraction, pixel + 0.5);
        double weight = rightEndPoint - leftEndPoint;
        return weight;
    }

    /* Create a strip which has only one plixel
        1. The pixel should be splitted into 2 parts: upper for call,
          and lower in background color
        2. After emit the pixel, clear Pixel element for next pixel
     */
    void Surface::emitPixelStrip(ThreadData* thread, int y, Pixel* pixel)
    {
        if (pixel->mSegment == nullptr) {
            return;
        }
        int x = pixel->mStart + 10;
        int height = static_cast<int>((pixel->mMaxWeight * 20.0 * 0.75));
        if (height < mMinStripHeight)
            height = mMinStripHeight;

        int remainder = 20 - height;
        if (remainder > 0) {
            Strip* strip = new Strip(x, y, 1, remainder, thread, pixel->mSegment, mFadeColors ? 0 : 1); // TODO
            mStripList.push_back(strip);
        }

        Strip* strip = new Strip(x, y + remainder, 1, height, thread, pixel->mSegment, pixel->mColor);
        mStripList.push_back(strip);
        pixel->mSegment = nullptr;
        pixel->mMaxWeight = 0.0;
    }

    void Surface::mouseMove(Point& pt, int flags)
    {
        Size dim = mParent->mSurface->getSize();
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
        int rownum = (mMouse.y + mParent->mScrollOffsetY) / 32;
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
        Size dim = mParent->mSurface->getSize();
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

    void Surface::mouseUp(Point& pt, int flags)
    {
        if (mGraphicsState != Marking) {
            mGraphicsState = Normal;
            return;
        }
        mGraphicsState = Animating;
        Size dim = mParent->mSurface->getSize();
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
        double minVal = mParent->getScaleInfo().getMinVal();
        double maxVal = mParent->getScaleInfo().getMaxVal();
        double ppr = mParent->getScaleInfo().getPixelsPerRange();
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

    void Surface::mouseScrolled(Point& pt, int flags)
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
            Size dim = mParent->mSurface->getSize();
            int x = pt.x;
            if (x < 10)
                x = 10;

            if (x > dim.cx - 60)
                x = dim.cx - 60;

            double ppr = mParent->getScaleInfo().getPixelsPerRange();
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
        mParent->mSurface->redraw();
    }

    void Surface::mouseDoubleClick(Point& pt, int flags)
    {
    }

    void Surface::startScaling(int mouseX)
    {
        Size dim = mParent->mSurface->getSize();
        int x = mouseX;
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
        int rownum = (pt.y + mParent->mScrollOffsetY) / 32;
        if (mParent->mMouseRow != rownum) {
            mParent->mMouseRow = rownum;
            redraw();
            mParent->mSurface->redraw();
        }
    }

    TimeLineView::TimeLineView(DmTraceReader* reader)
        : mScaleInfo(0.0, 0.0, 0, 50)
        , mMouseRow(-1)
        , mStartRow(0)
        , mEndRow(0)
        , mNumRows(0)
        , mScrollOffsetY(0)
    {
        mTraceData = nullptr;
        mHighlightMethodData = nullptr;
        mHighlightCall = nullptr;
        //mUnits = reader->getTraceUnits();
        mClockSource = reader->getClockSource();
        mHaveCpuTime = reader->haveCpuTime();
        mHaveRealTime = reader->haveRealTime();

        mTimescale = new Timescale(this);
        mSurface = new Surface(this);
        mLabels = new RowLabels(this);

        setData(reader);
    }

    void TimeLineView::setData(DmTraceReader* reader)
    {
        mTraceData = reader;

        double maxVal = reader->getMaxTime();
        double minVal = reader->getMinTime();

        mNumRows = reader->getThreads()->size();
        mScaleInfo.setMaxVal(maxVal);
        mScaleInfo.setMinVal(minVal);
        computeVisibleRows(10000);

        mSurface->setRange(minVal, maxVal);
        mSurface->setLimitRange(minVal, maxVal);
        mSurface->computeStrips();
    }

    Call* TimeLineView::startBetween(id_type threadId, uint32_t timeLeft, uint32_t timeRight)
    {
        CallList* callList = mTraceData->getCallList(threadId);

        if (callList == nullptr || callList->size() == 0) {
            return nullptr;
        }

        int callIndex = 0;
        while (callIndex < callList->size()) {
            Call* call = callList->get(callIndex);

            uint32_t startTime = call->getStartTime();


            // ---timeLeft--------timeRight-------startTime---
            //        |                |               |
            if (timeRight <= startTime) {
                return nullptr;
            }

            // ---timeLeft--------startTime--------timeRight---
            //        |                |               |
            if (timeLeft <= startTime && startTime < timeRight) {
                return call;
            }

            uint32_t endTime = call->getEndTime();

            // -----startTime-------endTime------timeLeft-------timeRight---
            //        |                |               |            |
            // There is no intersect, proceed to next block
            if (endTime < timeLeft) {
                callIndex = call->getNext();
                while (callIndex == -1) {
                    int caller = call->getParentBlockIndex();
                    if (caller == -1) {
                        // Call List is over
                        return nullptr;
                    }
                    callIndex = callList->get(caller)->getNext();
                }
                continue;
            }

            // -----startTime-------timeLeft-------endTime------timeRight---
            //        |                |               |            |
            //  or
            // -----startTime-------timeLeft------timeRight------endTime---_
            //        |                |               |            |
            // There is intersect, proceed to first child
            callIndex++;
        }
        return nullptr;
    }

    int TimeLineView::computeVisibleRows(int ydim)
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

    void TimeLineView::startHighlighting()
    {
        mSurface->mHighlightStep = 0;
        mSurface->mFadeColors = true;
        mSurface->mCachedEndRow = -1;
    }

};
