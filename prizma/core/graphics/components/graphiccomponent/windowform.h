#pragma once
#include "basegraphiccomponent.h"
#include "../contextmenu/basecontextmenuitem.h"
#include "window/windowdefs.h"

#include "callback.h"

class WindowForm: public BaseGraphicComponent
{
	friend bool OnTryClose(void* data);
	friend void Cursor_Move(void*data);
	friend void WindowResize(void* data);
public:
	void Draw();
	bool Create(UString name);
	bool Destroy();
	bool Open();
	bool Close();
	bool SetTitle(UString title);
	UString GetType();
	void SetOnTryCloseCallback(callback_t<bool> callback);
	void SetMouseMoveCallback(callback_t<void> callback);
	void SetWindowResizeCallback(callback_t<void> callback);
	bool OpenContextMenu(BaseContexMenuItem* menu);
	bool CloseContextMenu();
	bind_handler CreateBind(bind);
private:
	BaseGraphicComponent* last_hovered_cmp;
	BaseContexMenuItem* cnt_menu;
	callback_t<bool> on_try_close;
	callback_t<void> on_win_resize;
	callback_t<void> on_mouse_move;
	void* window;
};