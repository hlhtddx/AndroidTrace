/*
* Copyright (C) 2006 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <stdint.h>
#include <memory>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <set>

/* arbitrarily limit indentation */
#define MAX_STACK_DEPTH 10000

/* thread list in key file is not reliable, so just max out */
#define MAX_THREADS 32768

#define GRAPH_LABEL_VISITED 0x0001
#define GRAPH_NODE_VISITED 0x0002


/*
* Enumeration for the two "action" bits.
*/
enum {
	METHOD_TRACE_ENTER = 0x00, // method entry
	METHOD_TRACE_EXIT = 0x01, // method exit
	METHOD_TRACE_UNROLL = 0x02, // method exited by exception unrolling
	// 0x03 currently unused
};

#define TOKEN_CHAR '*'

#define METHOD_ACTION_MASK 0x03 /* two bits */
#define METHOD_ID(_method) ((_method) & (~METHOD_ACTION_MASK))
#define METHOD_ACTION(_method) (((unsigned int)(_method)) & METHOD_ACTION_MASK)
#define METHOD_COMBINE(_method, _action) ((_method) | (_action))

namespace Android {
	/*
	* Values from the header of the data file.
	*/
	typedef struct DataHeader {
		unsigned int magic;
		short version;
		short offsetToData;
		int64_t startWhen;
		short recordSize;
	} DataHeader;

	/*
	* Entry from the thread list.
	*/
	class ThreadEntry {
	public:
		int threadId;
		int index;
		const char* threadName;

		bool operator == (const ThreadEntry& obj1) {
			return threadId == obj1.threadId;
		}

		bool operator < (const ThreadEntry& obj1) {
			return threadId < obj1.threadId;
		}
	};

	typedef std::map<unsigned int, ThreadEntry*> ThreadMap;

	class MethodEntry;
	typedef std::vector<MethodEntry*> MethodList;
	typedef std::map<uint64_t, MethodEntry*> MethodMap;

	class TimedMethod {
	public:
		TimedMethod *next;
		uint64_t elapsedInclusive;
		int numCalls;
		MethodEntry *method;

		void Free()
		{
			TimedMethod *PCurr;
			TimedMethod *pNext;
			for (PCurr = this; PCurr != NULL; PCurr = pNext) {
				pNext = PCurr->next;
				delete PCurr;
			}
		}
	};

	class ClassEntry {
	public:
		const char* className;
		uint64_t elapsedExclusive;
		int umMethods;
		MethodList methods; /* list of methods in this class */
		int numCalls[2]; /* 0=normal, 1=recursive */

		bool operator == (const ClassEntry& obj1) {
			return className == obj1.className;
		}

		bool operator < (const ClassEntry& obj1) {
			return className < obj1.className;
		}
	};

	class UniqueMethodEntry {
	public:
		uint64_t elapsedExclusive;
		int numMethods;
		MethodList methods; /* list of methods with same name */
		int numCalls[2]; /* 0=normal, 1=recursive */
	};

	/*
	* Entry from the method list.
	*/
	class MethodEntry {
	public:
		int64_t methodId;
		const char* className;
		const char* methodName;
		const char* signature;
		const char* fileName;
		int lineNum;
		uint64_t elapsedExclusive;
		uint64_t elapsedInclusive;
		uint64_t topExclusive; /* non-recursive exclusive time */
		uint64_t recursiveInclusive;
		TimedMethod *parents[2]; /* 0=normal, 1=recursive */
		TimedMethod *children[2]; /* 0=normal, 1=recursive */
		int numCalls[2]; /* 0=normal, 1=recursive */
		int index; /* used after sorting to number methods */
		int recursiveEntries; /* number of entries on the stack */
		int graphState; /* used when graphing to see if this method has been visited before */

		/* Initializes a MethodEntry
		*/
		MethodEntry() {
			InitMethodEntry(0, "Bogus", "", "", "", NULL);
		}
		/* Initializes a MethodEntry
		*/
		MethodEntry(int64_t methodId,
			const char *className, const char *methodName,
			const char *signature, const char* fileName,
			const char* lineNumStr) {
			InitMethodEntry(methodId, className, methodName,
				signature, fileName, lineNumStr);
		}

		~MethodEntry() {
			parents[0]->Free();
			parents[1]->Free();
			children[0]->Free();
			children[1]->Free();
		}

