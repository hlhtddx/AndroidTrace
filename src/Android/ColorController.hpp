#pragma once
#include "Common.hpp"

namespace Android {

	class MethodData;
	typedef uint32_t COLOR;

	class ColorController
	{
	private:
		static COLOR rgbColors[];

	public:
		static 	void assignMethodColors(Vector<MethodData*>* methods);

	};

};
