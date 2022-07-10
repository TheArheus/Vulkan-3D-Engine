#include "vulkan_image.h"

#include "vulkan_device.h"

#include "core/vmemory.h"
#include "core/logger.h"

void VulkanImageCreate(vulkan_context* Context, 
                       VkImageType ImageType, 
                       u32 Width, u32 Height, 
                       VkFormat Format, VkImageTiling Tiling, 
                       VkImageUsageFlags Usage, VkMemoryPropertyFlags MemoryFlags,
                       b32 CreateView,
                       VkImageAspectFlags ViewAspectFlags, 
                       vulkan_image* OutImage)
{
    OutImage->Width  = Width;
    OutImage->Height = Height;

    VkImageCreateInfo ImageCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    ImageCreateInfo.extent.width = Width;
    ImageCreateInfo.extent.height = Height;
    ImageCreateInfo.extent.depth = 1;
    ImageCreateInfo.mipLevels = 4;
    ImageCreateInfo.arrayLayers = 1;
    ImageCreateInfo.format = Format;
    ImageCreateInfo.tiling = Tiling;
    ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ImageCreateInfo.usage = Usage;
    ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateImage(Context->Device.LogicalDevice, &ImageCreateInfo, Context->Allocator, &OutImage->Handle));

    VkMemoryRequirements MemoryRequirements;
    vkGetImageMemoryRequirements(Context->Device.LogicalDevice, OutImage->Handle, &MemoryRequirements);

    s32 MemoryType = Context->FindMemoryIndex(MemoryRequirements.memoryTypeBits, MemoryFlags);
    if(MemoryType == -1)
    {
        VENG_FATAL("Required memory type not found");
    }

    VkMemoryAllocateInfo MemoryAllocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
    MemoryAllocateInfo.memoryTypeIndex = MemoryType;
    VK_CHECK(vkAllocateMemory(Context->Device.LogicalDevice, &MemoryAllocateInfo, Context->Allocator, &OutImage->Memory));

    VK_CHECK(vkBindImageMemory(Context->Device.LogicalDevice, OutImage->Handle, OutImage->Memory, 0));

    if(CreateView)
    {
        OutImage->View = 0;
        VulkanImageViewCreate(Context, Format, OutImage, ViewAspectFlags);
    }
}

void VulkanImageViewCreate(vulkan_context* Context, VkFormat Format, vulkan_image* Image, VkImageAspectFlags AspectFlags)
{
    VkImageViewCreateInfo ViewCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};

    ViewCreateInfo.image = Image->Handle;
    ViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ViewCreateInfo.format = Format;

    ViewCreateInfo.subresourceRange.aspectMask = AspectFlags;
    ViewCreateInfo.subresourceRange.baseMipLevel = 0;
    ViewCreateInfo.subresourceRange.levelCount = 1;
    ViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    ViewCreateInfo.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(Context->Device.LogicalDevice, &ViewCreateInfo, Context->Allocator, &Image->View));
}

void VulkanImageDestroy(vulkan_context* Context, vulkan_image* Image)
{
    if(Image->View)
    {
        vkDestroyImageView(Context->Device.LogicalDevice, Image->View, Context->Allocator);
        Image->View = 0;
    }
    if(Image->Memory)
    {
        vkFreeMemory(Context->Device.LogicalDevice, Image->Memory, Context->Allocator);
        Image->Memory = 0;
    }
    if(Image->Handle)
    {
        vkDestroyImage(Context->Device.LogicalDevice, Image->Handle, Context->Allocator);
        Image->Handle = 0;
    }
}

void VulkanImageTransitionLayout(vulkan_context* Context, vulkan_command_buffer* CommandBuffer, vulkan_image* Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout)
{
    VkImageMemoryBarrier Barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    Barrier.oldLayout = OldLayout;
    Barrier.newLayout = NewLayout;
    Barrier.srcQueueFamilyIndex = Context->Device.GraphicsQueueIndex;
    Barrier.dstQueueFamilyIndex = Context->Device.GraphicsQueueIndex;
    Barrier.image = Image->Handle;
    Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    Barrier.subresourceRange.baseMipLevel = 0;
    Barrier.subresourceRange.layerCount = 1;
    Barrier.subresourceRange.baseArrayLayer = 0;
    Barrier.subresourceRange.levelCount = 1;

    VkPipelineStageFlags SrcStage;
    VkPipelineStageFlags DstStage;

    if(OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        Barrier.srcAccessMask = 0;
        Barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        SrcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        DstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if(OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        Barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        SrcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        DstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        VENG_FATAL("Unsuported image layout.");
        return;
    }
    
    vkCmdPipelineBarrier(CommandBuffer->Handle, SrcStage, DstStage, 0, 0, 0, 0, 0, 1, &Barrier);
}

void VulkanImageCopyFromBuffer(vulkan_context* Context, vulkan_image* Image, VkBuffer Buffer, vulkan_command_buffer* CommandBuffer)
{
    VkBufferImageCopy CopyRegion;
    ZeroMemory(&CopyRegion, sizeof(VkBufferImageCopy));
    CopyRegion.bufferOffset = 0;
    CopyRegion.bufferRowLength = 0;
    CopyRegion.bufferImageHeight = 0;

    CopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    CopyRegion.imageSubresource.mipLevel = 0;
    CopyRegion.imageSubresource.baseArrayLayer = 0;
    CopyRegion.imageSubresource.layerCount = 1;

    CopyRegion.imageExtent.width = Image->Width;
    CopyRegion.imageExtent.height = Image->Height;
    CopyRegion.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(CommandBuffer->Handle, Buffer, Image->Handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &CopyRegion);
}

