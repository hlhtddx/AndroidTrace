// Generated from /Traceview/src/com/android/traceview/TraceUnits.java
#include "TraceUnits.hpp"

namespace Android {

	TraceUnits::TraceUnits()
	{
		mTimeScale = MicroSeconds;
		mScale = 1.0;
	}

	double TraceUnits::getScaledValue(uint32_t value)
	{
		return value * mScale;
	}

	double TraceUnits::getScaledValue(double value)
	{
		return value * mScale;
	}

	const char* TraceUnits::valueOf(uint32_t value, String& outString)
	{
		return valueOf((double)value, outString);
	}

	const char* TraceUnits::valueOf(double value, String& outString)
	{
		double scaled = value * mScale;
		std::stringstream outs(outString);

		outs << std::setiosflags(std::ios::fixed);

		outs.fill('0');

		if (static_cast<int>(scaled) == scaled) {
			outs.width(6);
			outs.precision(0);
		}
		else {
			outs.width(9);
			outs.precision(3);
		}
		outs << value;
		return outString.c_str();
	}

	const char* TraceUnits::labelledString(double value, String& outString)
	{
		const char* units = label();
		String num;
		valueOf(value, num);
		std::stringstream outs(outString);
		outs << units << ": " << num;
		return outString.c_str();
	}

	const char* TraceUnits::labelledString(uint32_t value, String& outString)
	{
		return labelledString(static_cast<double>(value), outString);
	}

	const char* TraceUnits::label()
	{
		if (mScale == 1.0)
			return "usec";

		if (mScale == 0.001)
			return "msec";

		if (mScale == 1.0E-6)
			return "sec";

		return nullptr;
	}

	void TraceUnits::setTimeScale(TimeScale val)
	{
		switch(mTimeScale = val)
		{
		case Seconds:
			mScale = 1.0E-6;
			break;
		case MilliSeconds:
			mScale = 0.001;
			break;
		case MicroSeconds:
			mScale = 1.;
			break;
		default:
			throw - 1;
		}
	}

	TimeScale TraceUnits::getTimeScale()
	{
		return mTimeScale;
	}
};
