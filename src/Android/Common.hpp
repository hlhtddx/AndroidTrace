#pragma once

#include <stdint.h>
#include <set>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <functional>
#include <utility>
#include <iomanip>

namespace Android {
	typedef std::string String;
	typedef uint32_t COLOR;
	typedef uint32_t id_type;

	template<class _Ty> class List : public std::list<_Ty>
	{
	};

	template<class _Ty> class Vector : public std::vector<_Ty>
	{
	};

    template<class _Ty> class Stack : public std::vector<_Ty>
    {
    private:
        String mName;

    public:
        Stack(const char* name)
        : mName(name)
        {
        }
        typedef std::vector<_Ty> basetype;
        static _Ty invalidValue;

        void push(const _Ty &value)
        {
            std::vector<_Ty>::push_back(value);
//#define DUMP_STACK
#ifdef DUMP_STACK
            printf("Stack(%s) : ", mName.c_str());
            for (auto it = basetype::begin(); it != basetype::end(); it++) {
                printf(" %d", *it);
            }
            printf("\n");
#endif
        }

        void pop()
        {
            if (basetype::size() == 0) {
                return;
            }

            basetype::pop_back();
        }

        const _Ty& top() const {
            if (basetype::size() == 0)
                return invalidValue;
            return basetype::back();
        }

        _Ty& top() {
            if (basetype::size() == 0)
                return invalidValue;
            return basetype::back();
        }
    };

	template<class _Kty, class _Ty> class HashMap : public std::map<_Kty, _Ty>
	{
	public:
		typedef std::map<_Kty, _Ty> basetype;

		void add(_Kty key, _Ty value) {
			basetype::insert(std::make_pair(key, value));
		}

		List<_Ty> *value_list() {
			List<_Ty> *ret = new List<_Ty>;
			for (auto it = this->begin(); it != this->end(); it++) {
				ret->push_back(it->second);
			}
			return ret;
		}

		Vector<_Ty> *value_vector() {
			Vector<_Ty> *ret = new Vector<_Ty>;
			for (auto it = this->begin(); it != this->end(); it++) {
				ret->push_back(it->second);
			}
			return ret;
		}
	};

	class GeneralException
	{
	public:
		GeneralException() {
		}

		GeneralException(const char* description)
			: mDescription(description) {
		}

		virtual const char* getDescription() {
			return mDescription.c_str();
		}

	protected:
		String mDescription;
	};

	class MemoryException : public GeneralException
	{
	public:
		MemoryException(const char* description)
			: GeneralException(description) {
		}
	};

	class BoundaryException : public GeneralException
	{
	public:
		BoundaryException(const char* description, size_t index, size_t boundary)
		{
			mIndex = index;
			mBoundary = boundary;
			std::stringstream ss(mDescription);
			ss << "Out of range: " << description << " -> (" << index << " out of " << boundary << ")";
		}

	protected:
		size_t mIndex;
		size_t mBoundary;
	};

