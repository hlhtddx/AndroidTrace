#pragma once

#include "Common.hpp"
#include "MethodData.hpp"
#include "Call.hpp"
#ifdef CLOCK_SOURCE_THREAD_CPU
    #include "TraceAction.hpp"
#endif

namespace Android {

	typedef List<size_t> ThreadStack;
	typedef HashMap<MethodData*, int> MethodIntMap;
#ifdef CLOCK_SOURCE_THREAD_CPU
    typedef List<TraceAction> TraceActionList;
#endif

	class ThreadData : public Object
	{
	public:

	private:
		id_type mId;
		String mName;
		bool mIsEmpty;
		int mRootCall;
        int mLastCall;
        ThreadStack mStack;
		MethodIntMap mStackMethods;
        CallList mCallList;

	public:
        int mRank;
        bool mHaveGlobalTime;
		bool mHaveThreadTime;

		uint32_t mGlobalStartTime;
		uint32_t mGlobalEndTime;

		uint32_t mThreadStartTime;
		uint32_t mThreadEndTime;
		uint32_t mThreadCurrentTime;

	public:
		const char* getName() const
		{
			return mName.c_str();
		}
		Call* getRootCall()
		{
			return mCallList.get(mRootCall);
		}

		bool isEmpty() const
		{
			return mIsEmpty;
		}

	public:
#ifdef CLOCK_SOURCE_THREAD_CPU
        Call* enter(MethodData* method, TraceActionList* trace, MethodData* topLevel);
		Call* exit(MethodData* method, TraceActionList* trace);
		void endTrace(TraceActionList* trace);
#else
        Call* enter(MethodData* method, MethodData* topLevel);
        Call* exit(MethodData* method);
        void endTrace();
#endif

        Call* topCall();
        int top();

        void push(int index)
        {
            mStack.push_back(index);
        }

        void pop()
        {
            if (mStack.size() == 0) {
                return;
            }

            mStack.pop_back();
        }

        void updateRootCallTimeBounds()
		{
			if (!mIsEmpty) {
				mCallList.get(mRootCall)->mGlobalStartTime = mGlobalStartTime;
				mCallList.get(mRootCall)->mGlobalEndTime = mGlobalEndTime;
				mCallList.get(mRootCall)->mThreadStartTime = mThreadStartTime;
				mCallList.get(mRootCall)->mThreadEndTime = mThreadEndTime;
			}
		}

	public:
		id_type getId()
		{
			return mId;
		}
		
		uint32_t getCpuTime() const
		{
            if (mRootCall == -1) {
                return 0;
            }
			return mCallList.get(mRootCall)->mInclusiveCpuTime;
		}
		
		uint32_t getRealTime() const
		{
            if (mRootCall == -1) {
                return 0;
            }
            return mCallList.get(mRootCall)->mInclusiveRealTime;
		}

        CallList* getCallList()
        {
            return &mCallList;
        }

        int32_t getElapseTime() const
        {
            if (isEmpty()) {
                return -1;
            }
            return mGlobalEndTime - mGlobalStartTime;
        }

	public:
		ThreadData()
            : ThreadData(0, "Unknown")
		{
        }

		ThreadData(id_type id, const char* name)
		{
			mId = id;
			std::stringstream ss;
			ss << "[" << id << "] " << name;
			mName = ss.str();
			mIsEmpty = true;
            mRootCall = -1;
            mLastCall = -1;
            mRank = -1;
            mHaveGlobalTime = false;
            mHaveThreadTime = false;
        }

		~ThreadData()
		{
		}

        void addRoot(MethodData* topLevel)
        {
            mRootCall = mCallList.addNull();
            mCallList.get(mRootCall)->init(this, topLevel, -1, mRootCall);
            mStack.push_back(mRootCall);
            mIsEmpty = false;
        }

		struct Greater : public std::binary_function<ThreadData*, ThreadData*, bool> {
			TimeBase* mTimeBase;

            Greater(TimeBase* timeBase) {
                mTimeBase = timeBase;
			}

			bool operator() (const ThreadData* _Left, const ThreadData* _Right) const {
                if (_Left->isEmpty()) {
                    return false;
                }
                if (_Right->isEmpty()) {
                    return true;
                }
                return mTimeBase->getTime(_Left) > mTimeBase->getTime(_Right);
			}
		};

        struct GreaterElapse : public std::binary_function<ThreadData*, ThreadData*, bool> {
            bool operator() (const ThreadData* _Left, const ThreadData* _Right) const {
                return _Left->getElapseTime() > _Right->getElapseTime();
            }
        };
    };
};