		void InitMethodEntry(int64_t methodId,
			const char *className, const char *methodName,
			const char *signature, const char* fileName,
			const char* lineNumStr) {
			this->methodId = methodId;
			this->className = className;
			this->methodName = methodName;
			this->signature = signature;
			this->fileName = fileName;
			this->lineNum = (lineNumStr != NULL) ? atoi(lineNumStr) : -1;
			this->elapsedExclusive = 0;
			this->elapsedInclusive = 0;
			this->topExclusive = 0;
			this->recursiveInclusive = 0;
			this->parents[0] = NULL;
			this->parents[1] = NULL;
			this->children[0] = NULL;
			this->children[1] = NULL;
			this->numCalls[0] = 0;
			this->numCalls[1] = 0;
			this->index = 0;
			this->recursiveEntries = 0;
		}
	};

	/*
	* The parsed contents of the key file.
	*/
	class DataKeys {
	public:
		/* Version number in the key file.
		* Version 1 uses one byte for the thread id.
		* Version 2 uses two bytes for the thread ids.
		* Version 3 encodes the record size and adds an optional extra timestamp field.
		*/
		int				versionNumber;
		char*			fileData; /* contents of the entire file */
		long			fileLen;
		long			headerLen;
		int				numThreads;
		ThreadEntry*	threads;
		int				numMethods;
		MethodEntry*	methods; /* 2 extra methods: "toplevel" and "unknown" */
		MethodMap		methodMap;

		DataKeys()
		{
			fileData = NULL;
			fileLen = headerLen = 0;
			numThreads = 0;
			threads = NULL;
			numMethods = 0;
			methods = NULL;
		}

		~DataKeys()
		{
			if (fileData) {
				free(fileData);
			}

			if (threads) {
				delete [] threads;
			}

			if (methods) {
				delete[] methods;
			}
		}

		/*
		* Find the offset to the next occurrence of the specified character.
		*
		* "data" should point somewhere within the current line. "len" is the
		* number of bytes left in the buffer.
		*
		* Returns -1 if we hit the end of the buffer.
		*/
		int findNextChar(const char* data, int len, char lookFor);

		/*
		* Count the number of lines until the next token.
		*
		* Returns -1 if none found before EOF.
		*/
		int countLinesToToken(const char* data, int len);

		/*
		* Parse the key section, copy the parsed contents.
		*/
		bool parseKeys(FILE *fp, int verbose);

		/*
		* Make sure we're at the start of the right section.
		*
		* Returns the length of the token line, or -1 if something is wrong.
		*/
		int checkToken(const char* data, int len, const char* cmpStr);

		/*
		* Parse the "*version" section.
		*/
		long parseVersion(long offset, int verbose);

		/*
		* Parse the "*threads" section.
		*/
		long parseThreads(long offset);

		/*
		* Parse the "*methods" section.
		*/
		long parseMethods(long offset);

		/*
		* Parse the "*end" section.
		*/
		long parseEnd(long offset);

		/*
		* Look up a method by it's method ID.
		*
		* Returns NULL if no matching method was found.
		*/
		MethodEntry* lookupMethod(int64_t methodId)
		{
			if (methodMap.find(methodId) == methodMap.end()) {
				return NULL;
			}
			return methodMap[methodId];
		}

		/*
		* Look up a thread by it's thread ID.
		*
		* Returns NULL if no matching thread was found.
		*/
		ThreadEntry* lookupThread(int threadId)
		{
			for (int index = 0; index < numThreads; index++) {
				if (threadId == threads[index].threadId)
					return &(threads[index]);
			}
			return NULL;
		}
	};

#define TOPLEVEL_INDEX 0
#define UNKNOWN_INDEX 1

	typedef struct CallEntry {
		ThreadEntry*	thread;
		MethodEntry*	method;
		unsigned int	action;
		uint64_t		startTime;
		uint64_t		endTime;
		uint64_t		inclusionTime;
		uint64_t		exclusionTime;
		CallEntry*		parent;
		bool			hasChildren;

		CallEntry(ThreadEntry* thread, MethodEntry* method, unsigned action,
			uint64_t startTime, CallEntry* parent) {
			this->thread = thread;
			this->method = method;
			this->action = action;
			this->startTime = startTime;
			this->endTime = 0;
			this->inclusionTime = this->exclusionTime = 0;
			this->parent = parent;
			hasChildren = false;
		}

	} CallEntry;

