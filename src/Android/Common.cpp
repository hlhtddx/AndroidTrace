#include "Common.hpp"

namespace Android {

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
