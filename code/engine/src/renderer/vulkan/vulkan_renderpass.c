#include "vulkan_renderpass.h"

#include "core/vmemory.h"

void VulkanRenderpassCreate(vulkan_context* Context, vulkan_renderpass* OutRenderpass, 
                            v4 RenderArea, v4 Color, 
                            r32 Depth, u32 Stencil, u8 Flags, b8 HasPrevPass, b8 HasNextPass)
{
    OutRenderpass->RenderArea = RenderArea;
    OutRenderpass->Color      = Color;

    OutRenderpass->Depth   = Depth;
    OutRenderpass->Stencil = Stencil;

    OutRenderpass->ClearFlags  = Flags;
    OutRenderpass->HasPrevPass = HasPrevPass;
    OutRenderpass->HasNextPass = HasNextPass;

    VkSubpassDescription Subpass = {};
    Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    u32 AttachmentDescriptionCount = 0;
    VkAttachmentDescription AttachmentDescriptions[2];

    b8 DoClearColor = (OutRenderpass->ClearFlags & RENDERPASS_CLEAR_COLOR_BUFFER_FLAG) != 0;
    VkAttachmentDescription ColorAttachment = {};
    ColorAttachment.format = Context->Swapchain.ImageFormat.format;
    ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    ColorAttachment.loadOp = DoClearColor ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    ColorAttachment.initialLayout = HasPrevPass ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
    ColorAttachment.finalLayout = HasNextPass ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    AttachmentDescriptions[AttachmentDescriptionCount] = ColorAttachment;
    AttachmentDescriptionCount++;

    VkAttachmentReference ColorAttachmentReference;
    ColorAttachmentReference.attachment = 0;
    ColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    Subpass.colorAttachmentCount = 1;
    Subpass.pColorAttachments = &ColorAttachmentReference;

    b8 DoClearDepth = (OutRenderpass->ClearFlags & RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG) != 0;
    if(DoClearDepth)
    {
        VkAttachmentDescription DepthAttachment = {};
        DepthAttachment.format = Context->Device.DepthFormat;
        DepthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        DepthAttachment.loadOp = DoClearDepth ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        DepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        DepthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        DepthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        DepthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        DepthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        AttachmentDescriptions[AttachmentDescriptionCount] = DepthAttachment;
        AttachmentDescriptionCount++;

        VkAttachmentReference DepthAttachmentReference;
        DepthAttachmentReference.attachment = 1;
        DepthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        Subpass.pDepthStencilAttachment = &DepthAttachmentReference;
    }
    else
    {
        ZeroMemory(&AttachmentDescriptions[AttachmentDescriptionCount], sizeof(VkAttachmentDescription));
        Subpass.pDepthStencilAttachment = 0;
    }

    VkSubpassDependency Dependency;
    Dependency.srcSubpass      = VK_SUBPASS_EXTERNAL;
    Dependency.dstSubpass      = 0;
    Dependency.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    Dependency.srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    Dependency.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    Dependency.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
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
    BeginInfo.renderArea.offset.x = Renderpass->RenderArea.x;
    BeginInfo.renderArea.offset.y = Renderpass->RenderArea.y;
    BeginInfo.renderArea.extent.width  = Renderpass->RenderArea.z;
    BeginInfo.renderArea.extent.height = Renderpass->RenderArea.w;

    BeginInfo.clearValueCount = 0;
    BeginInfo.pClearValues    = 0;

    VkClearValue ClearValues[2];
    ZeroMemory(ClearValues, sizeof(VkClearValue) * 2);
    b8 DoClearColor = (Renderpass->ClearFlags & RENDERPASS_CLEAR_COLOR_BUFFER_FLAG) != 0;
    if(DoClearColor)
    {
        CopyMemory(ClearValues[BeginInfo.clearValueCount].color.float32, Renderpass->Color.E, sizeof(r32) * 4);
        BeginInfo.clearValueCount++;
    }

    b8 DoClearDepth = (Renderpass->ClearFlags & RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG) != 0;
    if(DoClearDepth)
    {
        CopyMemory(ClearValues[BeginInfo.clearValueCount].color.float32, Renderpass->Color.E, sizeof(r32) * 4);
        ClearValues[BeginInfo.clearValueCount].depthStencil.depth = Renderpass->Depth;

        b8 DoClearStencil = (Renderpass->ClearFlags & RENDERPASS_CLEAR_STENCIL_BUFFER_FLAG) != 0;
        ClearValues[BeginInfo.clearValueCount].depthStencil.stencil = DoClearStencil ? Renderpass->Stencil : 0;
        BeginInfo.clearValueCount++;
    }

    BeginInfo.pClearValues = BeginInfo.clearValueCount > 0 ? ClearValues : 0;

    vkCmdBeginRenderPass(CommandBuffer->Handle, &BeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    CommandBuffer->State = COMMAND_BUFFER_STATE_IN_RENDER_PASS;
}

void VulkanRenderpassEnd(vulkan_command_buffer* CommandBuffer, vulkan_renderpass* Renderpass)
{
    vkCmdEndRenderPass(CommandBuffer->Handle);
    CommandBuffer->State = COMMAND_BUFFER_STATE_RECORDING;
}

