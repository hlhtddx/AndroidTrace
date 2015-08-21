/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef WIN32 
	#include "../stdafx.h"

	#ifdef _DEBUG
		#define new DEBUG_NEW
	#endif
#endif

#ifdef DEBUG_OUTPUT
	#ifdef WIN32 
		#define DEBUG_TRACE TRACE
	#else
		#define DEBUG_TRACE printf
	#endif
#else
	#define DEBUG_TRACE
#endif

#include "TraceFile.h"
#include <assert.h>

namespace Android {

	static const char* szMethodActions[] =
	{
		"METHOD_TRACE_ENTER",
		"METHOD_TRACE_EXIT",
		"METHOD_TRACE_UNROLL",
		"UNKNOWN",
	};

	/*
	* Find the offset to the next occurrence of the specified character.
	*
	* "data" should point somewhere within the current line.  "len" is the
	* number of bytes left in the buffer.
	*
	* Returns -1 if we hit the end of the buffer.
	*/
	int DataKeys::findNextChar(const char* data, int len, char lookFor)
	{
		const char* start = data;

		while (len > 0) {
			if (*data == lookFor)
				return data - start;

			data++;
			len--;
		}

		return -1;
	}

	/*
	* Count the number of lines until the next token.
	*
	* Returns -1 if none found before EOF.
	*/
	int DataKeys::countLinesToToken(const char* data, int len)
	{
		int count = 0;
		int next;

		while (*data != TOKEN_CHAR) {
			next = findNextChar(data, len, '\n');
			if (next < 0)
				return -1;
			count++;
			data += next + 1;
			len -= next + 1;
		}

		return count;
	}

	/*
	* Make sure we're at the start of the right section.
	*
	* Returns the length of the token line, or -1 if something is wrong.
	*/
	int DataKeys::checkToken(const char* data, int len, const char* cmpStr)
	{
		int cmpLen = strlen(cmpStr);
		int next;

		if (*data != TOKEN_CHAR) {
			DEBUG_TRACE(
				"ERROR: not at start of %s (found '%.10s')\n", cmpStr, data);
			return -1;
		}

		next = findNextChar(data, len, '\n');
		if (next < cmpLen + 1)
			return -1;

		if (strncmp(data + 1, cmpStr, cmpLen) != 0) {
			DEBUG_TRACE( "ERROR: '%s' not found (got '%.7s')\n", cmpStr, data + 1);
			return -1;
		}

		return next + 1;
	}

	/*
	* Parse the key section, copy the parsed contents.
	*/
	bool DataKeys::parseKeys(FILE *fp, int verbose)
	{
		long offset;
		int i;

		/*
		* We load the entire file into memory.  We do this, rather than memory-
		* mapping it, because we want to change some whitespace to NULs.
		*/
		if (fseek(fp, 0L, SEEK_END) != 0) {
			perror("fseek");
			return false;
		}
		fileLen = ftell(fp);
		if (fileLen == 0) {
			DEBUG_TRACE("Key file is empty.\n");
			return false;
		}
		rewind(fp);

		fileData = (char*)malloc(fileLen);
		if (fileData == NULL) {
			DEBUG_TRACE("ERROR: unable to alloc %ld bytes\n", fileLen);
			return false;
		}

		if (fread(fileData, 1, fileLen, fp) != (size_t)fileLen)
		{
			DEBUG_TRACE("ERROR: unable to read %ld bytes from trace file\n",
				fileLen);
			return false;
		}

		offset = 0;

		offset = parseVersion(offset, verbose);
		offset = parseThreads(offset);
		offset = parseMethods(offset);
		offset = parseEnd(offset);
		if (offset < 0)
			return false;

		/* Reduce our allocation now that we know where the end of the key section is. */
		fileData = (char *)realloc(fileData, offset);
		headerLen = offset;
		/* Leave fp pointing to the beginning of the data section. */
		fseek(fp, offset, SEEK_SET);

		//		sortThreadList(pKeys);
		//		sortMethodList(pKeys);

		/*
		* Dump list of threads.
		*/
		if (verbose) {
			DEBUG_TRACE("Threads (%d):\n", numThreads);
			for (i = 0; i < numThreads; i++) {
				DEBUG_TRACE("%2d %s\n",
					threads[i].threadId, threads[i].threadName);
			}
		}

		return true;
	}

