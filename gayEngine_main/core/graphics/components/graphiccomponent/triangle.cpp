#include "triangle.h"

bool Triangle::Create()
{
    return true;
}

void Triangle::SetSize(int width, int height)
{
    this->width = width;
    this->height = height;
}

void Triangle::SetPos(int x, int y)
{
    this->pos_x = x;
    this->pos_y = y;
}

void Triangle::GetSize(int& width, int& height)
{
}

void Triangle::GetPos(int& x, int& y)
{
}

void Triangle::UpdateFrame()
{
}

void Triangle::RenderInit(void* data)
{
}
