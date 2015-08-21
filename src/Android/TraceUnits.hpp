#pragma once

#include "Common.hpp"

namespace Android {

	class TraceUnits : public Object
	{
	public:
		typedef enum TimeScale {
			Seconds, MilliSeconds, MicroSeconds
		} TimeScale;
	private:
		TimeScale mTimeScale;
		double mScale;

	public:
		double getScaledValue(uint32_t value);
		double getScaledValue(double value);
		const char* valueOf(uint32_t value, String& out_string);
		const char* valueOf(double value, String& out_string);
		const char* labelledString(double value, String& out_string);
		const char* labelledString(uint32_t value, String& out_string);
		const char* label();
		void setTimeScale(TimeScale val);
		TimeScale getTimeScale();

		// Generated
		TraceUnits();
	};

};
