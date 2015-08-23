#include "ColorController.hpp"
#include "MethodData.hpp"

namespace Android {

#ifndef RGB
#define RGB(r,g,b) ((r) | ((g) << 8) | ((b) << 16))
#endif

	COLOR ColorController::rgbColors[] = {
		RGB(90, 90, 255),
		RGB(0, 240, 0),
		RGB(255, 0, 0),
		RGB(0, 255, 255),
		RGB(255, 80, 255),
		RGB(200, 200, 0),
		RGB(40, 0, 200),
		RGB(150, 255, 150),
		RGB(150, 0, 0),
		RGB(30, 150, 150),
		RGB(200, 200, 255),
		RGB(0, 120, 0),
		RGB(255, 150, 150),
		RGB(140, 80, 140),
		RGB(150, 100, 50),
		RGB(70, 70, 70),
	};

	inline unsigned char GetColorRed(COLOR color) {
		return (unsigned char)color;
	}

	inline unsigned char GetColorGreen(COLOR color) {
		return ((unsigned short)color) >> 8;
	}

	inline unsigned char GetColorBlue(COLOR color) {
		return (color & 0xff0000) >> 16;
	}

	void ColorController::assignMethodColors(Vector<MethodData*>* methods)
	{
		int nextColorIndex = 0;
		for (auto it = methods->begin(); it != methods->end(); it++) {
			MethodData* md = *it;
			COLOR rgb = rgbColors[nextColorIndex];
			nextColorIndex++;
			if (nextColorIndex == sizeof(rgbColors) / sizeof(rgbColors[0]))
				nextColorIndex = 0;

			md->setColor(rgb);

			unsigned char fadedRed = 150 + GetColorRed(rgb) / 4;
			unsigned char fadedGreen = 150 + GetColorGreen(rgb) / 4;
			unsigned char fadedBlue = 150 + GetColorBlue(rgb) / 4;
			md->setFadedColor(RGB(fadedRed, fadedGreen, fadedBlue));
		}
	}
};
