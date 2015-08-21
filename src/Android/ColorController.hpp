#pragma once
#include "Common.hpp"

namespace Android {

	class MethodData;
	class ColorController
	{
	private:
		static uint32_t rgbColors[];

	public:
		static 	void assignMethodColors(Vector<MethodData*>* methods);

	};

};
