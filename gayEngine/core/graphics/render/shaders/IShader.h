#ifndef _ISHADER_H
#define _ISHADER_H

#ifdef _WIN32
#pragma once
#endif

#include "matvec.h"
#include "graphics/render/renderinterface.h"

enum cullmode_t
{
	CULL_NONE = 0,
	CULL_FRONT,
	CULL_BACK,
	CULL_FRONTANDBACK 
};
enum windorder_t
{
	FRONT_FACE_CLOCKWISE = 0,
	FRONT_FACE_COUNTER_CLOCKWISE
};

enum rendermode_t
{
	RENDERMODE_FILL = 0,
	RENDERMODE_WIREFRAME,
	RENDERMODE_POINT
};

struct shaderinit_t
{
	const char* pName;
	const void* pVertCode;
	const void* pFragCode;
	unsigned int uiVertSize;
	unsigned int uiFragSize;
	IRenderInterface* pRenderInterface;
};

struct vertex3d_t
{
	Vector pos;
	Vector normal;
	Vector2D texCoord;
};

struct vertex2d_t
{
	Vector pos;
	Vector color;
};

class IShader
{
public:
	virtual bool Init(shaderinit_t* pShaderInitInfo) = 0;
	virtual void Shutdown() = 0;

	virtual const char* GetName() = 0;

	// set line width command
	// only works for lines ofc
	virtual void SetLineWidth(float width) = 0;

	virtual void PushConstant(void* pData, unsigned int uiSize) = 0;
	virtual void Draw(IBufferRegion* pRegion, unsigned int offset, int iVertCount) = 0;
	virtual void DrawIndexed(IBufferRegion* pRegion, unsigned int uiVertexOffset, unsigned int uiIndexOffset, unsigned int idxCount) = 0;

	virtual void Bind(rendermode_t mode) = 0;
};

extern IShader* CreateShader();

#endif