	typedef std::vector<CallEntry*> CallEntryList;

	class StackEntry {
	public:
		MethodEntry *method;
		CallEntry	*callEntry;
	};

	class CallStack {
	public:
		ThreadEntry* thread;
		int maxStackDepth;
		int top;
		StackEntry calls[MAX_STACK_DEPTH];
		uint64_t lastEventTime;
		uint64_t threadStartTime;
	};

	class TraceData {
	public:
		CallStack *stacks[MAX_THREADS];
		int totalDepth;

		int numClasses;
		ClassEntry *classes;
		int numUniqueMethods;
		UniqueMethodEntry *uniqueMethods;

		TraceData() {
			totalDepth = 0;
			numClasses = 0;
			classes = NULL;
			numUniqueMethods = 0;
			uniqueMethods = NULL;
			memset(stacks, 0, sizeof(stacks));
		}

		~TraceData() {
			for (int ii = 0; ii < MAX_THREADS; ii++) {
				if (stacks[ii]) {
					delete stacks[ii];
				}
			}
		}
	};

	class TraceFile
	{
	public:
		TraceFile() {
			m_dataKeys = NULL;
			memset(&m_dataHeader, 0, sizeof(m_dataHeader));
		}

		~TraceFile() {
			if (m_dataKeys)
			{
				delete m_dataKeys;
			}
			
			for (CallEntryList::iterator it = m_callEntries.begin(); it != m_callEntries.end(); it++) {
				CallEntry* callEntry = *it;
				delete callEntry;
			}
		}

	public:
		DataKeys*		m_dataKeys;
		DataHeader		m_dataHeader;
		CallEntryList	m_callEntries;
		TraceData		m_traceData;
		uint64_t		m_threadTime;
		uint64_t		m_startTime;
		uint64_t		m_lastTime;

		/*
		* Sort the thread list entries.
		*/
		static int compareThreads(const void* thread1, const void* thread2);

		/*
		* Sort the method list entries.
		*/
		static int compareMethods(const void* meth1, const void* meth2);

		/*
		* Parse the key section, and return a copy of the parsed contents.
		*/
		bool parseKeys(FILE *fp, int verbose);

		/*
		* Parse the header of the data section.
		*
		* Returns with the file positioned at the start of the record data.
		*/
		int parseDataHeader(FILE *fp, DataHeader* pHeader);

		/*
		* Reads the next data record, and assigns the data values to threadId,
		* methodVal and elapsedTime. On end-of-file, the threadId, methodVal,
		* and elapsedTime are unchanged. Returns 1 on end-of-file, otherwise
		* returns 0.
		*/
		int readDataRecord(FILE *dataFp, DataHeader* dataHeader,
			int *threadId, unsigned int *methodVal, uint64_t *elapsedTime);

		/* Make the return value "unsigned int" instead of "unsigned short" so that
		* we can detect EOF.
		*/
		unsigned int read2LE(FILE* fp)
		{
			unsigned int val;

			val = getc(fp);
			val |= getc(fp) << 8;
			return val;
		}
		unsigned int read4LE(FILE* fp)
		{
			unsigned int val;

			val = getc(fp);
			val |= getc(fp) << 8;
			val |= getc(fp) << 16;
			val |= getc(fp) << 24;
			return val;
		}
		unsigned long long read8LE(FILE* fp)
		{
			unsigned long long val;

			val = getc(fp);
			val |= (unsigned long long) getc(fp) << 8;
			val |= (unsigned long long) getc(fp) << 16;
			val |= (unsigned long long) getc(fp) << 24;
			val |= (unsigned long long) getc(fp) << 32;
			val |= (unsigned long long) getc(fp) << 40;
			val |= (unsigned long long) getc(fp) << 48;
			val |= (unsigned long long) getc(fp) << 56;
			return val;
		}

		/* This routine adds the given time to the parent and child methods.
		* This is called when the child routine exits, after the child has
		* been popped from the stack. The elapsedTime parameter is the
		* duration of the child routine, including time spent in called routines.
		*/
		void addInclusiveTime(MethodEntry *parent, MethodEntry *child,
			uint64_t elapsedTime);
		void countRecursiveEntries(CallStack *pStack, int top, MethodEntry *method);

	public:
		/*
		* Read the key and data files and return the MethodEntries for those files
		*/
		bool parseDataKeys(const char* traceFileName);
	};

};