	/*
	* Parse the "*version" section.
	*/
	long DataKeys::parseVersion(long offset, int verbose)
	{
		char* data;
		char* dataEnd;
		int i, count, next;

		if (offset < 0)
			return -1;

		data = fileData + offset;
		dataEnd = fileData + fileLen;
		next = checkToken(data, dataEnd - data, "version");
		if (next <= 0)
			return -1;

		data += next;

		/*
		* Count the number of items in the "version" section.
		*/
		count = countLinesToToken(data, dataEnd - data);
		if (count <= 0) {
			DEBUG_TRACE(
				"ERROR: failed while reading version (found %d)\n", count);
			return -1;
		}

		/* find the end of the line */
		next = findNextChar(data, dataEnd - data, '\n');
		if (next < 0)
			return -1;

		data[next] = '\0';
		versionNumber = strtoul(data, NULL, 0);
		DEBUG_TRACE("VERSION: %d\n", versionNumber);

		data += next + 1;

		/* skip over the rest of the stuff, which is "name=value" lines */
		for (i = 1; i < count; i++) {
			next = findNextChar(data, dataEnd - data, '\n');
			if (next < 0)
				return -1;
			//data[next] = '\0';
			//DEBUG_TRACE("IGNORING: '%s'\n", data);
			data += next + 1;
		}

		return data - fileData;
	}

	/*
	* Parse the "*threads" section.
	*/
	long DataKeys::parseThreads(long offset)
	{
		char* data;
		char* dataEnd;
		int i, next, tab, count;

		if (offset < 0)
			return -1;

		data = fileData + offset;
		dataEnd = fileData + fileLen;
		next = checkToken(data, dataEnd - data, "threads");

		data += next;

		/*
		* Count the number of thread entries (one per line).
		*/
		count = countLinesToToken(data, dataEnd - data);
		if (count <= 0) {
			DEBUG_TRACE(
				"ERROR: failed while reading threads (found %d)\n", count);
			return -1;
		}

		DEBUG_TRACE("+++ found %d threads\n", count);
		numThreads = count;
		threads = new ThreadEntry[count];

		/*
		* Extract all entries.
		*/
		for (i = 0; i < count; i++) {
			next = findNextChar(data, dataEnd - data, '\n');
			assert(next > 0);
			data[next] = '\0';

			tab = findNextChar(data, next, '\t');
			data[tab] = '\0';

			threads[i].index = i;
			threads[i].threadId = atoi(data);
			threads[i].threadName = data + tab + 1;

			DEBUG_TRACE("THREAD: %d - %s\n", threads[i].threadId, threads[i].threadName);

			data += next + 1;
		}

		return data - fileData;
	}

