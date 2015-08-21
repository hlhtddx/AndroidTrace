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
		int32_t mY;
		uint32_t mColor;

		// Generated
		Range(int32_t xStart, int32_t width, int32_t y, uint32_t color);
	};

	class RowData : public Object
	{
	public:
		String mName;
		uint32_t mRank;
		uint32_t mElapsed;
		Vector<int> mStack;
		uint32_t mEndTime;

	public:
		virtual void push(int index);
		virtual int top();
		virtual void pop();

	public:
		RowData(ThreadData* row);
	};

	typedef struct Segment
	{
	public:
		RowData* mRowData;
		int64_t mStartTime;
		int64_t mEndTime;
		int mBlock;
		bool mIsContextSwitch;

	public:
		void init(RowData* rowData, Call::CallList* callList, int callIndex, int64_t startTime, int64_t endTime);
	} Segment;

	typedef FastArray<Segment> SegmentList;

	class Strip : public Object
	{
	public:
		int32_t mX;
		int32_t mY;
		int32_t mWidth;
		int32_t mHeight;
		RowData* mRowData;
		Segment* mSegment;
		uint32_t mColor;

	public:
		Strip(int32_t x, int32_t y, int32_t width, int32_t height, RowData* rowData, Segment* segment, uint32_t color);
	};

	class TickScaler : public Object
	{
	protected:
		double mMinVal;
		double mMaxVal;
		double mRangeVal;
		int32_t mNumPixels;
		int32_t mPixelsPerTick;
		double mPixelsPerRange;
		double mTickIncrement;
		double mMinMajorTick;

	public:

		TickScaler(double minVal, double maxVal, int32_t numPixels, int32_t pixelsPerTick)
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

		void setNumPixels(int32_t numPixels)
		{
			mNumPixels = numPixels;
		}

		int32_t getNumPixels()
		{
			return mNumPixels;
		}

		void setPixelsPerTick(int32_t pixelsPerTick)
		{
			mPixelsPerTick = pixelsPerTick;
		}

		int32_t getPixelsPerTick()
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

		int32_t valueToPixel(double value)
		{
			return (int32_t) ceil(mPixelsPerRange * (value - mMinVal) - 0.5);
		}

		double valueToPixelFraction(double value)
		{
			return mPixelsPerRange * (value - mMinVal);
		}

		double pixelToValue(int32_t pixel)
		{
			return mMinVal + pixel / mPixelsPerRange;
		}

		void computeTicks(bool useGivenEndPoints);

	};

	class Pixel
		: public Object
	{
	public:
		Pixel();
		void setFields(int32_t start, double weight, Segment* segment, uint32_t color, RowData* rowData);

	public:
		int32_t mStart;
		double mMaxWeight;
		Segment* mSegment;
		uint32_t mColor;
		RowData* mRowData;

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
		const int32_t TotalXMargin = 70;
		const int32_t yMargin = 1;
		const int32_t MinZoomPixelMargin = 10;

	protected:
		void initZoomFractionsWithExp();
		void initZoomFractionsWithSinWave();

	public:
		void setRange(double minVal, double maxVal);
		void setLimitRange(double minVal, double maxVal);
		void resetScale();
		void setScaleFromHorizontalScrollBar(int32_t selection);

	protected:
		void draw();
		void drawHighlights(Point dim);
		bool drawingSelection();
		void computeStrips();
		double computeWeight(double start, double end, bool isContextSwitch, int32_t pixel);
		void emitPixelStrip(RowData* rd, int32_t y, Pixel* pixel);
		virtual void mouseMove(Point& pt, int flags);
		virtual void mouseDown(Point& pt, int flags);
		virtual void mouseUp(Point& pt, int flags);
		virtual void mouseScrolled(Point& pt, int flags);
		virtual void mouseDoubleClick(Point& pt, int flags);

	public:
		virtual void startScaling(int32_t mouseX);
		virtual void stopScaling(int32_t mouseX);

	protected:
		GraphicsState mGraphicsState;
		Point mMouse;
		int32_t mMouseMarkStartX;
		int32_t mMouseMarkEndX;
		bool mDebug;
		Vector<Strip*> mStripList;
		Vector<Range> mHighlightExclusive;
		Vector<Range> mHighlightInclusive;
		int32_t mMinStripHeight;
		double mCachedMinVal;
		double mCachedMaxVal;
		int32_t mCachedStartRow;
		int32_t mCachedEndRow;
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

		int32_t mHighlightHeight;
		static int highlightHeights[];
		int32_t HIGHLIGHT_STEPS;
		bool mFadeColors;
		bool mShowHighlightName;
		double mZoomFractions[8];
		int32_t mZoomStep;
		int32_t mZoomMouseStart;
		int32_t mZoomMouseEnd;
		int32_t mMouseStartDistance;
		int32_t mMouseEndDistance;
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
		int32_t mHighlightStep;

	protected:		//Callbacks
		virtual void animateHighlight();
		virtual void clearHighlights();
		virtual void animateZoom();

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
		uint32_t mMethodColor;
		String mDetails;
		int32_t mMethodStartY;
		int32_t mDetailsStartY;
		int32_t mMarkStartX;
		int32_t mMarkEndX;
		const int32_t METHOD_BLOCK_MARGIN = 10;

	public:
		//void setVbarPosition(int32_t x);
		//void setMarkStart(int32_t x);
		//void setMarkEnd(int32_t x);
		//void setMethodName(String name);
		//void setMethodColor(uint32_t color);
		//void setDetails(String details);

	protected:
		//virtual void mouseMove(Point& pt, int flags);
		//virtual void mouseDown(Point& pt, int flags);
		//virtual void mouseUp(Point& pt, int flags);
		//virtual void mouseScrolled(Point& pt, int flags);
		//virtual void mouseDoubleClick(Point& pt, int flags);

		//void draw();
		//void drawSelection();
		//void drawTickLegend();
		//void drawMethod();
		//void drawDetails();
		//void drawTicks();
		TimeLineView *mParent;

	public:
		Timescale(TimeLineView *parent);
	};
	class RowLabels : public Canvas
	{
	protected:
		const int32_t labelMarginX = 2;

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
		HashMap<String, RowData*> mRowByName;
		Vector<RowData*>* mRows;
		SegmentList mSegments;
		HashMap<int, String> mThreadLabels;
		Timescale* mTimescale;
		Surface* mSurface;
		RowLabels* mLabels;
		int32_t mScrollOffsetY;

	public:
		const int32_t PixelsPerTick = 50;

	protected:
		Call::CallList* mCallList;
		TickScaler mScaleInfo;
		const int32_t LeftMargin = 10;
		const int32_t RightMargin = 60;
		const int rowHeight = 20;
		const int rowYMargin = 12;
		const int rowYMarginHalf = 6;
		const int rowYSpace = 32;
		const int majorTickLength = 8;
		const int minorTickLength = 4;
		const int timeLineOffsetY = 58;
		const int tickToFontSpacing = 2;
		const int topMargin = 90;
		int32_t mMouseRow;
		int32_t mNumRows;
		int32_t mStartRow;
		int32_t mEndRow;
		TraceUnits* mUnits;
		String mClockSource;
		bool mHaveCpuTime;
		bool mHaveRealTime;
		int32_t mSmallFontWidth;
		int32_t mSmallFontHeight;
		MethodData* mHighlightMethodData;
		Call* mHighlightCall;
		const int32_t MinInclusiveRange = 3;
		bool mSetFonts;

	public:
		Size mSize;

		Size getSize() const {
			return mSize;
		}
		void setData(Call::CallList* callList);
		TickScaler& getScaleInfo() {
			return mScaleInfo;
		}

	protected:
		static void popFrames(RowData* rd, Call::CallList* callList, Call* top, int64_t startTime, SegmentList* segmentList);
		int32_t computeVisibleRows(int32_t ydim);
		void startHighlighting();

	public:
		TimeLineView(DmTraceReader* reader/*, SelectionController* selectionController*/);

		friend class Surface;
		friend class Timescale;
		friend class RowLabels;
	};

};
