#pragma once
#include "Common.hpp"

namespace Android {

	class MethodData;
	
	class ColorController
	{
	private:
		static COLOR rgbColors[];

	public:
		static 	void assignMethodColors(Vector<MethodData*>* methods);

	};

};