	/*
	* Parse the "*methods" section.
	*/
	long DataKeys::parseMethods(long offset)
	{
		char* data;
		char* dataEnd;
		int i, next, count;

		if (offset < 0)
			return -1;

		data = fileData + offset;
		dataEnd = fileData + fileLen;
		next = checkToken(data, dataEnd - data, "methods");
		if (next < 0)
			return -1;

		data += next;

		/*
		* Count the number of method entries (one per line).
		*/
		count = countLinesToToken(data, dataEnd - data);
		if (count <= 0) {
			DEBUG_TRACE(
				"ERROR: failed while reading methods (found %d)\n", count);
			return -1;
		}

		count += 2;
		methods = new MethodEntry[count];

		/* Reserve an extra method at location 0 for the "toplevel" method,
		* and another extra method for all other "unknown" methods.
		*/
		methods[TOPLEVEL_INDEX].InitMethodEntry(-2, "(toplevel)", "", "", "", "");
		methods[UNKNOWN_INDEX].InitMethodEntry(-1, "(unknown)", "", "", "", "");

		/*
		* Extract all entries, starting with index 2.
		*/
		for (i = UNKNOWN_INDEX + 1; i < count; i++) {
			int tab1, tab2, tab3, tab4, tab5;
			int64_t id;
			char* endptr;
			MethodEntry* method = NULL;

			next = findNextChar(data, dataEnd - data, '\n');
			assert(next > 0);
			data[next] = '\0';

			tab1 = findNextChar(data, next, '\t');
			tab2 = findNextChar(data + (tab1 + 1), next - (tab1 + 1), '\t');
			tab3 = findNextChar(data + (tab1 + tab2 + 2), next - (tab1 + tab2 + 2), '\t');
			tab4 = findNextChar(data + (tab1 + tab2 + tab3 + 3),
				next - (tab1 + tab2 + tab3 + 3), '\t');
			tab5 = findNextChar(data + (tab1 + tab2 + tab3 + tab4 + 4),
				next - (tab1 + tab2 + tab3 + tab4 + 4), '\t');
			if (tab1 < 0) {
				DEBUG_TRACE( "ERROR: missing field on method line: '%s'\n",
					data);
				return -1;
			}
			assert(data[tab1] == '\t');
			data[tab1] = '\0';

			id = strtoul(data, &endptr, 0);
			if (*endptr != '\0') {
				DEBUG_TRACE( "ERROR: bad method ID '%s'\n", data);
				return -1;
			}

			// Allow files that specify just a function name, instead of requiring
			// "class \t method \t signature"
			if (tab2 > 0 && tab3 > 0) {
				tab2 += tab1 + 1;
				tab3 += tab2 + 1;
				assert(data[tab2] == '\t');
				assert(data[tab3] == '\t');
				data[tab2] = data[tab3] = '\0';

				// This is starting to get awkward.  Allow filename and line #.
				if (tab4 > 0 && tab5 > 0) {
					tab4 += tab3 + 1;
					tab5 += tab4 + 1;

					assert(data[tab4] == '\t');
					assert(data[tab5] == '\t');
					data[tab4] = data[tab5] = '\0';

					methods[i].InitMethodEntry(id, data + tab1 + 1,
						data + tab2 + 1, data + tab3 + 1, data + tab4 + 1,
						data + tab5 + 1);
				}
				else {
					methods[i].InitMethodEntry(id, data + tab1 + 1,
						data + tab2 + 1, data + tab3 + 1, "", NULL);
				}
			}
			else {
				methods[i].InitMethodEntry(id, data + tab1 + 1,
					"", "", "", "");
			}

			methodMap[id] = &methods[i];
			DEBUG_TRACE("Method: id=%d, class=%s, methodname=%s, file=%s(%d)\n", (int)methods[i].methodId,
				methods[i].className, methods[i].methodName, methods[i].fileName, methods[i].lineNum);
			data += next + 1;
		}

		return data - fileData;
	}

	/*
	* Parse the "*end" section.
	*/
	long DataKeys::parseEnd(long offset)
	{
		char* data;
		char* dataEnd;
		int next;

		if (offset < 0)
			return -1;

		data = fileData + offset;
		dataEnd = fileData + fileLen;
		next = checkToken(data, dataEnd - data, "end");
		if (next < 0)
			return -1;

		data += next;

		return data - fileData;
	}

	/*
	* Sort the thread list entries.
	*/
	int TraceFile::compareThreads(const void* thread1, const void* thread2)
	{
		return ((const ThreadEntry*)thread1)->threadId -
			((const ThreadEntry*)thread2)->threadId;
	}

	/*
	* Sort the method list entries.
	*/
	int TraceFile::compareMethods(const void* meth1, const void* meth2)
	{
		int64_t id1, id2;

		id1 = ((const MethodEntry*)meth1)->methodId;
		id2 = ((const MethodEntry*)meth2)->methodId;
		if (id1 < id2)
			return -1;
		if (id1 > id2)
			return 1;
		return 0;
	}

