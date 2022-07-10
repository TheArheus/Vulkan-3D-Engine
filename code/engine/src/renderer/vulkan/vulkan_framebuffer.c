#include "vulkan_framebuffer.h"

#include "core/vmemory.h"


void VulkanFramebufferCreate(vulkan_context* Context, vulkan_renderpass* Renderpass, u32 Width, u32 Height, u32 AttachmentCount, VkImageView* Attachments, vulkan_framebuffer* OutFramebuffer)
{
    OutFramebuffer->Attachments = Allocate(sizeof(VkImageView) * AttachmentCount, MEMORY_TAG_RENDERER);

    for(u32 AttachmentIndex = 0;
        AttachmentIndex < AttachmentCount;
        ++AttachmentIndex)
    {
        OutFramebuffer->Attachments[AttachmentIndex] = Attachments[AttachmentIndex];
    }

    OutFramebuffer->Renderpass = Renderpass;
    OutFramebuffer->AttachmentCount = AttachmentCount;

    VkFramebufferCreateInfo FramebufferCreateInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    FramebufferCreateInfo.renderPass = Renderpass->Handle;
    FramebufferCreateInfo.attachmentCount = AttachmentCount;
    FramebufferCreateInfo.pAttachments = OutFramebuffer->Attachments;
    FramebufferCreateInfo.width  = Width;
    FramebufferCreateInfo.height = Height;
    FramebufferCreateInfo.layers = 1;

    VK_CHECK(vkCreateFramebuffer(Context->Device.LogicalDevice, &FramebufferCreateInfo, Context->Allocator, &OutFramebuffer->Handle));
}

void VulkanFramebufferDestroy(vulkan_context* Context, vulkan_framebuffer* Framebuffer)
{
    vkDestroyFramebuffer(Context->Device.LogicalDevice, Framebuffer->Handle, Context->Allocator);
    if(Framebuffer->Attachments)
    {
        Free(Framebuffer->Attachments, sizeof(VkImageView) * Framebuffer->AttachmentCount, MEMORY_TAG_RENDERER);
        Framebuffer->Attachments = 0;
    }
    Framebuffer->Handle = 0;
    Framebuffer->AttachmentCount = 0;
    Framebuffer->Renderpass = 0;
}

