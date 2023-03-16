#pragma once
#include "basegraphiccomponent.h"
#include "debugging/log.h"
#include <cmath>

BaseGraphicComponent::BaseGraphicComponent()
{
	name = "";
	angle = 0;
	pos_x = 0;
	pos_y = 0;
	width = 100;
	height = 100;
	focused_callback.func = nullptr;
	focused_callback.data = nullptr;
	outoffocus_callback.func = nullptr;
	outoffocus_callback.data = nullptr;
	click_callback.func = nullptr;
	click_callback.data = nullptr;
	dbclick_callback.func = nullptr;
	dbclick_callback.data = nullptr;
	released_callback.func = nullptr;
	released_callback.data = nullptr;
	anchros.pos.x = false;
	anchros.pos.y = false;
	anchros.size.x = false;
	anchros.size.y = false;
}

bool BaseGraphicComponent::Create(UString nm)
{
	name = nm;
	angle = 0;
	pos_x = 0;
	pos_y = 0;
	width = 100;
	height = 100;
	focused_callback.func = nullptr;
	focused_callback.data = nullptr;
	outoffocus_callback.func = nullptr;
	outoffocus_callback.data = nullptr;
	click_callback.func = nullptr;
	click_callback.data = nullptr;
	dbclick_callback.func = nullptr;
	dbclick_callback.data = nullptr;
	released_callback.func = nullptr;
	released_callback.data = nullptr;
	anchros.pos.x = false;
	anchros.pos.y = false;
	anchros.size.x = false;
	anchros.size.y = false;
	return true;
}

bool BaseGraphicComponent::Destroy()
{
	return false;
}

void BaseGraphicComponent::SetPos(double x, double y)
{
	for (size_t i = 0; i < childs.Size(); i++)
	{
		childs[i]->ParentChangePos(x, y);
	}
	pos_x = x;
	pos_y = y;
}

void BaseGraphicComponent::GetPos(double* x, double* y)
{
	*x = pos_x;
	*y = pos_y;
}

void BaseGraphicComponent::SetSize(double width, double height)
{
	for (size_t i = 0; i < childs.Size(); i++)
	{
		childs[i]->ParentResize(width, height);
	}
	this->width = width;
	this->height = height;
}

void BaseGraphicComponent::GetSize(double* width, double* height)
{
	*width = this->width;
	*height = this->height;
}

void BaseGraphicComponent::SetAngle(int agnle)
{
	this->angle = angle;
}

int BaseGraphicComponent::GetAngle()
{
	return angle;
}

UString BaseGraphicComponent::GetName()
{
	return name;
}

void BaseGraphicComponent::SetName(UString nm)
{
	name = nm;
}

UString BaseGraphicComponent::GetType()
{
	return "BaseGraphicComponent";
}


void BaseGraphicComponent::SetAnchors(Anchors ancr)
{
	anchros = ancr;
}

Anchors BaseGraphicComponent::GetAnchors()
{
	return anchros;
}

void BaseGraphicComponent::Click(double x, double y, mouse_btn btn)
{
	if (click_callback.func)
		click_callback.func(click_callback.data);
}

void BaseGraphicComponent::DbClick(double x, double y, mouse_btn btn)
{
	if (dbclick_callback.func)
		dbclick_callback.func(dbclick_callback.data);
}

void BaseGraphicComponent::Focused()
{
	if (focused_callback.func)
		focused_callback.func(focused_callback.data);
}

void BaseGraphicComponent::OutOfFocus()
{
	if (outoffocus_callback.func)
		outoffocus_callback.func(outoffocus_callback.data);
}

void BaseGraphicComponent::Pressed(double x, double y)
{
}

void BaseGraphicComponent::Released(double x, double y)
{
}

void BaseGraphicComponent::Hovered(double x, double y)
{
}

void BaseGraphicComponent::UnHovered(double x, double y)
{
}

void BaseGraphicComponent::Draw()
{
	if (visible)
		for (size_t i = 0; i < childs.Size(); i++)
		{
			childs[i]->Draw();
		}
}

void BaseGraphicComponent::ForceRedraw()
{
}

void BaseGraphicComponent::SetPressedCallback(callback_t<void> data)
{
	pressed_callback.func = data.func;
	pressed_callback.data = data.data;
}

