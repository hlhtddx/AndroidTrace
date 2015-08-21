#include "Common.hpp"

namespace Android {
#ifdef _CRTDBG_MAP_ALLOC
	std::set<Object*> Object::allObjects;
#endif
	uint64_t Object::mReferenceCount = 0;

	bool readToken(std::istream& str, String& buffer, char token) {
		while (true) {
			char ch = str.get();
			if (str.eof())
				break;
			if (ch == token)
				return true;
			buffer.push_back(ch);
		}
		return !buffer.empty();
	}
};