	template<class _Ty> class FastArray
	{
	public:
		FastArray()
			: FastArray(16)
		{
		}

		FastArray(int Capacity) {
			mSize = 0;
			mCapacity = Capacity;
			mContents = (_Ty*)malloc(sizeof(_Ty) * mCapacity);
		}

		~FastArray()
		{
			free(mContents);
		}

		int add(const _Ty& value) {
			ensureCapacity(mSize + 1);
			memcpy(mContents + mSize, &value, sizeof(_Ty));
			return (int)mSize++;
		}

		int addNull() {
			ensureCapacity(mSize + 1);
			return (int)mSize++;
		}

		_Ty* add2(const _Ty& value) {
			return get(add(value));
		}

		_Ty* addNull2() {
			return get(addNull());
		}

		_Ty& operator[] (size_t index) {
			return *(get(index));
		}

		const _Ty& operator[] (size_t index) const {
			return *(get(index));
		}

		_Ty* get(size_t index) {
			if (index >= mSize || index < 0) {
				throw BoundaryException("Cannot access the element", index, mSize);
			}
			return &(mContents[index]);
		}

		const _Ty* get(size_t index) const {
			if (index >= mSize) {
				throw BoundaryException("Cannot access the element", index, mSize);
			}
			return &(mContents[index]);
		}

		_Ty& set(size_t index, const _Ty& value) {
			if (index >= mSize) {
				throw BoundaryException("Cannot access the element", index, mSize);
			}
			memcpy(mContents + index, &value, sizeof(_Ty));
		}

		void clear() {
			free(mContents);
			mSize = 0;
			mCapacity = 16;
			mContents = (_Ty*)malloc(sizeof(_Ty) * mCapacity);
		}

		void freeExtra() {
			if (mCapacity < mSize) {
				throw BoundaryException("freeExtra", mSize, mCapacity);
			}
			else if (mCapacity == mSize) {
				return;
			}
            else if (mSize == 0) {
                free(mContents);
                mContents = nullptr;
            }
            else
            {
                _Ty* newContents = (_Ty*)realloc(mContents, sizeof(_Ty) * mSize);
                if (newContents == nullptr) {
                    throw MemoryException("Cannot reallocate memory for freeExtra");
                }

                mContents = newContents;
            }
			mCapacity = mSize;
		}

		size_t size() const {
			return mSize;
		}
		
		void sort() {
			::qsort(mContents, mSize, sizeof(_Ty), compare);
		}

		static int compare(const void* _Left, const void* _Right);
	private:
		size_t mSize;
		size_t mCapacity;
		_Ty* mContents;

	protected:
		void ensureCapacity(size_t size) {
			if (size <= mCapacity)
				return;

			size_t newSize = getNewCapacity(size);
			_Ty* newContents = (_Ty*)realloc(mContents, sizeof(_Ty) * newSize);
			if (newContents == nullptr) {
				throw MemoryException("Cannot reallocate memory");
			}

			mContents = newContents;
			mCapacity = newSize;
		}

		size_t getNewCapacity(size_t size) {
			size_t newSize = mCapacity;
            if (newSize == 0) {
                newSize = 16;
            }
			while (newSize < size) {
				newSize *= 2;
			}
			return newSize;
		}

		size_t capacity() const {
			return mCapacity;
		}
	};

	class Object {
	public:
		Object() {
		}
		virtual ~Object() {
		}
	};

	class ByteBuffer {
	private:
		char* mData;
		size_t mSize;
		char* mCurrent;
		char* mEnd;
	public:
		ByteBuffer(void* data, size_t size) {
			mData = mCurrent = (char*)data;
			mSize = size;
			mEnd = mData + mSize;
		}

		~ByteBuffer() {
			if (mData) {
				delete mData;
			}
		}
		
		unsigned char getUByte() {
			if (end()) {
				throw BoundaryException("ByteBuffer out of bound", mCurrent - mData, mSize);
			}
			return (unsigned char)(*(mCurrent++));
		}

		char getByte() {
			return (char)getByte();
		}

		unsigned short getUShort() {
			uint32_t val = getUByte();
			val |= getUByte() << 8;
			return val;
		}

		short getShort() {
			return (short)getUShort();
		}

		uint32_t getUInt() {
			uint32_t val = getUShort();
			val |= getUShort() << 16;
			return val;
		}

		int getInt() {
			return (int)getUInt();
		}
		
		uint64_t getULong() {
			uint64_t val = getUInt();
			val |= ((uint64_t)getUInt()) << 32;
			return val;
		}

		int64_t getLong() {
			return (int64_t)getULong();
		}
		
		void skip(size_t n) {
			mCurrent += n;
		}

		bool end() {
			return mCurrent >= mData + mSize;
		}
	};

	bool readToken(std::istream& str, String& buffer, char token);

	inline bool readLine(std::istream& str, String& line) {
		return readToken(str, line, '\n');
	}
};

