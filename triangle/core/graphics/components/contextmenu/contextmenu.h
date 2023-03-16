#pragma once
#include "basecontextmenuitem.h"
#include "list.h"


class ContextMenu : public BaseContexMenuItem
{
public:
	ContextMenu();
	~ContextMenu();
	virtual bool Create(UString name);
	virtual bool Destroy();
	virtual void Draw();
	virtual void SetName(UString newname);
	virtual UString GetName();
	virtual UString GetType();
	virtual void InitDraw(void* HWIN);
	virtual void Open(double x, double y);
	virtual void Close();
	virtual void SetPos(double x, double y);
	virtual void GetPos(double &x, double& y);
	virtual void SetSize(double width, double height);
	virtual void GetSize(double* width, double* height);
	virtual void AppendChild(ContextMenu* child);
	virtual bool PointOnMe(double x, double y);
	virtual void Click(double x, double y, mouse_btn btn);
	virtual void DbClick(double x, double y, mouse_btn btn);
	virtual void Pressed(double x, double y);
	virtual void Released(double x, double y);
	virtual void Hovered(double x, double y);
	virtual void UnHovered(double x, double y);
	virtual void AppendItem(BaseContexMenuItem* item);
	virtual void RemoveItem(BaseContexMenuItem* item);
	virtual void RemoveItem(size_t i);
protected:
	double x, y, width, height;
	UString name;
	ContextMenu* parent;
	List<ContextMenu*> childs;
	List<BaseContexMenuItem*> items;
private:

};