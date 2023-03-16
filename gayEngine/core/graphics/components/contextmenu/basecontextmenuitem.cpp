#include "basecontextmenuitem.h"

BaseContexMenuItem::BaseContexMenuItem()
{
}

BaseContexMenuItem::~BaseContexMenuItem()
{
}

void BaseContexMenuItem::Draw()
{
}

void BaseContexMenuItem::InitDraw(void* HWIN)
{
}

bool BaseContexMenuItem::Create(UString name)
{
	return false;
}

bool BaseContexMenuItem::Destroy()
{
	return false;
}

UString BaseContexMenuItem::GetType()
{
	return "BaseContextMenuItem";
}


void BaseContexMenuItem::SetName(UString name)
{
	this->name = name;
}

UString BaseContexMenuItem::GetName()
{
	return name;
}

void BaseContexMenuItem::SetPos(double x, double y)
{
	this->x = x;
	this->y = y;
}

void BaseContexMenuItem::GetPos(double* x, double* y)
{
	*x = this->x;
	*y = this->y;
}
