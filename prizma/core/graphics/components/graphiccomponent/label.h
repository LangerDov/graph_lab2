#pragma once
#include "basegraphiccomponent.h"
#include "graphics/render/font/font.h"

class Label : public BaseGraphicComponent
{
public:
	Label();
	~Label();
	void SetText(UString text);
	void SetFont(Font* font);
private:
	UString text;
	Font* font;
};