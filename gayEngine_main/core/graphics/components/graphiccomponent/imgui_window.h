#include "basegraphiccomponent.h"
class ImguiWin : public BaseGraphicComponent
{
public:
	void UpdateFrame();
	bool drawable;
	bool need_init;
	float* val;
	void (*Draw)();
	void* frame;
	void RenderInit(void* data);
	void SetSize(int width, int height);
	void SetPos(int x, int y);
	void GetSize(int& width, int& height);
	void GetPos(int& x, int& y);
private:

};
