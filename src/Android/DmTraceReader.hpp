#pragma once

#include "Common.hpp"
#include "MethodData.hpp"
#include "ThreadData.hpp"
#include "Call.hpp"

#ifdef METHOD_ACTION_MASK
	#undef METHOD_ACTION_MASK
#endif
namespace Android {

	class DmTraceReader : public Object
	{
	public:
		typedef HashMap<String, String> PropertyMap;
		typedef HashMap<int64_t, MethodData*> MethodMap;
		typedef HashMap<int, ThreadData*> ThreadMap;
		typedef Vector<ThreadData*> ThreadList;
		typedef Vector<MethodData*> MethodList;

		typedef enum ClockSource {
			THREAD_CPU, WALL, DUAL, UNKNOWN,
		} ClockSource;

		typedef enum MethodAction {
			METHOD_TRACE_ENTER = 0,
			METHOD_TRACE_EXIT = 1,
			METHOD_TRACE_UNROLL = 2,
		} MethodAction;

	private:
		const int TRACE_MAGIC = 0x574F4C53;
		const int METHOD_ACTION_MASK = 0x3;
		const int METHOD_ID_MASK = ~METHOD_ACTION_MASK;

		const int64_t MIN_CONTEXT_SWITCH_TIME_USEC = 100L;
		int mVersionNumber;
		bool mRegression;
		/*ProfileProvider* mProfileProvider;*/

		String mTraceFileName;
		MethodData* mTopLevel;
		MethodData* mContextSwitch;

		Call::CallList mCallList;
		PropertyMap mPropertiesMap;
		MethodMap mMethodMap;
		ThreadMap mThreadMap;
		ThreadList* mSortedThreads;
		MethodList* mSortedMethods;

		uint32_t mTotalCpuTime;
		uint32_t mTotalRealTime;
		int mRecordSize;
		ClockSource mClockSource;

	public:
		enum {
			PARSE_VERSION = 0,
			PARSE_THREADS = 1,
			PARSE_METHODS = 2,
			PARSE_OPTIONS = 4,
		};

	public:
		void generateTrees() /* throws(IOException) */;

	public:
		//ProfileProvider* getProfileProvider();

	private:
		ByteBuffer* mapFile(const char* filename, int64_t offset);
		void readDataFileHeader(ByteBuffer* buffer);
		void parseData(int64_t offset) /* throws(IOException) */;

	public:
		int64_t parseKeys() /* throws(IOException) */;
		void parseOption(const String &line);
		void parseThread(const String &line);
		void parseMethod(const String &line);

	private:
		String constructPathname(const String &className, const String &pathname);
		void analyzeData();

	public:
		Call::CallList* getThreadTimeRecords() {
			return &mCallList;
		}
		//	::java::util::HashMap* getThreadLabels();
		//
		private:
			void dumpThreadTimes();
			void dumpCallTimes();
			void dumpMethodStats();
			void dumpTimeRecs();

	public:
		Vector<MethodData*>* getMethods();
		Vector<ThreadData*>* getThreads();
		int64_t getTotalCpuTime();
		int64_t getTotalRealTime();
		bool haveCpuTime();
		bool haveRealTime();
		PropertyMap& getProperties();
		TimeBase* getPreferredTimeBase();
		const char* getClockSource();

		// Generated
		DmTraceReader(const char* traceFileName, bool regression);
		~DmTraceReader();
	};
};
