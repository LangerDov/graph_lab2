#pragma once
#include "../viewport.h"

#include "list.h"

class BaseContexMenuItem : ViewPort
{
public:
	BaseContexMenuItem();
	~BaseContexMenuItem();
	virtual void Draw();
	virtual void InitDraw(void* HWIN);
	virtual bool Create(UString name);
	virtual bool Destroy();
	virtual UString GetType();
	virtual void SetName(UString name);
	virtual UString GetName();
	virtual void SetPos(double x, double y);
	virtual void GetPos(double* x, double* y);
	virtual void SetSize(double width, double y) = 0;
	virtual void GetSize(double* width, double* y) = 0;
protected:
	UString name;
	double x, y;
private:

};