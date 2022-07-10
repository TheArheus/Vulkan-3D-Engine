#include "vulkan_pipeline.h"
#include "vulkan_utils.h"

#include "core/vmemory.h"
#include "core/logger.h"
#include "math/math_types.h"

b8 VulkanGraphicsPipelineCreate(vulkan_context* Context, vulkan_renderpass* Renderpass, 
                                u32 AttributeCount, VkVertexInputAttributeDescription* Attributes,
                                u32 DescriptorSetLayoutCount, VkDescriptorSetLayout* DescriptorSetLayouts, 
                                u32 StageCount, VkPipelineShaderStageCreateInfo* Stages,
                                VkViewport Viewport, VkRect2D Scissor, b8 IsWireframe, vulkan_pipeline* OutPipeline)
{
    VkPipelineViewportStateCreateInfo ViewportState = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    ViewportState.viewportCount = 1;
    ViewportState.pViewports = &Viewport;
    ViewportState.scissorCount = 1;
    ViewportState.pScissors = &Scissor;

    VkPipelineRasterizationStateCreateInfo RasterizerCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    RasterizerCreateInfo.polygonMode = IsWireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    RasterizerCreateInfo.lineWidth = 1.0f;
    RasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    RasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo MultisamplingCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    MultisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    MultisamplingCreateInfo.minSampleShading = 1.0f;

    VkPipelineDepthStencilStateCreateInfo DepthStencil = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    DepthStencil.depthTestEnable = VK_TRUE;
    DepthStencil.depthWriteEnable = VK_TRUE;
    DepthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

    VkPipelineColorBlendAttachmentState ColorBlendState = {};
    ColorBlendState.blendEnable = VK_TRUE;
    ColorBlendState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    ColorBlendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    ColorBlendState.colorBlendOp = VK_BLEND_OP_ADD;
    ColorBlendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    ColorBlendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    ColorBlendState.alphaBlendOp = VK_BLEND_OP_ADD;
    ColorBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo ColorBlendStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    ColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    ColorBlendStateCreateInfo.attachmentCount = 1;
    ColorBlendStateCreateInfo.pAttachments = &ColorBlendState;

    const u32 DynamicStateCount = 3;
    VkDynamicState DynamicStates[DynamicStateCount] = 
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };

    VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    DynamicStateCreateInfo.dynamicStateCount = DynamicStateCount;
    DynamicStateCreateInfo.pDynamicStates = DynamicStates;

    VkVertexInputBindingDescription BindingDescription;
    BindingDescription.binding = 0;
    BindingDescription.stride = sizeof(vertex_3d);
    BindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkPipelineVertexInputStateCreateInfo VertexInputInfo = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    VertexInputInfo.vertexBindingDescriptionCount = 1;
    VertexInputInfo.pVertexBindingDescriptions = &BindingDescription;
    VertexInputInfo.vertexAttributeDescriptionCount = AttributeCount;
    VertexInputInfo.pVertexAttributeDescriptions = Attributes;

    VkPipelineInputAssemblyStateCreateInfo InputAssembly = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

    VkPushConstantRange ConstantRange;
    ConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ConstantRange.offset = sizeof(mat4) * 0;
    ConstantRange.size   = sizeof(mat4) * 2;
    PipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    PipelineLayoutCreateInfo.pPushConstantRanges    = &ConstantRange;

    PipelineLayoutCreateInfo.setLayoutCount = DescriptorSetLayoutCount;
    PipelineLayoutCreateInfo.pSetLayouts    = DescriptorSetLayouts;

    VK_CHECK(vkCreatePipelineLayout(Context->Device.LogicalDevice, &PipelineLayoutCreateInfo, Context->Allocator, &OutPipeline->PipelineLayout));

    VkGraphicsPipelineCreateInfo PipelineCreateInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    PipelineCreateInfo.stageCount = StageCount;
    PipelineCreateInfo.pStages = Stages;
    PipelineCreateInfo.pVertexInputState = &VertexInputInfo;
    PipelineCreateInfo.pInputAssemblyState = &InputAssembly;

    PipelineCreateInfo.pViewportState = &ViewportState;
    PipelineCreateInfo.pRasterizationState = &RasterizerCreateInfo;
    PipelineCreateInfo.pMultisampleState = &MultisamplingCreateInfo;
    PipelineCreateInfo.pDepthStencilState = &DepthStencil;
    PipelineCreateInfo.pColorBlendState = &ColorBlendStateCreateInfo;
    PipelineCreateInfo.pDynamicState = &DynamicStateCreateInfo;
    PipelineCreateInfo.pTessellationState = 0;

    PipelineCreateInfo.layout = OutPipeline->PipelineLayout;

    PipelineCreateInfo.renderPass = Renderpass->Handle;
    PipelineCreateInfo.subpass = 0;
    PipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    PipelineCreateInfo.basePipelineIndex = -1;

    VkResult Result = vkCreateGraphicsPipelines(Context->Device.LogicalDevice, VK_NULL_HANDLE, 1, &PipelineCreateInfo, Context->Allocator, &OutPipeline->Handle);

    if(VulkanResultIsSuccess(Result))
    {
        VENG_DEBUG("Graphics Pipeline created!");
        return true;
    }

    VENG_ERROR("vkCreateGraphicsPipelines failed with %s", VulkanResultString(Result, true));
    return false;
}

void VulkanGraphicsPipelineDestroy(vulkan_context* Context, vulkan_pipeline* Pipeline)
{
    if(Pipeline)
    {
        if(Pipeline->Handle)
        {
            vkDestroyPipeline(Context->Device.LogicalDevice, Pipeline->Handle, Context->Allocator);
            Pipeline->Handle = 0;
        }

        if(Pipeline->PipelineLayout)
        {
            vkDestroyPipelineLayout(Context->Device.LogicalDevice, Pipeline->PipelineLayout, Context->Allocator);
            Pipeline->PipelineLayout = 0;
        }
    }
}

void VulkanPipelineBind(vulkan_command_buffer* CommandBuffer, VkPipelineBindPoint BindPoint, vulkan_pipeline* Pipeline)
{
    vkCmdBindPipeline(CommandBuffer->Handle, BindPoint, Pipeline->Handle);
}

