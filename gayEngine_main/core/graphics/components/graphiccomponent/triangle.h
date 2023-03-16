#pragma once
#include "basegraphiccomponent.h"

class Triangle : public BaseGraphicComponent
{
public:
	bool Create();
	void SetSize(int width, int height);
	void SetPos(int x, int y);
	void GetSize(int& width, int& height);
	void GetPos(int& x, int& y);
	void UpdateFrame();
	void RenderInit(void* data);
private:

};