void BaseGraphicComponent::SetReleasedCallback(callback_t<void> data)
{
	released_callback.func = data.func;
	released_callback.data = data.data;
}

void BaseGraphicComponent::SetDbClickCallback(callback_t<void> data)
{
	dbclick_callback.func = data.func;
	dbclick_callback.data = data.data;
}

void BaseGraphicComponent::SetFocusedCallback(callback_t<void> data)
{
	focused_callback.func = data.func;
	focused_callback.data = data.data;
}

void BaseGraphicComponent::SetOutOfFocusCallback(callback_t<void> data)
{
	outoffocus_callback.func = data.func;
	outoffocus_callback.data = data.data;
}

void BaseGraphicComponent::AppendChild(BaseGraphicComponent* comp)
{
	comp->parent = this;
	childs.Add(comp);
}

bool BaseGraphicComponent::RemoveChild(int i)
{
	if (i >= 0 or i < childs.Size())
	{
		childs.Remove(i);
		return true;
	}
	return false;
}

bool BaseGraphicComponent::RemoveChild(BaseGraphicComponent* comp)
{
	return childs.Find(comp);
}

void BaseGraphicComponent::InitDraw(void* HWIN)
{
}

bool BaseGraphicComponent::PointOnMe(double x, double y)
{
	double x1 = pos_x;
	double x2 = pos_x;
	double x3 = pos_x + width;
	double y1 = pos_y;
	double y2 = pos_y + height;
	double y3 = pos_y;
	double t1 = (x1 - x) * (y2 - y1) - (x2 - x1) * (y1 - y);
	double t2 = (x2 - x) * (y3 - y2) - (x3 - x2) * (y2 - y);
	double t3 = (x3 - x) * (y1 - y3) - (x1 - x3) * (y3 - y);
	if (!t1 or !t2 or !t3)
		return true;
	if ((t1 > 0 and t2 > 0 and t3 > 0) or (t1 < 0 and t2 < 0 and t3 < 0))
		return true;
	x1 = pos_x + width;
	x2 = pos_x;
	x3 = pos_x + width;
	y1 = pos_y + height;
	y2 = pos_y + height;
	y3 = pos_y;
	t1 = (x1 - x) * (y2 - y1) - (x2 - x1) * (y1 - y);
	t2 = (x2 - x) * (y3 - y2) - (x3 - x2) * (y2 - y);
	t3 = (x3 - x) * (y1 - y3) - (x1 - x3) * (y3 - y);
	if (!t1 or !t2 or !t3)
		return true;
	if ((t1 > 0 and t2 > 0 and t3 > 0) or (t1 < 0 and t2 < 0 and t3 < 0))
		return true;
	return false;
}

void BaseGraphicComponent::SeVisibility(bool val)
{
	visible = val;
}

bool BaseGraphicComponent::GetVisibility()
{
	return visible;
}

void BaseGraphicComponent::ParentResize(double new_w, double new_h)
{
	if (parent)
	{
		if (!new_w)
			new_w = 1;
		if (!new_h)
			new_h = 1;
		double inew_h = height;
		double inew_w = width;
		double inew_x = height;
		double inew_y = width;
		if (anchros.pos.x)
		{
			inew_x = pos_x / parent->width *(new_w - parent->width);
		}
		if (anchros.pos.y)
		{
			inew_y = pos_y / parent->height * abs(new_h - parent->height);
		}
		if (anchros.size.height)
		{
			inew_w = width / new_w * abs(new_w - parent->width);
		}
		if (anchros.size.height)
		{
			inew_h = height / new_h * abs(new_h - parent->height);
		}
		for (size_t i = 0; i < childs.Size(); i++)
		{
			childs[i]->ParentResize(inew_w, inew_h);
			childs[i]->ParentChangePos(inew_x, inew_y);
		}
	}
	else
		Log(lErr, "called parent resize but parent undefined name %s type %s", this->GetType().Str(), this->GetName().Str());
}

void BaseGraphicComponent::ParentChangePos(double new_x, double new_y)
{
	if (parent)
	{
		pos_x += new_x - parent->pos_x;
		pos_y += new_y -  parent->pos_y;
		for (size_t i = 0; i < childs.Size(); i++)
		{
			childs[i]->ParentChangePos(pos_x, pos_y);
		}
	}
	else
		Log(lErr, "called parent repos but parent undefined name %s type %s", this->GetType().Str(), this->GetName().Str());
}
