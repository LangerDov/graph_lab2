#ifndef _BUFFER_H
#define _BUFFER_H

#include "vulkan/vulkan.h"
#include "graphics/render/renderinterface.h"
#include "optional.h"
#include "list.h"

const unsigned int ulBlockSize = 256;
struct memoryblock_t
{
	int owner;
};

class CVulkanBuffer : public IBuffer
{
public:
	class CBufferRegion : public IBufferRegion
	{
	public:
		CBufferRegion(IBuffer* pBuffer, uint32_t _iBlockStart, uint32_t _iBlockEnd, uint32_t offset, uint32_t size, uint32_t idx)
		{
			pMappedData = nullptr;
			this->pBuffer = pBuffer;
			iBlockStart = _iBlockStart;
			iBlockEnd = _iBlockEnd;
			ulOffset = offset, ulSize = size;
			this->idx = idx;
			uiBlockCount = _iBlockEnd - _iBlockStart;
		}
		~CBufferRegion() {}

		virtual unsigned int GetSize() { return ulSize; }
		virtual unsigned int GetOffset() { return ulOffset; }
		virtual IBuffer* GetBuffer() { return pBuffer; }

		virtual void* Map()
		{
			if (pMappedData)
				return pMappedData;
			if (pBuffer->GetMemoryType() & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
				return nullptr;

			VkDevice hDevice = ((CVulkanBuffer*)pBuffer)->GetVkDevice();
			VkDeviceMemory hMemory = ((CVulkanBuffer*)pBuffer)->GetVkMemory();
			if (vkMapMemory(hDevice, hMemory, ulOffset, ulSize, 0, &pMappedData))
				return nullptr;

			return pMappedData;
		}
		virtual void UnMap()
		{
			vkUnmapMemory(((CVulkanBuffer*)pBuffer)->GetVkDevice(), ((CVulkanBuffer*)pBuffer)->GetVkMemory());
		}

		uint32_t GetIdx() { return idx; }
		uint32_t GetStartBlock() { return iBlockStart; }
		uint32_t GetEndBlock() { return iBlockEnd; }
	private:
		void* pMappedData;
		IBuffer* pBuffer;
		uint32_t iBlockStart, iBlockEnd;
		uint32_t ulOffset;
		uint32_t ulSize;
		uint32_t idx;
		unsigned int uiBlockCount;
	};

	CVulkanBuffer(VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize size, VkDevice device)
	{
		hBuffer = buffer;
		hMemory = memory;
		ulSize = size;
		hDevice = device;
		uiBufferUsage = 0;
		uiMemoryFlags = 0;
		
		uiBlockCount = (size / 256) + 1;
		blocks = new memoryblock_t[uiBlockCount];
		memset(blocks, 0, sizeof(memoryblock_t) * uiBlockCount);
	}
	~CVulkanBuffer()
	{
		for (int i = 0; i < regions.Size(); i++)
			delete regions[i];
	}

	// non-virtual functions
	VkBuffer GetVkBuffer() { return hBuffer; }
	VkDeviceMemory GetVkMemory() { return hMemory; }
	uint32_t GetVkSize() { return ulSize; }
	VkDevice GetVkDevice() { return hDevice; }
	uint32_t GetUsageFlags() { return uiBufferUsage; }
	uint32_t GetMemoryFlags() { return uiMemoryFlags; }

	// virtual functions
	virtual uint32_t GetMemoryType() { return uiMemoryFlags; }
	virtual uint32_t GetBufferUsage() { return uiBufferUsage; }

	virtual IBufferRegion* AllocMemory(uint32_t size)
	{
		size = (size / 256) * 256 + 256;
		uint32_t requiredBlocks = size / 256;

		int iOwnerId = regions.Size() + 1;

		uint32_t i = 0;
		Optional<uint32_t> startBlock, endBlock;
		while (i < uiBlockCount)
		{
			if (!blocks[i].owner && !startBlock.HasValue())
				startBlock = i;
			if (i - startBlock.GetValue() > 0 && !blocks[i].owner)
			{
				endBlock = i;
				break;
			}
			if (blocks[i].owner)
				startBlock.Erase();

			i++;
		}
		if (!startBlock.HasValue())
			return nullptr;

		for (uint32_t i = startBlock.GetValue(); i <= endBlock.GetValue(); i++)
			blocks[i].owner = iOwnerId;

		CBufferRegion* pBufferRegion = new CBufferRegion(this, startBlock.GetValue(), endBlock.GetValue(), startBlock.GetValue() * 256, size, iOwnerId);
		regions.Add(pBufferRegion);
		return pBufferRegion;
	}
	virtual void FreeMemory(IBufferRegion* pRegion)
	{
		if (!pRegion)
			return;

		CBufferRegion* pReg = (CBufferRegion*)pRegion;

		for (uint32_t i = pReg->GetStartBlock(); i < pReg->GetEndBlock(); i++)
			blocks[i].owner = 0;

		uint32_t regionIdx = ((CBufferRegion*)pRegion)->GetIdx();
		regions.Remove(regionIdx);
		delete pRegion;
	}

private:
	VkBuffer hBuffer;
	VkDeviceMemory hMemory;
	uint32_t ulSize;
	uint32_t uiBufferUsage;
	uint32_t uiMemoryFlags;

	unsigned int uiBlockCount;
	memoryblock_t *blocks;
	List<CBufferRegion*> regions;

	VkDevice hDevice;
};

#endif