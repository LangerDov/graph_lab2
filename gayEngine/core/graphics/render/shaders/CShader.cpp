#include "vulkan/vulkan.h"
#include "IShader.h"
#include "../vulkanbuffer.h"

VkVertexInputAttributeDescription attrDesc[] = {
	{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex2d_t, pos)},
	{1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex2d_t, color)}
};
VkVertexInputBindingDescription vertDesc = { 0, sizeof(vertex2d_t), VK_VERTEX_INPUT_RATE_VERTEX };

class CGraphicsPipeline : public IShader
{
public:
	CGraphicsPipeline();
	~CGraphicsPipeline();

	virtual bool Init(shaderinit_t* pShaderInitInfo);
	virtual void Shutdown();

	virtual const char* GetName();

	virtual void SetLineWidth(float width) { vkCmdSetLineWidth(hCurrentBuffer, width); }

	virtual void PushConstant(void* pData, unsigned int uiSize);
	virtual void Draw(IBufferRegion* pRegion, unsigned int offset, int iVertCount);
	virtual void DrawIndexed(IBufferRegion* pRegion, unsigned int uiVertexOffset, unsigned int uiIndexOffset, unsigned int idxCount);

	virtual void Bind(rendermode_t mode);

private:
	VkPipeline CreatePipeline(VkShaderModule vertModule, VkShaderModule fragModule, VkPolygonMode polyMode);

private:
	char szName[32] = "";
	VkPhysicalDevice hPhysDevice;
	VkDevice hDevice;
	VkRenderPass hRenderPass;
	VkQueue hQueue;
	VkCommandBuffer hCurrentBuffer;
	IRenderInterface* pRender;

	VkPipeline hWireframePipeline, hFillPipeline;
	VkPipelineLayout hLayout;
};

CGraphicsPipeline::CGraphicsPipeline()
{
	hPhysDevice = nullptr;
	hDevice = nullptr;
	hLayout = nullptr;
	hWireframePipeline = hFillPipeline = nullptr;
	hRenderPass = nullptr;
	pRender = nullptr;
}

CGraphicsPipeline::~CGraphicsPipeline()
{
}

bool CGraphicsPipeline::Init(shaderinit_t* pShaderInitInfo)
{
	if (!pShaderInitInfo)
		return false;

	pRender = (IRenderInterface*)pShaderInitInfo->pRenderInterface;
	hPhysDevice = (VkPhysicalDevice)pRender->GetPhysDevice();
	hDevice = (VkDevice)pRender->GetDevice();
	hRenderPass = (VkRenderPass)pRender->GetRenderPass();
	hQueue = (VkQueue)pRender->GetGraphicsQueue();
	strcpy(szName, pShaderInitInfo->pName);

	VkShaderModule vertModule = nullptr, fragModule = nullptr;
	VkShaderModuleCreateInfo vertInfo = {};
	vertInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vertInfo.pCode = (uint32_t*)pShaderInitInfo->pVertCode;
	vertInfo.codeSize = pShaderInitInfo->uiVertSize;
	if (vkCreateShaderModule(hDevice, &vertInfo, nullptr, &vertModule))
		return false;
	VkShaderModuleCreateInfo fragInfo = {};
	fragInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	fragInfo.pCode = (uint32_t*)pShaderInitInfo->pFragCode;
	fragInfo.codeSize = pShaderInitInfo->uiFragSize;
	if (vkCreateShaderModule(hDevice, &fragInfo, nullptr, &fragModule))
		return false;

	VkPushConstantRange pushConstant = {};
	pushConstant.size = 128;
	pushConstant.offset = 0;
	pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
	pipelineLayoutInfo.pushConstantRangeCount = 1;

	if (vkCreatePipelineLayout(hDevice, &pipelineLayoutInfo, nullptr, &hLayout))
		return false;

	hWireframePipeline = CreatePipeline(vertModule, fragModule, VK_POLYGON_MODE_LINE);
	hFillPipeline = CreatePipeline(vertModule, fragModule, VK_POLYGON_MODE_FILL);

	vkDestroyShaderModule(hDevice, fragModule, nullptr);
	vkDestroyShaderModule(hDevice, vertModule, nullptr);

	return true;
}

void CGraphicsPipeline::Shutdown()
{
	vkDestroyPipeline(hDevice, hWireframePipeline, nullptr);
	vkDestroyPipeline(hDevice, hFillPipeline, nullptr);
	vkDestroyPipelineLayout(hDevice, hLayout, nullptr);
}

const char* CGraphicsPipeline::GetName()
{
	return "xyu";
}

void CGraphicsPipeline::PushConstant(void* pData, unsigned int uiSize)
{
	vkCmdPushConstants(hCurrentBuffer, hLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, uiSize, pData);
}

void CGraphicsPipeline::Draw(IBufferRegion* pRegion, unsigned int offset, int iVertCount)
{
	if (!pRegion)
		return;

	CVulkanBuffer* pBuffer = ((CVulkanBuffer*)pRegion->GetBuffer());
	VkBuffer pVkBuffer = pBuffer->GetVkBuffer();
	VkDeviceSize offsets[] = { ((CVulkanBuffer::CBufferRegion*)pRegion)->GetOffset() + offset };
	vkCmdBindVertexBuffers(hCurrentBuffer, 0, 1, &pVkBuffer, offsets);

	vkCmdDraw(hCurrentBuffer, iVertCount, 1, 0, 0);
}

void CGraphicsPipeline::DrawIndexed(IBufferRegion* pRegion, unsigned int uiVertexOffset, unsigned int uiIndexOffset, unsigned int idxCount)
{
	if (!pRegion)
		return;

	CVulkanBuffer* pBuffer = ((CVulkanBuffer*)pRegion->GetBuffer());
	VkBuffer pVkBuffer = pBuffer->GetVkBuffer();
	unsigned int regionSize = ((CVulkanBuffer::CBufferRegion*)pRegion)->GetOffset();
	VkDeviceSize vertexOffset[] = { regionSize + uiVertexOffset };
	vkCmdBindVertexBuffers(hCurrentBuffer, 0, 1, &pVkBuffer, vertexOffset);
	vkCmdBindIndexBuffer(hCurrentBuffer, pVkBuffer, regionSize + uiIndexOffset, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(hCurrentBuffer, idxCount, 1, 0, 0, 0);
}

void CGraphicsPipeline::Bind(rendermode_t mode)
{
	hCurrentBuffer = (VkCommandBuffer)pRender->GetCurrentBuffer();
	VkPipeline pipeline = 0;
	switch (mode)
	{
	case RENDERMODE_FILL:
		pipeline = hFillPipeline;
		break;
	case RENDERMODE_WIREFRAME:
		pipeline = hWireframePipeline;
		break;
	case RENDERMODE_POINT:
		break;
	}
	vkCmdBindPipeline(hCurrentBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

VkPipeline CGraphicsPipeline::CreatePipeline(VkShaderModule vertModule, VkShaderModule fragModule, VkPolygonMode polyMode)
{
	VkPipeline pipeline = 0;

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo stages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = 2;
	vertexInputInfo.pVertexBindingDescriptions = &vertDesc;
	vertexInputInfo.pVertexAttributeDescriptions = attrDesc;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = polyMode;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	//rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 3U;
	dynamicState.pDynamicStates = dynamicStates;

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = stages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = hLayout;
	pipelineInfo.renderPass = hRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VkPipelineCacheCreateInfo cacheInfo = {};
	cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	if (vkCreateGraphicsPipelines(hDevice, nullptr, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
		return nullptr;

	return pipeline;
}


IShader* CreateShader()
{
	return new CGraphicsPipeline();
}