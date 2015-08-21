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

	const char* TraceUnits::valueOf(uint32_t value, String& out_string)
	{
		return valueOf((double)value, out_string);
	}

	const char* TraceUnits::valueOf(double value, String& out_string)
	{
		double scaled = value * mScale;
		std::stringstream outs(out_string);

		outs << std::setiosflags(std::ios::fixed);

		outs.fill('0');

		if (static_cast<int32_t>(scaled) == scaled) {
			outs.width(6);
			outs.precision(0);
		}
		else {
			outs.width(9);
			outs.precision(3);
		}
		outs << value;
		return out_string.c_str();
	}

	const char* TraceUnits::labelledString(double value, String& out_string)
	{
		const char* units = label();
		String num;
		valueOf(value, num);
		std::stringstream outs(out_string);
		outs << units << ": " << num;
		return out_string.c_str();
	}

	const char* TraceUnits::labelledString(uint32_t value, String& out_string)
	{
		return labelledString(static_cast<double>(value), out_string);
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

	TraceUnits::TimeScale TraceUnits::getTimeScale()
	{
		return mTimeScale;
	}
};
