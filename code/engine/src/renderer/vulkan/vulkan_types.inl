#pragma once

#include "defines.h"
#include "core/asserts.h"
#include "renderer/renderer_types.inl"

#include <vulkan/vulkan.h>

#define VK_CHECK(Expr) Assert(Expr == VK_SUCCESS)

typedef struct vulkan_buffer
{
    u64 TotalSize;
    VkBuffer Handle;
    VkBufferUsageFlagBits Usage;
    b8 IsLocked;
    VkDeviceMemory Memory;
    s32 MemoryIndex;
    u32 MemoryPropertyFlags;
} vulkan_buffer;

typedef struct vulkan_swapchain_support_info
{
    VkSurfaceCapabilitiesKHR Capabilities;

    u32 FormatCount;
    VkSurfaceFormatKHR* Formats;

    u32 PresentModeCount;
    VkPresentModeKHR* PresentModes;
} vulkan_swapchain_support_info;

typedef struct vulkan_device
{
    VkPhysicalDevice PhysicalDevice;
    VkDevice LogicalDevice;
    vulkan_swapchain_support_info SwapchainSupport;

    u32 GraphicsQueueIndex;
    u32 PresentQueueIndex;
    u32 TransferQueueIndex;

    b8 SupportsDeviceLocalHostVisible;

    VkQueue GraphicsQueue;
    VkQueue PresentQueue;
    VkQueue TransferQueue;

    VkCommandPool GraphicsCommandPool;

    VkPhysicalDeviceProperties Properties;
    VkPhysicalDeviceFeatures   Features;
    VkPhysicalDeviceMemoryProperties MemoryProperties;

    VkFormat DepthFormat;
} vulkan_device;

typedef struct vulkan_image
{
    VkImage Handle;
    VkDeviceMemory Memory;
    VkImageView View;
    u32 Width;
    u32 Height;
} vulkan_image;

typedef enum vulkan_renderpass_state
{
    READY,
    RECORDING,
    IN_RENDER_PASS,
    RECORDING_ENDED,
    SUBMITTED,
    NOT_ALLOCATED
} vulkan_renderpass_state;

typedef struct vulkan_renderpass
{
    VkRenderPass Handle;
    r32 X, Y;
    r32 Width, Height;
    r32 R, G, B, A;

    r32 Depth;
    u32 Stencil;

    vulkan_renderpass_state State;
} vulkan_renderpass;

typedef struct vulkan_framebuffer
{
    VkFramebuffer Handle;
    u32 AttachmentCount;
    VkImageView* Attachments;
    vulkan_renderpass* Renderpass;
} vulkan_framebuffer;

typedef struct vulkan_swapchain
{
    VkSurfaceFormatKHR ImageFormat;
    u8 MaxFramesInFlight;
    VkSwapchainKHR Handle;
    u32 ImageCount;
    VkImage* Images;
    VkImageView* Views;

    vulkan_image DepthAttachment;

    vulkan_framebuffer* Framebuffers;
} vulkan_swapchain;

typedef enum vulkan_command_buffer_state
{
    COMMAND_BUFFER_STATE_READY,
    COMMAND_BUFFER_STATE_RECORDING,
    COMMAND_BUFFER_STATE_IN_RENDER_PASS,
    COMMAND_BUFFER_STATE_RECORDING_ENDED,
    COMMAND_BUFFER_STATE_SUBMITTED,
    COMMAND_BUFFER_STATE_NOT_ALLOCATED
} vulkan_command_buffer_state;

typedef struct vulkan_command_buffer
{
    VkCommandBuffer Handle;

    vulkan_command_buffer_state State;
} vulkan_command_buffer;

typedef struct vulkan_fence
{
    VkFence Handle;
    b8 IsSignaled;
} vulkan_fence;

typedef struct vulkan_pipeline
{
    VkPipeline Handle;
    VkPipelineLayout PipelineLayout;
} vulkan_pipeline;

typedef struct vulkan_shader_stage
{
    VkShaderModuleCreateInfo CreateInfo;
    VkShaderModule Handle;
    VkPipelineShaderStageCreateInfo ShaderStageCreateInfo;
} vulkan_shader_stage;

#define OBJECT_SHADER_STAGE_COUNT 2
#define VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT 2
#define VULKAN_OBJECT_MAX_OBJECT_COUNT 1024

typedef struct vulkan_descriptor_state
{
    u32 Generations[3];
    u32 IDs[3];
} vulkan_descriptor_state;

typedef struct vulkan_object_shader_object_state
{
    VkDescriptorSet DescriptorSets[3];
    vulkan_descriptor_state DescriptorStates[VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT];
} vulkan_object_shader_object_state;

typedef struct vulkan_material_shader
{
    vulkan_shader_stage Stages[OBJECT_SHADER_STAGE_COUNT];

    VkDescriptorPool GlobalDescriptorPool;
    VkDescriptorSetLayout GlobalDescriptorSetLayout;

    VkDescriptorSet  GlobalDescriptorSets[3];

    global_uniform_object GlobalUBO;

    vulkan_buffer GlobalUniformBuffer;

    vulkan_pipeline Pipeline;

    VkDescriptorPool ObjectDescriptorPool;
    VkDescriptorSetLayout ObjectDescriptorSetLayout;

    vulkan_buffer ObjectUniformBuffer;

    u32 ObjectUniformBufferIndex;

    vulkan_object_shader_object_state ObjectStates[VULKAN_OBJECT_MAX_OBJECT_COUNT];

    b8 DescriptorUpdated[3];
} vulkan_material_shader;

typedef struct vulkan_context
{
    r32 DeltaTime;

    VkInstance Instance;
    VkAllocationCallbacks* Allocator;
    VkSurfaceKHR Surface;

    u32 FramebufferWidth;
    u32 FramebufferHeight;
    u64 FramebufferSizeGeneration;
    u64 FramebufferSizeLastGeneration;

#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT DebugMessenger;
#endif

    vulkan_device Device;

    vulkan_swapchain Swapchain;
    vulkan_renderpass MainRenderpass;

    vulkan_buffer ObjectVertexBuffer;
    vulkan_buffer ObjectIndexBuffer;

    vulkan_command_buffer* GraphicsCommandBuffers;

    VkSemaphore* ImageAvailableSemaphores;
    VkSemaphore* QueueCompleteSemaphores;

    u32 InFlightFenceCount;
    vulkan_fence* InFlightFences;

    vulkan_fence** ImagesInFlight;

    u32 ImageIndex;
    u32 CurrentFrame;

    b8 RecreatingSwapchain;

    vulkan_material_shader MaterialShader;

    u64 GeometryVertexOffset;
    u64 GeometryIndexOffset;

    s32 (*FindMemoryIndex)(u32 TypeFilter, u32 PropertyFlags);
} vulkan_context;

typedef struct vulkan_texture 
{
    vulkan_image Image;
    VkSampler Sampler;
} vulkan_texture;

