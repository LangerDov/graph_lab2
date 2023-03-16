#pragma once
#include "list.h"
#include "callback.h"
#include "../viewport.h"

struct Anchors
{
	struct size_acnhr
	{
		bool x = false;
		bool y = false;
		bool width = false;
		bool height = false;
	};
	struct pos_acnr
	{
		bool x = false;
		bool y = false;
	};
	size_acnhr size;
	pos_acnr pos;
};



class BaseGraphicComponent : public ViewPort
{
public:
	BaseGraphicComponent();
	virtual bool Create(UString name);
	virtual bool Destroy();
	virtual void SetPos(double x, double y);
	virtual void GetPos(double* x, double* y);
	virtual void SetSize(double width, double height);
	virtual void GetSize(double* width, double* height);
	virtual void SetAngle(int agnle);
	virtual int GetAngle();
	virtual UString GetName();
	virtual void SetName(UString newname);
	virtual UString GetType();
	virtual void SetAnchors(Anchors ancr);
	virtual Anchors GetAnchors();
	virtual void Draw();
	virtual void ForceRedraw();
	virtual void SetPressedCallback(callback_t<void>);
	virtual void SetReleasedCallback(callback_t<void>);
	virtual void SetDbClickCallback(callback_t<void>);
	virtual void SetFocusedCallback(callback_t<void>);
	virtual void SetOutOfFocusCallback(callback_t<void>);
	virtual void AppendChild(BaseGraphicComponent* comp);
	virtual bool RemoveChild(int i);
	virtual bool RemoveChild(BaseGraphicComponent* comp);
	virtual void InitDraw(void* HWIN);
	virtual void Click(double x, double y, mouse_btn btn);
	virtual void DbClick(double x, double y, mouse_btn btn);
	virtual void Focused();
	virtual void OutOfFocus();
	virtual void Pressed(double x, double y);
	virtual void Released(double x, double y);
	virtual void Hovered(double x, double y);
	virtual void UnHovered(double x, double y);
	virtual bool PointOnMe(double x, double y);
	virtual void SeVisibility(bool val);
	virtual bool GetVisibility();
protected:
	virtual void ParentResize(double new_w, double new_h);
	virtual void ParentChangePos(double new_x, double new_y);
	BaseGraphicComponent* parent;
	List<BaseGraphicComponent*> childs;
	double pos_x;
	double pos_y;
	double width;
	double height;
	double angle;
	bool visible;
	Anchors anchros;
	UString name;
	callback_t<void> pressed_callback;
	callback_t<void> released_callback;
	callback_t<void> click_callback;
	callback_t<void> dbclick_callback;
	callback_t<void> focused_callback;
	callback_t<void> outoffocus_callback;



private:
	
};