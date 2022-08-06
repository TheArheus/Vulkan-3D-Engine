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
    v4 RenderArea;
    v4 Color;

    r32 Depth;
    u32 Stencil;

    u8 ClearFlags;
    b8 HasPrevPass;
    b8 HasNextPass;

    vulkan_renderpass_state State;
} vulkan_renderpass;

typedef struct vulkan_swapchain
{
    VkSurfaceFormatKHR ImageFormat;
    u8 MaxFramesInFlight;
    VkSwapchainKHR Handle;
    u32 ImageCount;
    VkImage* Images;
    VkImageView* Views;

    vulkan_image DepthAttachment;

    VkFramebuffer Framebuffers[3];
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

#define UI_SHADER_STAGE_COUNT 2
#define VULKAN_UI_SHADER_DESCRIPTOR_COUNT 2
#define VULKAN_UI_SHADER_SAMPLER_COUNT 1

#define MATERIAL_SHADER_STAGE_COUNT 2
#define VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT 2
#define VULKAN_MATERIAL_SHADER_SAMPLER_COUNT 1

#define VULKAN_MAX_UI_COUNT 1024
#define VULKAN_MAX_MATERIAL_COUNT 1024
#define VULKAN_MAX_GEOMETRY_COUNT 4096

typedef struct vulkan_descriptor_state
{
    u32 Generations[3];
    u32 IDs[3];
} vulkan_descriptor_state;

typedef struct vulkan_object_shader_object_state
{
    VkDescriptorSet DescriptorSets[3];
    vulkan_descriptor_state DescriptorStates[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT];
} vulkan_object_shader_object_state;

typedef struct global_uniform_material_object
{
    mat4 Projection;
    mat4 View;
    mat4 Reserved0;
    mat4 Reserved1;
} global_uniform_material_object;

// NOTE: material_uniform_object
typedef struct local_uniform_material_object 
{
    v4 DiffuseColor;
    v4 Reserved0;
    v4 Reserved1;
    v4 Reserved2;
    mat4 Reserved3;
    mat4 Reserved4;
    mat4 Reserved5;
} local_uniform_material_object;

typedef struct global_uniform_ui_object
{
    mat4 Projection;
    mat4 View;
    mat4 Reserved0;
    mat4 Reserved1;
} global_uniform_ui_object;

// NOTE: material_uniform_object
typedef struct local_uniform_ui_object 
{
    v4 DiffuseColor;
    v4 Reserved0;
    v4 Reserved1;
    v4 Reserved2;
    mat4 Reserved3;
    mat4 Reserved4;
    mat4 Reserved5;
} local_uniform_ui_object;

typedef struct vulkan_material_shader
{
    vulkan_shader_stage Stages[MATERIAL_SHADER_STAGE_COUNT];

    VkDescriptorPool GlobalDescriptorPool;
    VkDescriptorSetLayout GlobalDescriptorSetLayout;

    VkDescriptorSet  GlobalDescriptorSets[3];

    global_uniform_material_object GlobalUBO;

    vulkan_buffer GlobalUniformBuffer;

    VkDescriptorPool ObjectDescriptorPool;
    VkDescriptorSetLayout ObjectDescriptorSetLayout;

    vulkan_buffer ObjectUniformBuffer;

    u32 ObjectUniformBufferIndex;

    texture_use SamplerUses[VULKAN_MATERIAL_SHADER_SAMPLER_COUNT];

    vulkan_object_shader_object_state ObjectStates[VULKAN_MAX_MATERIAL_COUNT];

    vulkan_pipeline Pipeline;
} vulkan_material_shader;

typedef struct vulkan_ui_shader
{
    vulkan_shader_stage Stages[UI_SHADER_STAGE_COUNT];

    VkDescriptorPool GlobalDescriptorPool;
    VkDescriptorSetLayout GlobalDescriptorSetLayout;

    VkDescriptorSet  GlobalDescriptorSets[3];

    global_uniform_ui_object GlobalUBO;

    vulkan_buffer GlobalUniformBuffer;

    VkDescriptorPool ObjectDescriptorPool;
    VkDescriptorSetLayout ObjectDescriptorSetLayout;

    vulkan_buffer ObjectUniformBuffer;

    u32 ObjectUniformBufferIndex;

    texture_use SamplerUses[VULKAN_UI_SHADER_SAMPLER_COUNT];

    vulkan_object_shader_object_state ObjectStates[VULKAN_MAX_UI_COUNT];

    vulkan_pipeline Pipeline;
} vulkan_ui_shader;

typedef struct vulkan_geometry_data
{
    u32 ID;
    u32 Generation;
    u32 VertexCount;
    u32 VertexSize;
    u32 VertexBufferOffset;
    u32 IndexCount;
    u32 IndexSize;
    u32 IndexBufferOffset;
} vulkan_geometry_data;

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
    vulkan_renderpass UiRenderpass;

    vulkan_buffer ObjectVertexBuffer;
    vulkan_buffer ObjectIndexBuffer;

    vulkan_command_buffer* GraphicsCommandBuffers;

    VkSemaphore* ImageAvailableSemaphores;
    VkSemaphore* QueueCompleteSemaphores;

    u32 InFlightFenceCount;
    VkFence InFlightFences[2];

    // NOTE: Holds pointers to fence, one per frame
    VkFence* ImagesInFlight[3];

    u32 ImageIndex;
    u32 CurrentFrame;

    b8 RecreatingSwapchain;

    vulkan_material_shader MaterialShader;
    vulkan_ui_shader UiShader;

    u64 GeometryVertexOffset;
    u64 GeometryIndexOffset;

    vulkan_geometry_data Geometries[VULKAN_MAX_GEOMETRY_COUNT];

    VkFramebuffer WorldFramebuffers[3];

    s32 (*FindMemoryIndex)(u32 TypeFilter, u32 PropertyFlags);
} vulkan_context;

typedef struct vulkan_texture 
{
    vulkan_image Image;
    VkSampler Sampler;
} vulkan_texture;

