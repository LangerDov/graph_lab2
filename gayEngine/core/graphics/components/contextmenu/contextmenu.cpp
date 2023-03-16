#include "contextmenu.h"

ContextMenu::ContextMenu()
{
	x = y = width = height = 0;
}

ContextMenu::~ContextMenu()
{
}

bool ContextMenu::Create(UString name)
{
	return false;
}

bool ContextMenu::Destroy()
{
	return false;
}

void ContextMenu::Draw()
{
}

void ContextMenu::SetName(UString newname)
{
	name = newname;
}

UString ContextMenu::GetName()
{
	return name;
}

UString ContextMenu::GetType()
{
	return "ContextMenu";
}

void ContextMenu::InitDraw(void* HWIN)
{
}

void ContextMenu::Open(double x, double y)
{
}

void ContextMenu::Close()
{
}

void ContextMenu::SetPos(double x, double y)
{
}

void ContextMenu::GetPos(double& x, double& y)
{
}

void ContextMenu::SetSize(double width, double height)
{
}

void ContextMenu::GetSize(double* width, double* height)
{
}

void ContextMenu::AppendChild(ContextMenu* child)
{
}

bool ContextMenu::PointOnMe(double x, double y)
{
	return false;
}

void ContextMenu::Click(double x, double y, mouse_btn btn)
{
}

void ContextMenu::DbClick(double x, double y, mouse_btn btn)
{
}


void ContextMenu::Pressed(double x, double y)
{
}

void ContextMenu::Released(double x, double y)
{
}

void ContextMenu::Hovered(double x, double y)
{
}

void ContextMenu::UnHovered(double x, double y)
{
}

void ContextMenu::AppendItem(BaseContexMenuItem* item)
{
}

void ContextMenu::RemoveItem(BaseContexMenuItem* item)
{
}


void ContextMenu::RemoveItem(size_t tm)
{
}