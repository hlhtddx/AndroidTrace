#pragma once

#include "Common.hpp"
#include "MethodData.hpp"
#include "ThreadData.hpp"
#include "Call.hpp"

#ifdef METHOD_ACTION_MASK
	#undef METHOD_ACTION_MASK
#endif
namespace Android {

	typedef std::streampos filepos;
	typedef HashMap<String, String> PropertyMap;
	typedef HashMap<uint32_t, ThreadData*> ThreadPtrMap;
	typedef HashMap<uint32_t, MethodData*> MethodPtrMap;
	typedef Vector<ThreadData*> ThreadPtrList;
	typedef Vector<MethodData*> MethodPtrList;
	
	typedef enum MethodAction {
		METHOD_TRACE_ENTER = 0,
		METHOD_TRACE_EXIT = 1,
		METHOD_TRACE_UNROLL = 2,
	} MethodAction;

	typedef enum ClockSource {
		THREAD_CPU, WALL, DUAL, UNKNOWN,
	} ClockSource;

	class DmTraceReader : public Object
	{
	public:

	private:
		const int TRACE_MAGIC = 0x574F4C53;
		const uint32_t METHOD_ACTION_MASK = 0x3;
		const uint32_t METHOD_ID_MASK = ~METHOD_ACTION_MASK;

		const uint32_t MIN_CONTEXT_SWITCH_TIME_USEC = 100L;
		int mVersionNumber;
		bool mRegression;
		/*ProfileProvider* mProfileProvider;*/

		String mTraceFileName;
		MethodData* mTopLevel;
		MethodData* mContextSwitch;

		CallList mCallList;
		PropertyMap mPropertiesMap;
		MethodPtrMap mMethodMap;
		ThreadPtrMap mThreadMap;
		ThreadPtrList* mSortedThreads;
		MethodPtrList* mSortedMethods;

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
		ByteBuffer* mapFile(const char* filename, filepos offset);
		void readDataFileHeader(ByteBuffer* buffer);
		void parseData(filepos offset) /* throws(IOException) */;

	public:
		filepos parseKeys() /* throws(IOException) */;
		void parseOption(const String &line);
		void parseThread(const String &line);
		void parseMethod(const String &line);

	private:
		void constructPathname(String &className, String &pathname);
		void analyzeData();

	public:
		CallList* getThreadTimeRecords() {
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
		uint32_t getTotalCpuTime();
		uint32_t getTotalRealTime();
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
