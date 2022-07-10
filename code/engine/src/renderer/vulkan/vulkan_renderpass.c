#include "vulkan_renderpass.h"

#include "core/vmemory.h"

void VulkanRenderpassCreate(vulkan_context* Context, vulkan_renderpass* OutRenderpass, 
                            r32 X, r32 Y, 
                            r32 W, r32 H, 
                            r32 R, r32 G, r32 B, r32 A, 
                            r32 Depth, u32 Stencil)
{
    OutRenderpass->X = X;
    OutRenderpass->Y = Y;

    OutRenderpass->Width = W;
    OutRenderpass->Height = H;

    OutRenderpass->R = R;
    OutRenderpass->G = G;
    OutRenderpass->B = B;
    OutRenderpass->A = A;

    OutRenderpass->Depth   = Depth;
    OutRenderpass->Stencil = Stencil;

    VkSubpassDescription Subpass = {};
    Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    const u32 AttachmentDescriptionCount = 2;
    VkAttachmentDescription AttachmentDescriptions[AttachmentDescriptionCount];

    VkAttachmentDescription ColorAttachment = {};
    ColorAttachment.format = Context->Swapchain.ImageFormat.format;
    ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    AttachmentDescriptions[0] = ColorAttachment;

    VkAttachmentReference ColorAttachmentReference;
    ColorAttachmentReference.attachment = 0;
    ColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription DepthAttachment = {};
    DepthAttachment.format = Context->Device.DepthFormat;
    DepthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    DepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    DepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    DepthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    DepthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    DepthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    DepthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    AttachmentDescriptions[1] = DepthAttachment;

    VkAttachmentReference DepthAttachmentReference;
    DepthAttachmentReference.attachment = 1;
    DepthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    Subpass.colorAttachmentCount = 1;
    Subpass.pColorAttachments = &ColorAttachmentReference;
    Subpass.pDepthStencilAttachment = &DepthAttachmentReference;

    VkSubpassDependency Dependency;
    Dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    Dependency.dstSubpass    = 0;
    Dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    Dependency.srcAccessMask = 0;
    Dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    Dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    Dependency.dependencyFlags = 0;

    VkRenderPassCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    CreateInfo.attachmentCount = AttachmentDescriptionCount;
    CreateInfo.pAttachments = AttachmentDescriptions;
    CreateInfo.subpassCount = 1;
    CreateInfo.pSubpasses = &Subpass;
    CreateInfo.dependencyCount = 1;
    CreateInfo.pDependencies = &Dependency;

    VK_CHECK(vkCreateRenderPass(Context->Device.LogicalDevice, &CreateInfo, Context->Allocator, &OutRenderpass->Handle));
}

void VulkanRenderpassDestroy(vulkan_context* Context, vulkan_renderpass* Renderpass)
{
    if(Renderpass && Renderpass->Handle)
    {
        vkDestroyRenderPass(Context->Device.LogicalDevice, Renderpass->Handle, Context->Allocator);
        Renderpass->Handle = 0;
    }
}

void VulkanRenderpassBegin(vulkan_command_buffer* CommandBuffer, vulkan_renderpass* Renderpass, VkFramebuffer Framebuffer)
{
    VkRenderPassBeginInfo BeginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    BeginInfo.renderPass  = Renderpass->Handle;
    BeginInfo.framebuffer = Framebuffer;
    BeginInfo.renderArea.offset.x = Renderpass->X;
    BeginInfo.renderArea.offset.y = Renderpass->Y;
    BeginInfo.renderArea.extent.width  = Renderpass->Width;
    BeginInfo.renderArea.extent.height = Renderpass->Height;

    VkClearValue ClearValues[2];
    ZeroMemory(ClearValues, sizeof(VkClearValue) * 2);
    ClearValues[0].color.float32[0] = Renderpass->R;
    ClearValues[0].color.float32[1] = Renderpass->G;
    ClearValues[0].color.float32[2] = Renderpass->B;
    ClearValues[0].color.float32[3] = Renderpass->A;
    ClearValues[1].depthStencil.depth   = Renderpass->Depth;
    ClearValues[1].depthStencil.stencil = Renderpass->Stencil;

    BeginInfo.clearValueCount = 2;
    BeginInfo.pClearValues = ClearValues;

    vkCmdBeginRenderPass(CommandBuffer->Handle, &BeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    CommandBuffer->State = COMMAND_BUFFER_STATE_IN_RENDER_PASS;
}

void VulkanRenderpassEnd(vulkan_command_buffer* CommandBuffer, vulkan_renderpass* Renderpass)
{
    vkCmdEndRenderPass(CommandBuffer->Handle);
    CommandBuffer->State = COMMAND_BUFFER_STATE_RECORDING;
}

