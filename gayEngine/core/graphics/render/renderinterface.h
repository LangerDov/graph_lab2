#ifndef _RENDERINTERFACE_H
#define _RENDERINTERFACE_H

#include "matvec.h"

typedef void (*msgcallback_t)(const char *pMsg);

class IBuffer;

enum buffermemory_t
{
	memory_ram = 1,
	memory_gpu
};
enum bufferusage_t
{
	vertex_buffer_bit = (1 << 0),
	index_buffer_bit = (1 << 1),
	uniform_buffer_bit = (1 << 2),
	transfer_src_bit = (1 << 3),
	transfer_dst_bit = (1 << 4)
};

class IBufferRegion {
public:
	virtual unsigned int GetSize() = 0;
	virtual unsigned int GetOffset() = 0;
	virtual IBuffer* GetBuffer() = 0;

	// if region is already mapped it'll return already mapped memory pointer
	// if memory has flag device locale bit method will return nullptr
	virtual void* Map() = 0;
	virtual void UnMap() = 0;
};

class IBuffer {
public:
	// size will be aligned to 256 bytes
	virtual IBufferRegion* AllocMemory(unsigned int size) = 0;
	virtual void FreeMemory(IBufferRegion* pMemory) = 0;

	virtual unsigned int GetMemoryType() = 0;
	virtual unsigned int GetBufferUsage() = 0;
};

extern bool InitVulkan(msgcallback_t pCallbackFunc);
extern void DestroyVulkan();

class IRenderInterface
{
public:
	/// <summary>
	/// Initializes pukan interface
	/// </summary>
	/// <param name="pWindow">Pointer to GLFW window</param>
	/// <returns>True if succeded, false if failed</returns>
	virtual bool Init(void *pWindow) = 0;
	virtual void Shutdown() = 0;

	virtual void SetViewport(int width, int height) = 0;
	
	virtual IBuffer* CreateBuffer(size_t ulBufferSize, unsigned int iUsage, unsigned int iMemoryType) = 0;
	virtual void DeleteBuffer(IBuffer *pBuffer) = 0;

	virtual void BeginTransfer() = 0;
	virtual bool CopyBuffer(IBufferRegion *pSource, IBufferRegion *pDestination, 
		unsigned int size, unsigned int srcOffset, unsigned int dstOffset) = 0;
	virtual void EndTransfer(bool bWait = true) = 0;

	/*virtual void PushConstant(void* pData, size_t ulSize) = 0;
	virtual void Draw(IBufferRegion* pRegion, unsigned int offset, unsigned int vertCount) = 0;*/

	// RENDER COMMAND QUEUE

	// Begin recording command queue
	virtual void StartQueue() = 0;

	// End recording command queue and present rendered to screen
	virtual void EndQueue() = 0;
	virtual void WaitForDevice() = 0;

	virtual void *GetPhysDevice() = 0;
	virtual void* GetDevice() = 0;
	virtual void* GetCurrentBuffer() = 0;
	virtual void* GetRenderPass() = 0;
	virtual void* GetGraphicsQueue() = 0;
};

extern IRenderInterface* CreateVulkanInterface();

#endif