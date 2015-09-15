// Generated from /Traceview/src/com/android/traceview/DmTraceControl.java

#pragma once

#include "Common.hpp"
#include "Call.hpp"
#include "MethodData.hpp"
#include "ThreadData.hpp"
#include "DmTraceModel.hpp"
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
        Point()
        {
            x = y = 0;
        }
        Point(int px, int py)
        {
            x = px;
            y = py;
        }
    };

    struct Size
    {
        int cx;
        int cy;
        Size()
        {
            cx = cy = 0;
        }
        Size(int width, int height)
        {
            cx = width;
            cy = height;
        }
    };

    struct Rectangle
    {
        Point topleft;
        Size size;
        Rectangle() {};
        Rectangle(int x, int y, int width, int height)
            : topleft(x, y)
            , size(width, height)
        {
        }
    };

    struct Range
    {
        Point mXdim;
        int mY;
        COLOR mColor;
        Range(int xStart, int width, int y, COLOR color);
    };

    class TickScaler
    {
    protected:
        uint32_t mMinVal;           // Minimal value (in microsecond, timeline at the most left) of the visible range
        uint32_t mMaxVal;           // Maximal value (in microsecond, timeline at the most right) of the visible range
        int64_t mRangeVal;         // Visible range (in microsecond), mRangeVal = mMaxVal- mMinVal
        int mNumPixels;             // The number of pixels horizontal in visible range
        int mPixelsPerTick;         // The number of pixels within one tick, in px/tick
        uint32_t mTickIncrement;    // The scale unit, in us/tick. mTickIncrement = mRangeVal * mPixelsPerTick / mNumPixels
        uint32_t mMinMajorTick;     // The minimal major Tick which is greater than mMinVal, in microsecond.

    public:

        TickScaler(uint32_t minVal, uint32_t maxVal, int numPixels, int pixelsPerTick)
        {
            mMinVal = minVal;
            mMaxVal = maxVal;
            mRangeVal = mMaxVal - mMinVal;
            mNumPixels = numPixels;
            mPixelsPerTick = pixelsPerTick;
        }

        void setMinVal(uint32_t minVal)
        {
            mMinVal = minVal;
        }

        uint32_t getMinVal()
        {
            return mMinVal;
        }

        void setMaxVal(uint32_t maxVal)
        {
            mMaxVal = maxVal;
        }

        uint32_t getMaxVal()
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

        void setTickIncrement(uint32_t tickIncrement)
        {
            mTickIncrement = tickIncrement;
        }

        uint32_t getTickIncrement()
        {
            return mTickIncrement;
        }

        void setMinMajorTick(uint32_t minMajorTick)
        {
            mMinMajorTick = minMajorTick;
        }

        uint32_t getMinMajorTick()
        {
            return mMinMajorTick;
        }

        int valueToPixel(uint32_t value)
        {
            return (int)(((int64_t)value - mMinVal) * mNumPixels / mRangeVal);
        }

        uint32_t pixelToValue(int pixel)
        {
            return uint32_t(mMinVal + pixel * mRangeVal / mNumPixels);
        }

        /* Compute the scale ticks for following parameters:
             1. mMinVal, mMaxVal ==> The visible range of min/max timeline in us(If useGivenEndPoints is true, they are aligned with mTickIncrement)
                mRangeVal          = mMaxVal - mMinVal

             2. mNumPixels       ==> How many pixels is visible width range

             3. mTickIncrement   ==> Scaling unit (1, 2, 5 * 10^N) in us

             4. mMinMajorTick    ==> Ceil of mMinVal aligned with mTickIncrement
         */
        void computeTicks(bool useGivenEndPoints);
    };

    class Pixel
    {
    public:
        
        static const double qualifiedWeight;
        Pixel()
        {
            mStart = -2;
        }

        void setFields(int start, double weight, Call* call, COLOR color, ThreadData* thread);

    public:
        int mStart;
        double mMaxWeight;
        Call* mCall;
        COLOR mColor;
        ThreadData* mThread;

    protected:
        friend class DmTraceControl;
    };

    class DmTraceControl;

    class Canvas
    {
    public:
        Canvas() {
            mBoundRect.topleft.x = mBoundRect.topleft.y = 0;
            mBoundRect.size.cx = mBoundRect.size.cy = 0;
        }

        Canvas(Size& size)
        {
            mBoundRect.topleft.x = mBoundRect.topleft.y = 0;
            mBoundRect.size.cx = size.cx;
            mBoundRect.size.cy = size.cy;
        }

        Size getSize() const {
            return mBoundRect.size;
        }

        void getBoundary(Rectangle* rcBound) const
        {
            *rcBound = mBoundRect;
        }

    public:  //overrides
        virtual void draw(void* context) = 0;
        virtual void redraw() = 0;

        virtual void mouseMove(Point& pt, int flags) {};
        virtual void mouseDown(Point& pt, int flags) {};
        virtual void mouseUp(Point& pt, int flags) {};
        virtual void mouseScrolled(Point& pt, int flags) {};
        virtual void mouseDoubleClick(Point& pt, int flags) {};
        virtual void setBoundary(Rectangle* rcBound)
        {
            mBoundRect = *rcBound;
        }

    protected:
        Rectangle mBoundRect;
    };

    class TimeLineView : public Canvas
    {
    protected:
        const int TotalXMargin = 70;
        const int yMargin = 1;
        const int MinZoomPixelMargin = 10;

    protected:
        GraphicsState mGraphicsState;
        Point mMouse;
        int mMouseMarkStartX;
        int mMouseMarkEndX;
        bool mDebug;
        int mMinStripHeight;
        double mCachedMinVal;
        double mCachedMaxVal;
        int mCachedStartRow;
        int mCachedEndRow;
        double mScalePixelsPerRange;
        uint32_t mScaleMinVal;
        uint32_t mScaleMaxVal;
        uint32_t mLimitMinVal;
        uint32_t mLimitMaxVal;
        uint32_t mMinDataVal;
        uint32_t mMaxDataVal;
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

    protected:
        void initZoomFractionsWithExp();
        void initZoomFractionsWithSinWave();

    public:
        void setRange(uint32_t minVal, uint32_t maxVal);
        void setLimitRange(uint32_t minVal, uint32_t maxVal);
        void resetScale();
        void setScaleFromHorizontalScrollBar(int selection);

    protected:
        void drawHighlights(Point dim);
        bool drawingSelection();

    public:  //overrides
        virtual void draw(void* context);
        virtual void mouseMove(Point& pt, int flags);
        virtual void mouseDown(Point& pt, int flags);
        virtual void mouseUp(Point& pt, int flags);
        virtual void mouseScrolled(Point& pt, int flags);
        virtual void mouseDoubleClick(Point& pt, int flags);
        virtual void setBoundary(Rectangle* rcBound);

    public:
        void startScaling(int mouseX);
        void stopScaling(int mouseX);

    protected:		//Callbacks
        void animateHighlight();
        void clearHighlights();
        void animateZoom();

    public:
        TimeLineView(DmTraceControl *parent);
        DmTraceControl *mParent;

    public:
        friend class DmTraceControl;
    };

    class TimeScaleView : public Canvas
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

    public:  //overrides
        virtual void draw(void* context) = 0;
        virtual void setBoundary(Rectangle* rcBound);

    protected:
        DmTraceControl *mParent;

    public:
        TimeScaleView(DmTraceControl *parent);

    public:
        friend class DmTraceControl;
    };

    class ThreadLabelView : public Canvas
    {
    protected:
        const int labelMarginX = 2;

    public:
        virtual void draw(void* context) = 0;
        virtual void setBoundary(Rectangle* rcBound);
        void mouseMove(Point& pt, int flags);

    public:
        ThreadLabelView(DmTraceControl *parent);
        DmTraceControl *mParent;

    public:
        friend class DmTraceControl;
    };

    class DmTraceControl : public Canvas
    {
    protected:
        TimeScaleView* mTimescale;
        TimeLineView* mTimeLine;
        ThreadLabelView* mThreadLabel;
        int mScrollOffsetY;

    public:
        static const int PixelsPerTick = 50;
        static const int LeftMargin = 10;
        static const int RightMargin = 60;
        static const int rowHeight = 20;
        static const int rowYMargin = 12;
        static const int rowYMarginHalf = 6;
        static const int rowYSpace = 32;
        static const int majorTickLength = 8;
        static const int minorTickLength = 4;
        static const int timeLineOffsetY = 58;
        static const int tickToFontSpacing = 2;
        static const int topMargin = 90;

        static const int MinInclusiveRange = 3;

        DmTraceModel* mDmTraceData;
        TickScaler mScaleInfo;

        int mMouseRow;
        int mNumRows;
        int mStartRow;
        int mEndRow;

        TraceUnits* mUnits;
        String mClockSource;
        Vector<Range> mHighlightExclusive;
        Vector<Range> mHighlightInclusive;

        bool mHaveCpuTime;
        bool mHaveRealTime;

        MethodData* mHighlightMethodData;
        Call* mHighlightCall;

    public:

        TickScaler& getScaleInfo() {
            return mScaleInfo;
        }

        void computeStrips();
        int computeVisibleRows(int ydim);

        ThreadPtrList* getThreads() {
            return mDmTraceData->getThreads();
        }

    public: //overrides
        virtual void draw(void* context) {};
        virtual void setBoundary(Rectangle* rcBound);

    protected:
        void startHighlighting();
        void createStrip(uint32_t recordStart, uint32_t recordEnd, uint32_t baseline, Call * call, bool isContextSwitch, Pixel & pixel);
        void popFrames(CallStack & stack, CallList * callList, Call * top, uint32_t startTime, Pixel& pixel, FastArray<Strip>* stripList);
        void dumpStrips();

    public:
        DmTraceControl();
        void setData(DmTraceModel* traceData);

        friend class TimeLineView;
        friend class TimeScaleView;
        friend class ThreadLabelView;
    };

};
