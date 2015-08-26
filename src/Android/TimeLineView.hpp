// Generated from /Traceview/src/com/android/traceview/TimeLineView.java

#pragma once

#include "Common.hpp"
#include "Call.hpp"
#include "MethodData.hpp"
#include "ThreadData.hpp"
#include "DmTraceReader.hpp"
#include <math.h>

namespace Android {

    class Selection;
    class TraceUnits;

    typedef enum GraphicsState {
        Normal, Marking, Scaling, Animating, Scrolling,
    } GraphicsState;

    struct Point
    {
        int x;
        int y;
    };

    struct Size
    {
        int cx;
        int cy;
    };

    class Range : public Object
    {
    public:
        Point mXdim;
        int mY;
        COLOR mColor;

        // Generated
        Range(int xStart, int width, int y, COLOR color);
    };

    class Strip : public Object
    {
    public:
        int mX;
        int mY;
        int mWidth;
        int mHeight;
        ThreadData* mThread;
        Segment*    mSegment;
        Call*       mCall;
        COLOR       mColor;

    public:
        Strip(int x, int y, int width, int height, ThreadData* thread, Segment* segment, COLOR color);
        Strip(int x, int y, int width, int height, ThreadData* thread, Call* call, COLOR color);
    };

    class TickScaler : public Object
    {
    protected:
        double mMinVal;
        double mMaxVal;
        double mRangeVal;
        int mNumPixels;
        int mPixelsPerTick;
        double mPixelsPerRange;
        double mTickIncrement;
        double mMinMajorTick;

    public:

        TickScaler(double minVal, double maxVal, int numPixels, int pixelsPerTick)
        {
            mMinVal = minVal;
            mMaxVal = maxVal;
            mNumPixels = numPixels;
            mPixelsPerTick = pixelsPerTick;
        }

        void setMinVal(double minVal)
        {
            mMinVal = minVal;
        }

        double getMinVal()
        {
            return mMinVal;
        }

        void setMaxVal(double maxVal)
        {
            mMaxVal = maxVal;
        }

        double getMaxVal()
        {
            return mMaxVal;
        }

        // Set the pixels for visible width
        void setNumPixels(int numPixels)
        {
            mNumPixels = numPixels;
        }

        uint32_t getNumPixels()
        {
            return mNumPixels;
        }

        void setPixelsPerTick(int pixelsPerTick)
        {
            mPixelsPerTick = pixelsPerTick;
        }

        uint32_t getPixelsPerTick()
        {
            return mPixelsPerTick;
        }

        void setPixelsPerRange(double pixelsPerRange)
        {
            mPixelsPerRange = pixelsPerRange;
        }

        double getPixelsPerRange()
        {
            return mPixelsPerRange;
        }

        void setTickIncrement(double tickIncrement)
        {
            mTickIncrement = tickIncrement;
        }

        double getTickIncrement()
        {
            return mTickIncrement;
        }

        void setMinMajorTick(double minMajorTick)
        {
            mMinMajorTick = minMajorTick;
        }

        double getMinMajorTick()
        {
            return mMinMajorTick;
        }

        int valueToPixel(double value)
        {
            return (int)ceil(mPixelsPerRange * (value - mMinVal) - 0.5);
        }

        double valueToPixelFraction(double value)
        {
            return mPixelsPerRange * (value - mMinVal);
        }

        double pixelToValue(int pixel)
        {
            return mMinVal + pixel / mPixelsPerRange;
        }