	/*
	* Parse the key section, and return a copy of the parsed contents.
	*/
	bool TraceFile::parseKeys(FILE *fp, int verbose)
	{
		if (m_dataKeys)
			return false;

		DataKeys* pKeys = new DataKeys;
		if (pKeys == NULL)
			return false;

		if (!pKeys->parseKeys(fp, verbose)) {
			delete pKeys;
			return false;
		}

		m_dataKeys = pKeys;
		return true;
	}

	/*
	* Parse the header of the data section.
	*
	* Returns with the file positioned at the start of the record data.
	*/
	int TraceFile::parseDataHeader(FILE *fp, DataHeader* pHeader)
	{
		int bytesToRead;

		pHeader->magic = read4LE(fp);
		pHeader->version = read2LE(fp);
		pHeader->offsetToData = read2LE(fp);
		pHeader->startWhen = read8LE(fp);
		bytesToRead = pHeader->offsetToData - 16;
		if (pHeader->version == 1) {
			pHeader->recordSize = 9;
		}
		else if (pHeader->version == 2) {
			pHeader->recordSize = 10;
		}
		else if (pHeader->version == 3) {
			pHeader->recordSize = read2LE(fp);
			bytesToRead -= 2;
		}
		else {
			DEBUG_TRACE( "Unsupported trace file version: %d\n", pHeader->version);
			return -1;
		}

		if (fseek(fp, bytesToRead, SEEK_CUR) != 0) {
			return -1;
		}

		return 0;
	}

	/*
	* Reads the next data record, and assigns the data values to threadId,
	* methodVal and elapsedTime.  On end-of-file, the threadId, methodVal,
	* and elapsedTime are unchanged.  Returns 1 on end-of-file, otherwise
	* returns 0.
	*/
	int TraceFile::readDataRecord(FILE *dataFp, DataHeader* dataHeader,
		int *threadId, unsigned int *methodVal, uint64_t *elapsedTime)
	{
		int id;
		int bytesToRead;

		bytesToRead = dataHeader->recordSize;
		if (dataHeader->version == 1) {
			id = getc(dataFp);
			bytesToRead -= 1;
		}
		else {
			id = read2LE(dataFp);
			bytesToRead -= 2;
		}
		if (id == EOF)
			return 1;
		*threadId = id;

		*methodVal = read4LE(dataFp);
		*elapsedTime = read4LE(dataFp);
		bytesToRead -= 8;

		while (bytesToRead-- > 0) {
			getc(dataFp);
		}

		if (feof(dataFp)) {
			DEBUG_TRACE( "WARNING: hit EOF mid-record\n");
			return 1;
		}
		return 0;
	}


	/* This routine adds the given time to the parent and child methods.
	* This is called when the child routine exits, after the child has
	* been popped from the stack.  The elapsedTime parameter is the
	* duration of the child routine, including time spent in called routines.
	*/
	void TraceFile::addInclusiveTime(MethodEntry *parent, MethodEntry *child,
		uint64_t elapsedTime)
	{
		TimedMethod *pTimed;

		int childIsRecursive = (child->recursiveEntries > 0);
		int parentIsRecursive = (parent->recursiveEntries > 1);

		if (child->recursiveEntries == 0) {
			child->elapsedInclusive += elapsedTime;
		}
		else if (child->recursiveEntries == 1) {
			child->recursiveInclusive += elapsedTime;
		}
		child->numCalls[childIsRecursive] += 1;

		/* Find the child method in the parent */
		TimedMethod *children = parent->children[parentIsRecursive];
		for (pTimed = children; pTimed; pTimed = pTimed->next) {
			if (pTimed->method == child) {
				pTimed->elapsedInclusive += elapsedTime;
				pTimed->numCalls += 1;
				break;
			}
		}
		if (pTimed == NULL) {
			/* Allocate a new TimedMethod */
			pTimed = new TimedMethod;
			pTimed->elapsedInclusive = elapsedTime;
			pTimed->numCalls = 1;
			pTimed->method = child;

			/* Add it to the front of the list */
			pTimed->next = children;
			parent->children[parentIsRecursive] = pTimed;
		}

		/* Find the parent method in the child */
		TimedMethod *parents = child->parents[childIsRecursive];
		for (pTimed = parents; pTimed; pTimed = pTimed->next) {
			if (pTimed->method == parent) {
				pTimed->elapsedInclusive += elapsedTime;
				pTimed->numCalls += 1;
				break;
			}
		}
		if (pTimed == NULL) {
			/* Allocate a new TimedMethod */
			pTimed = new TimedMethod;
			pTimed->elapsedInclusive = elapsedTime;
			pTimed->numCalls = 1;
			pTimed->method = parent;

			/* Add it to the front of the list */
			pTimed->next = parents;
			child->parents[childIsRecursive] = pTimed;
		}
	}

