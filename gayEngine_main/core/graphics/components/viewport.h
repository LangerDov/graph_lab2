#pragma once
#include "str.h"

enum mouse_btn
{
	left,
	right,
	middle
};

class ViewPort
{
public:
	virtual void Draw() = 0;
	virtual void InitDraw(void* HWIN) = 0;
	virtual bool Create(UString name) = 0;
	virtual bool Destroy() = 0;
	virtual UString GetType() = 0;
	virtual void SetName(UString name) = 0;
	virtual UString GetName() = 0;
	virtual void SetPos(double x, double y) = 0;
	virtual void GetPos(double* x, double* y) = 0;
	virtual void SetSize(double x, double y) = 0;
	virtual void GetSize(double* x, double* y) = 0;
};