        /* Compute the scale ticks for following parameters:
             1. mMinVal, mMaxVal ==> The visible range of min/max timeline in us(which is aligned with scaled tick
                  mRangeVal          = mMaxVal - mMinVal

             2. mNumPixels       ==> How many pixels is visible width range

             3. mTickIncrement   ==> Scaling unit (1, 2, 5 * 10^N)
                  minorTickIncrement = 1/5 of mTickIncrement

             4. mPixelsPerRange  ==> How many pixels within one Tick
         */
        void computeTicks(bool useGivenEndPoints)
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
                int ival = static_cast<int>((mMinVal / mTickIncrement));
                mMinMajorTick = (ival * mTickIncrement);
                if (mMinMajorTick < mMinVal) {
                    mMinMajorTick += mTickIncrement;
                }
            }
            mRangeVal = (mMaxVal - mMinVal);
            mPixelsPerRange = (mNumPixels / mRangeVal);
        }
    };

    class Pixel : public Object
    {
    public:
        
        static const double qualifiedWeight = 0.5;
        Pixel()
        {
            mStart = -2;
        }

        void setFields(int start, double weight, Segment* segment, COLOR color, ThreadData* thread);
        void setFields(int start, double weight, Call* call, COLOR color, ThreadData* thread);

    public:
        int mStart;
        double mMaxWeight;
        Segment* mSegment;
        Call* mCall;
        COLOR mColor;
        ThreadData* mThread;

    protected:
        friend class TimeLineView;
    };

    class TimeLineView;

    class Canvas : public Object
    {
    public:
        Canvas() {
            mSize.cx = mSize.cy = 0;
        }

        Canvas(Size& size)
            : mSize(size)
        {
        }

        virtual void mouseMove(Point& pt, int flags);
        virtual void mouseDown(Point& pt, int flags);
        virtual void mouseUp(Point& pt, int flags);
        virtual void mouseScrolled(Point& pt, int flags);
        virtual void mouseDoubleClick(Point& pt, int flags);

        void setSize(Size size)
        {
            mSize = size;
        }

        Size getSize() const {
            return mSize;
        }

        virtual void redraw() { /* Do nothing by default */ }
    protected:
        Size mSize;
    };

    class Surface : public Canvas
    {
    protected:
        const int TotalXMargin = 70;
        const int yMargin = 1;
        const int MinZoomPixelMargin = 10;

    protected:
        void initZoomFractionsWithExp();
        void initZoomFractionsWithSinWave();

    public:
        void setRange(double minVal, double maxVal);
        void setLimitRange(double minVal, double maxVal);
        void resetScale();
        void setScaleFromHorizontalScrollBar(int selection);

    protected:
        void draw();
        void drawHighlights(Point dim);
        bool drawingSelection();
        void computeStrips();
        double computeWeight(double start, double end, bool isContextSwitch, int pixel);
        void emitPixelStrip(ThreadData* thread, int y, Pixel* pixel);
        virtual void mouseMove(Point& pt, int flags);
        virtual void mouseDown(Point& pt, int flags);
        virtual void mouseUp(Point& pt, int flags);
        virtual void mouseScrolled(Point& pt, int flags);
        virtual void mouseDoubleClick(Point& pt, int flags);

    public:
        void startScaling(int mouseX);
        void stopScaling(int mouseX);

    protected:
        GraphicsState mGraphicsState;
        Point mMouse;
        int mMouseMarkStartX;
        int mMouseMarkEndX;
        bool mDebug;
        Vector<Strip*> mStripList;
        Vector<Range> mHighlightExclusive;
        Vector<Range> mHighlightInclusive;
        int mMinStripHeight;
        double mCachedMinVal;
        double mCachedMaxVal;
        int mCachedStartRow;
        int mCachedEndRow;
        double mScalePixelsPerRange;
        double mScaleMinVal;
        double mScaleMaxVal;
        double mLimitMinVal;
        double mLimitMaxVal;
        double mMinDataVal;
        double mMaxDataVal;
        const int ZOOM_TIMER_INTERVAL = 10;
        const int HIGHLIGHT_TIMER_INTERVAL = 50;
        const int ZOOM_STEPS = 8;

        int mHighlightHeight;
        static int highlightHeights[];
        int HIGHLIGHT_STEPS;
        bool mFadeColors;
        bool mShowHighlightName;
        double mZoomFractions[8];
        int mZoomStep;
        int mZoomMouseStart;
        int mZoomMouseEnd;
        int mMouseStartDistance;
        int mMouseEndDistance;
        Point mMouseSelect;
        double mZoomFixed;
        double mZoomFixedPixel;
        double mFixedPixelStartDistance;
        double mFixedPixelEndDistance;
        double mZoomMin2Fixed;
        double mMin2ZoomMin;
        double mFixed2ZoomMax;
        double mZoomMax2Max;
        double mZoomMin;
        double mZoomMax;
        int mHighlightStep;

    protected:		//Callbacks
        void animateHighlight();
        void clearHighlights();
        void animateZoom();

    public:
        Surface(TimeLineView *parent);
        TimeLineView *mParent;

    public:
        friend class TimeLineView;
    };

    class Timescale : public Canvas
    {
    protected:
        Point mMouse;
        String mMethodName;
        COLOR mMethodColor;
        String mDetails;
        int mMethodStartY;
        int mDetailsStartY;
        int mMarkStartX;
        int mMarkEndX;
        const int METHOD_BLOCK_MARGIN = 10;

    protected:
        TimeLineView *mParent;

    public:
        Timescale(TimeLineView *parent);
    };

    class RowLabels : public Canvas
    {
    protected:
        const int labelMarginX = 2;

    protected:
        void mouseMove(Point& pt, int flags);
        void draw();

    public:
        RowLabels(TimeLineView *parent);
        TimeLineView *mParent;

    protected:
        friend class TimeLineView;
    };

    class TimeLineView : public Object
    {
    protected:
        SegmentList mSegments;
        Timescale* mTimescale;
        Surface* mSurface;
        RowLabels* mLabels;
        int mScrollOffsetY;

    public:
        const int PixelsPerTick = 50;

    protected:
        const int LeftMargin = 10;
        const int RightMargin = 60;
        const int rowHeight = 20;
        const int rowYMargin = 12;
        const int rowYMarginHalf = 6;
        const int rowYSpace = 32;
        const int majorTickLength = 8;
        const int minorTickLength = 4;
        const int timeLineOffsetY = 58;
        const int tickToFontSpacing = 2;
        const int topMargin = 90;

        const int MinInclusiveRange = 3;

        DmTraceReader* mTraceData;
        TickScaler mScaleInfo;

        int mMouseRow;
        int mNumRows;
        int mStartRow;
        int mEndRow;

        TraceUnits* mUnits;
        String mClockSource;

        bool mHaveCpuTime;
        bool mHaveRealTime;

        MethodData* mHighlightMethodData;
        Call* mHighlightCall;
        Size mSize;

    public:

        Size getSize() const {
            return mSize;
        }

        TickScaler& getScaleInfo() {
            return mScaleInfo;
        }

        void setData(DmTraceReader* reader);

        Call* startBetween(id_type threadId, uint32_t timeLeft, uint32_t timeRight);

    protected:
        int computeVisibleRows(int ydim);
        void startHighlighting();

    public:
        TimeLineView(DmTraceReader* reader);

        friend class Surface;
        friend class Timescale;
        friend class RowLabels;
    };

};