	void TraceFile::countRecursiveEntries(CallStack *pStack, int top, MethodEntry *method)
	{
		int ii;

		method->recursiveEntries = 0;
		for (ii = 0; ii < top; ++ii) {
			if (pStack->calls[ii].method == method)
				method->recursiveEntries += 1;
		}
	}

	/*
	* Read the key and data files and return the MethodEntries for those files
	*/
	bool TraceFile::parseDataKeys(const char* traceFileName)
	{
		MethodEntry* method;
		FILE* dataFp = NULL;
		int ii;
		m_startTime = UINT64_MAX;
		m_lastTime = 0;
		MethodEntry* caller;

		dataFp = fopen(traceFileName, "rb");
		if (dataFp == NULL)
			goto bail;

		if (!parseKeys(dataFp, 0))
			goto bail;

		if (parseDataHeader(dataFp, &m_dataHeader) < 0)
			goto bail;

#if 0
		FILE *dumpStream = fopen("debug", "w");
#endif

		while (1) {
			int threadId;
			unsigned int methodVal;
			uint64_t currentTime;
			int action;
			int64_t methodId;
			CallStack *pStack;
			/*
			* Extract values from file.
			*/
			if (readDataRecord(dataFp, &m_dataHeader, &threadId, &methodVal, &currentTime))
				break;

			action = METHOD_ACTION(methodVal);
			methodId = METHOD_ID(methodVal);

			/* Get the call stack for this thread */
			pStack = m_traceData.stacks[threadId];

			/* If there is no call stack yet for this thread, then allocate one */
			if (pStack == NULL) {
				pStack = new CallStack;
				pStack->thread = m_dataKeys->lookupThread(threadId);
				pStack->maxStackDepth = 0;
				pStack->top = 0;
				pStack->lastEventTime = currentTime;
				pStack->threadStartTime = currentTime;
				m_traceData.stacks[threadId] = pStack;
			}

			/* Lookup the current method */
			method = m_dataKeys->lookupMethod(methodId);
			if (method == NULL)
				method = &(m_dataKeys->methods[UNKNOWN_INDEX]);

			DEBUG_TRACE("Data Record: threadId=%d, action=%s(%d), method=(Id=%d, name=%s.%s, signature=%s)\n",
				threadId, szMethodActions[action], action, (int)methodId, method->className, method->methodName, method->signature);

			if (action == METHOD_TRACE_ENTER) {
				/* This is a method entry */
				if (pStack->top >= MAX_STACK_DEPTH) {
					DEBUG_TRACE( "Stack overflow (exceeded %d frames)\n",
						MAX_STACK_DEPTH);
					return false;
				}
				
				CallEntry* callEntry = new CallEntry(pStack->thread, method, action, currentTime, NULL);
				/* Get the caller method */
				if (pStack->top >= 1) {
					caller = pStack->calls[pStack->top - 1].method;
					callEntry->parent = pStack->calls[pStack->top - 1].callEntry;
					callEntry->parent->hasChildren = true;
				}
				else {
					caller = &(m_dataKeys->methods[TOPLEVEL_INDEX]);
					callEntry->parent = NULL;
				}
				countRecursiveEntries(pStack, pStack->top, caller);
				caller->elapsedExclusive += currentTime - pStack->lastEventTime;


				if (caller->recursiveEntries <= 1) {
					caller->topExclusive += currentTime - pStack->lastEventTime;
				}

				/* Push the method on the stack for this thread */
				pStack->calls[pStack->top].method = method;
				pStack->calls[pStack->top++].callEntry = callEntry;

				m_callEntries.push_back(callEntry);

				if (pStack->top > pStack->maxStackDepth) {
					pStack->maxStackDepth = pStack->top;
				}
			}
			else {
				/* This is a method exit */
				uint64_t entryTime = 0;

				/* Pop the method off the stack for this thread */
				if (pStack->top > 0) {
					pStack->top -= 1;
					entryTime = pStack->calls[pStack->top].callEntry->startTime;
					if (method != pStack->calls[pStack->top].method) {
						if (method->methodName) {
							DEBUG_TRACE(
								"Exit from method %s.%s %s does not match stack:\n",
								method->className, method->methodName,
								method->signature);
						}
						else {
							DEBUG_TRACE(
								"Exit from method %s does not match stack:\n",
								method->className);
						}
						return false;
					}

				}

				/* Get the caller method */
				if (pStack->top >= 1) {
					caller = pStack->calls[pStack->top - 1].method;
					pStack->calls[pStack->top].callEntry->endTime = currentTime;
				}
				else {
					caller = &(m_dataKeys->methods[TOPLEVEL_INDEX]);
					CallEntry* callEntry = new CallEntry(pStack->thread, method, action, pStack->threadStartTime, NULL);
					callEntry->endTime = currentTime;
					m_callEntries.push_back(callEntry);
				}

				countRecursiveEntries(pStack, pStack->top, caller);
				countRecursiveEntries(pStack, pStack->top, method);

				uint64_t elapsed = currentTime - entryTime;
				addInclusiveTime(caller, method, elapsed);
				method->elapsedExclusive += currentTime - pStack->lastEventTime;
				if (method->recursiveEntries == 0) {
					method->topExclusive += currentTime - pStack->lastEventTime;
				}
			}
			/* Remember the time of the last entry or exit event */
//			ASSERT(currentTime >= m_lastTime);
			pStack->lastEventTime = currentTime;
		}

		TRACE("The totol block count = %d\n", m_callEntries.size());

		/* If we have calls on the stack when the trace ends, then clean
		* up the stack and add time to the callers by pretending that we
		* are exiting from their methods now.
		*/
		CallStack *pStack;
		int threadId;
		uint64_t sumThreadTime = 0;
		for (threadId = 0; threadId < MAX_THREADS; ++threadId) {
			pStack = m_traceData.stacks[threadId];

			/* If this thread never existed, then continue with next thread */
			if (pStack == NULL)
				continue;

			/* Also, add up the time taken by all of the threads */
			sumThreadTime += pStack->lastEventTime - pStack->threadStartTime;

			if (pStack->lastEventTime > m_lastTime) m_lastTime = pStack->lastEventTime;
			if (pStack->threadStartTime < m_startTime) m_startTime = pStack->threadStartTime;

			for (ii = 0; ii < pStack->top; ++ii) {
				if (ii == 0)
					caller = &(m_dataKeys->methods[TOPLEVEL_INDEX]);
				else
					caller = pStack->calls[ii - 1].method;
				method = pStack->calls[ii].method;
				countRecursiveEntries(pStack, ii, caller);
				countRecursiveEntries(pStack, ii, method);

				pStack->calls[ii].callEntry->endTime = pStack->lastEventTime;

				uint64_t entryTime = pStack->calls[ii].callEntry->startTime;
				uint64_t elapsed = pStack->lastEventTime - entryTime;
				addInclusiveTime(caller, method, elapsed);
			}
		}
		caller = &(m_dataKeys->methods[TOPLEVEL_INDEX]);
		caller->elapsedInclusive = sumThreadTime;

		m_threadTime = sumThreadTime;

	bail:
		if (dataFp != NULL)
			fclose(dataFp);

		return true;
	}

}
