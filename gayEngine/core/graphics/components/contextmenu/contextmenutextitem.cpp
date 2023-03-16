#include "contextmenutextitem.h"

ContextMenuTextItem::ContextMenuTextItem()
{
}

ContextMenuTextItem::~ContextMenuTextItem()
{
}

void ContextMenuTextItem::Draw()
{
}

void ContextMenuTextItem::InitDraw(void* HWIN)
{
}

bool ContextMenuTextItem::Create(UString name)
{
	return false;
}

bool ContextMenuTextItem::Destroy()
{
	return false;
}

UString ContextMenuTextItem::GetType()
{
	return "ContextMenuTextItem";
}

void ContextMenuTextItem::SetText(UString text)
{
	this->text = text;
}

UString ContextMenuTextItem::GetText()
{
	return text;
}

void ContextMenuTextItem::SetIcon(void* icon)
{
}

void ContextMenuTextItem::RemoveIcon()
{
}

void ContextMenuTextItem::SetTextFont(TextFont fonr)
{
}

TextFont ContextMenuTextItem::GetTextFont()
{
	return TextFont();
}

void ContextMenuTextItem::SetBGColor(int color)
{
	BGColor = color;
}

int ContextMenuTextItem::GetBGColor()
{
	return BGColor;
}

void ContextMenuTextItem::SetSize(double width, double y)
{
}

void ContextMenuTextItem::GetSize(double* width, double* y)
{
}
