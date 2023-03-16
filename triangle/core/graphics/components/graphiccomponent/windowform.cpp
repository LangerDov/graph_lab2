#include "windowform.h"
#include "window/windowmanager.h"
#include "window/window.h"

bool OnTryClose(void* data)
{
	if (((WindowForm*)data)->on_try_close.func)
		return ((WindowForm*)data)->on_try_close.func(((WindowForm*)data)->on_try_close.data);
	return true;
}

void Cursor_Move(void* data)
{
	if (((WindowForm*)((crsr_pos_callback_data_t*)data)->data)->last_hovered_cmp->PointOnMe(((crsr_pos_callback_data_t*)data)->x, ((crsr_pos_callback_data_t*)data)->y))
	{
		return;
	}
	for (size_t i = 0; i < ((WindowForm*)((crsr_pos_callback_data_t*)data)->data)->childs.Size(); i++)
	{
		if (((WindowForm*)((crsr_pos_callback_data_t*)data)->data)->childs[i]->PointOnMe(((crsr_pos_callback_data_t*)data)->x, ((crsr_pos_callback_data_t*)data)->y))
		{
			((WindowForm*)((crsr_pos_callback_data_t*)data)->data)->last_hovered_cmp->UnHovered(((crsr_pos_callback_data_t*)data)->x, ((crsr_pos_callback_data_t*)data)->y);
			((WindowForm*)((crsr_pos_callback_data_t*)data)->data)->childs[i]->Hovered(((crsr_pos_callback_data_t*)data)->x, ((crsr_pos_callback_data_t*)data)->y);
			((WindowForm*)((crsr_pos_callback_data_t*)data)->data)->last_hovered_cmp = ((WindowForm*)((crsr_pos_callback_data_t*)data)->data)->childs[i];
			return;
		}
	}
	((WindowForm*)((crsr_pos_callback_data_t*)data)->data)->last_hovered_cmp->UnHovered(((crsr_pos_callback_data_t*)data)->x, ((crsr_pos_callback_data_t*)data)->y);
	((WindowForm*)((crsr_pos_callback_data_t*)data)->data)->Hovered(((crsr_pos_callback_data_t*)data)->x, ((crsr_pos_callback_data_t*)data)->y);
	((WindowForm*)((crsr_pos_callback_data_t*)data)->data)->last_hovered_cmp = ((WindowForm*)((crsr_pos_callback_data_t*)data)->data);
}

void WindowResize(void* data)
{
	if (((WindowForm*)((resize_callback_data_t*)data)->data)->on_win_resize.func)
		((WindowForm*)((resize_callback_data_t*)data)->data)->on_win_resize.func(((WindowForm*)((resize_callback_data_t*)data)->data)->on_win_resize.data);
	((WindowForm*)((resize_callback_data_t*)data)->data)->SetSize(((resize_callback_data_t*)data)->width, ((resize_callback_data_t*)data)->height);
	return;
}

void WindowForm::Draw()
{
	BaseGraphicComponent::Draw();
	if (cnt_menu)
		cnt_menu->Draw();
}

bool WindowForm::Create(UString name)
{
	on_try_close.data = nullptr;
	on_try_close.func = nullptr;
	on_win_resize.data = nullptr;
	on_win_resize.func = nullptr;
	last_hovered_cmp = this;
	cnt_menu = nullptr;
	BaseGraphicComponent::Create(name);
	window = WinManager->CreateWindow();
	if (!window)
		return false;
	((Window*)window)->SetRoot(this);

	callback_t<bool> trclose;
	trclose.data = this;
	trclose.func = OnTryClose;
	((Window*)window)->SetOnTryClose(trclose);
	
	callback_t<void> crsmove;
	crsmove.data = this;
	crsmove.func = Cursor_Move;
	((Window*)window)->SetOnCursorMove(crsmove);

	callback_t<void> winresize;
	winresize.data = this;
	winresize.func = WindowResize;
	((Window*)window)->SetOnWindowResize(winresize);
	return true;
}

bool WindowForm::Destroy()
{
	BaseGraphicComponent::Destroy();
	this->Close();
	return true;
}

bool WindowForm::Open()
{
	window_open_attr attrs;
	attrs.focused = true;
	if (window)
		return ((Window*)window)->Open(attrs);
	return false;
}

bool WindowForm::Close()
{
	if (window)
		return ((Window*)window)->Close();
	return false;
}

bool WindowForm::SetTitle(UString title)
{
	if (window)
		return ((Window*)window)->SetTitle(title);
	else
		return false;
}

UString WindowForm::GetType()
{
	return "WindowForm";
}

void WindowForm::SetOnTryCloseCallback(callback_t<bool> callback)
{
	on_try_close = callback;
}

void WindowForm::SetMouseMoveCallback(callback_t<void> callback)
{
}

void WindowForm::SetWindowResizeCallback(callback_t<void> callback)
{
	on_win_resize = callback;
}

bool WindowForm::OpenContextMenu(BaseContexMenuItem* menu)
{
	cnt_menu = menu;
	return true;
}

bool WindowForm::CloseContextMenu()
{
	if (!cnt_menu)
		return false;
	cnt_menu = nullptr;
	return true;
}


bind_handler WindowForm::CreateBind(bind)
{
	return bind_handler();
}

