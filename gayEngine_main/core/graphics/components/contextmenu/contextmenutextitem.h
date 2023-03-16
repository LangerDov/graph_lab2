#pragma once
#include "basecontextmenuitem.h"

typedef void* TextFont; // nado sdelat class dlya font!!!! FreeType nado sdelat

class ContextMenuTextItem : public BaseContexMenuItem
{
public:
	ContextMenuTextItem();
	~ContextMenuTextItem();
	virtual void Draw();
	virtual void InitDraw(void* HWIN);
	virtual bool Create(UString name);
	virtual bool Destroy();
	virtual UString GetType();
	virtual void SetText(UString text);
	virtual UString GetText();
	virtual void SetIcon(void* icon);
	virtual void RemoveIcon();
	virtual void SetTextFont(TextFont fonr);
	virtual TextFont GetTextFont();
	virtual void SetBGColor(int color);
	virtual int GetBGColor();
	virtual void SetSize(double width, double y);
	virtual void GetSize(double* width, double* y);
protected:
	UString text;
	TextFont font;
	int BGColor;
};

