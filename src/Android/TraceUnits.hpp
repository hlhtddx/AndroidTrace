#pragma once

#include "Common.hpp"

namespace Android {

	typedef enum TimeScale {
		Seconds, MilliSeconds, MicroSeconds
	} TimeScale;

	class TraceUnits : public Object
	{
	private:
		TimeScale mTimeScale;
		double mScale;

	public:
		double getScaledValue(uint32_t value);
		double getScaledValue(double value);
		const char* valueOf(uint32_t value, String& outString);
		const char* valueOf(double value, String& outString);
		const char* labelledString(double value, String& outString);
		const char* labelledString(uint32_t value, String& outString);
		const char* label();
		void setTimeScale(TimeScale val);
		TimeScale getTimeScale();

	public:
		TraceUnits();
	};

};
