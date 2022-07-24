#include "vulkan_material_shader.h"

#include "core/logger.h"
#include "core/vmemory.h"

#include "math/math_types.h"
#include "math/vmath.h"

#include "renderer/vulkan/vulkan_shader_utils.h"
#include "renderer/vulkan/vulkan_pipeline.h"
#include "renderer/vulkan/vulkan_buffer.h"

#include "systems/texture_system.h"

#define BUILTIN_SHADER_NAME_MATERIAL "material.shader"

b8 VulkanMaterialShaderCreate(vulkan_context* Context, vulkan_material_shader* OutShader)
{
    char StageTypeStrings[MATERIAL_SHADER_STAGE_COUNT][5] = {"vert", "frag"};
    VkShaderStageFlagBits StageTypes[MATERIAL_SHADER_STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

    for(u32 StageIndex = 0;
        StageIndex < MATERIAL_SHADER_STAGE_COUNT;
        ++StageIndex)
    {
        if(!CreateShaderModule(Context, BUILTIN_SHADER_NAME_MATERIAL, StageTypeStrings[StageIndex], StageTypes[StageIndex], StageIndex, OutShader->Stages))
        {
            VENG_ERROR("Unable to create %s shader module for '%s'", StageTypeStrings[StageIndex], BUILTIN_SHADER_NAME_MATERIAL);
            return false;
        }
    }

    VkDescriptorSetLayoutBinding GlobalUBOLayoutBinding;
    GlobalUBOLayoutBinding.binding = 0;
    GlobalUBOLayoutBinding.descriptorCount = 1;
    GlobalUBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    GlobalUBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo GlobalLayoutCreateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    GlobalLayoutCreateInfo.bindingCount = 1;
    GlobalLayoutCreateInfo.pBindings = &GlobalUBOLayoutBinding;
    VK_CHECK(vkCreateDescriptorSetLayout(Context->Device.LogicalDevice, &GlobalLayoutCreateInfo, Context->Allocator, &OutShader->GlobalDescriptorSetLayout));

    VkDescriptorPoolSize PoolSize;
    PoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    PoolSize.descriptorCount = Context->Swapchain.ImageCount;

    VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    DescriptorPoolCreateInfo.poolSizeCount = 1;
    DescriptorPoolCreateInfo.pPoolSizes = &PoolSize;
    DescriptorPoolCreateInfo.maxSets = Context->Swapchain.ImageCount;
    VK_CHECK(vkCreateDescriptorPool(Context->Device.LogicalDevice, &DescriptorPoolCreateInfo, Context->Allocator, &OutShader->GlobalDescriptorPool));

    OutShader->SamplerUses[0] = TEXTURE_USE_MAP_DIFFUSE;

    VkDescriptorType DescriptorTypes[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT] = 
    {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    };

    VkDescriptorSetLayoutBinding Bindings[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT];
    ZeroMemory(Bindings, sizeof(VkDescriptorSetLayoutBinding) * VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT);
    for(u32 BindingIndex = 0;
        BindingIndex < VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT;
        ++BindingIndex)
    {
        Bindings[BindingIndex].binding = BindingIndex;
        Bindings[BindingIndex].descriptorCount = 1;
        Bindings[BindingIndex].descriptorType = DescriptorTypes[BindingIndex];
        Bindings[BindingIndex].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    VkDescriptorSetLayoutCreateInfo LayoutInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    LayoutInfo.bindingCount = VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT;
    LayoutInfo.pBindings = Bindings;
    VK_CHECK(vkCreateDescriptorSetLayout(Context->Device.LogicalDevice, &LayoutInfo, 0, &OutShader->ObjectDescriptorSetLayout));

    VkDescriptorPoolSize ObjectPoolSizes[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT];
    ObjectPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ObjectPoolSizes[0].descriptorCount = VULKAN_MAX_MATERIAL_COUNT;
    ObjectPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    ObjectPoolSizes[1].descriptorCount = VULKAN_MATERIAL_SHADER_SAMPLER_COUNT * VULKAN_MAX_MATERIAL_COUNT;

    VkDescriptorPoolCreateInfo ObjectPoolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    ObjectPoolInfo.poolSizeCount = VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT;
    ObjectPoolInfo.pPoolSizes = ObjectPoolSizes;
    ObjectPoolInfo.maxSets = VULKAN_MAX_MATERIAL_COUNT;
    ObjectPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    VK_CHECK(vkCreateDescriptorPool(Context->Device.LogicalDevice, &ObjectPoolInfo, Context->Allocator, &OutShader->ObjectDescriptorPool));

    VkViewport Viewport;
    Viewport.x        = 0.0f;
    Viewport.y        = (r32)Context->FramebufferHeight;
    Viewport.width    =  (r32)Context->FramebufferWidth;
    Viewport.height   = -(r32)Context->FramebufferHeight;
    Viewport.minDepth = 0.0f;
    Viewport.maxDepth = 1.0f;

    VkRect2D Scissor;
    Scissor.offset.x = Scissor.offset.y = 0;
    Scissor.extent.width  = Context->FramebufferWidth;
    Scissor.extent.height = Context->FramebufferHeight;

    u32 Offset = 0;
    const s32 AttributeCount = 2;
    VkVertexInputAttributeDescription AttributeDescriptions[AttributeCount];

    VkFormat Formats[AttributeCount] = 
    {
        VK_FORMAT_R32G32B32_SFLOAT,
        VK_FORMAT_R32G32_SFLOAT,
    };

    u64 Sizes[AttributeCount] = 
    {
        sizeof(v3),
        sizeof(v2),
    };

    for(u32 AttributeIndex = 0;
        AttributeIndex < AttributeCount;
        ++AttributeIndex)
    {
        AttributeDescriptions[AttributeIndex].binding  = 0;
        AttributeDescriptions[AttributeIndex].location = AttributeIndex;
        AttributeDescriptions[AttributeIndex].format   = Formats[AttributeIndex];
        AttributeDescriptions[AttributeIndex].offset   = Offset;

        Offset += Sizes[AttributeIndex];
    }

    const s32 DescriptorSetLayoutCount = 2;
    VkDescriptorSetLayout Layouts[DescriptorSetLayoutCount] = 
    {
        OutShader->GlobalDescriptorSetLayout,
        OutShader->ObjectDescriptorSetLayout,
    };

    VkPipelineShaderStageCreateInfo StageCreateInfos[MATERIAL_SHADER_STAGE_COUNT];
    ZeroMemory(StageCreateInfos, sizeof(StageCreateInfos));
    for(u32 StageIndex = 0;
        StageIndex < MATERIAL_SHADER_STAGE_COUNT;
        ++StageIndex)
    {
        StageCreateInfos[StageIndex].sType = OutShader->Stages[StageIndex].ShaderStageCreateInfo.sType;
        StageCreateInfos[StageIndex] = OutShader->Stages[StageIndex].ShaderStageCreateInfo;
    }

    if(!VulkanGraphicsPipelineCreate(Context, &Context->MainRenderpass, 
                                     AttributeCount, AttributeDescriptions, 
                                     DescriptorSetLayoutCount, Layouts, 
                                     MATERIAL_SHADER_STAGE_COUNT, StageCreateInfos, 
                                     Viewport, Scissor, false, 
                                     &OutShader->Pipeline))
    {
        VENG_ERROR("Failed to load graphics pipeline for object shader");
        return false;
    }

    u32 DeviceLocalBits = Context->Device.SupportsDeviceLocalHostVisible ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : 0;
    if(!VulkanCreateBuffer(Context, sizeof(global_uniform_object) * 3, 
                           VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | DeviceLocalBits, 
                           true, &OutShader->GlobalUniformBuffer))
    {
        return false;
    }

    VkDescriptorSetLayout SetLayouts[3] = 
    {
        OutShader->GlobalDescriptorSetLayout,
        OutShader->GlobalDescriptorSetLayout,
        OutShader->GlobalDescriptorSetLayout,
    };

    VkDescriptorSetAllocateInfo SetAllocateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    SetAllocateInfo.descriptorPool = OutShader->GlobalDescriptorPool;
    SetAllocateInfo.descriptorSetCount = 3;
    SetAllocateInfo.pSetLayouts = SetLayouts;
    VK_CHECK(vkAllocateDescriptorSets(Context->Device.LogicalDevice, &SetAllocateInfo, OutShader->GlobalDescriptorSets));

    if(!VulkanCreateBuffer(Context, sizeof(local_uniform_object) * VULKAN_MAX_MATERIAL_COUNT, 
                           VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                           DeviceLocalBits | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                           true, &OutShader->ObjectUniformBuffer))
    {
        return false;
    }

    return true;
}

void VulkanMaterialShaderDestroy(vulkan_context* Context, vulkan_material_shader* Shader)
{
    VkDevice LogicalDevice = Context->Device.LogicalDevice;

    vkDestroyDescriptorPool(LogicalDevice, Shader->ObjectDescriptorPool, Context->Allocator);
    vkDestroyDescriptorSetLayout(LogicalDevice, Shader->ObjectDescriptorSetLayout, Context->Allocator);

    VulkanDestroyBuffer(Context, &Shader->GlobalUniformBuffer);
    VulkanDestroyBuffer(Context, &Shader->ObjectUniformBuffer);

    VulkanDestroyBuffer(Context, &Shader->GlobalUniformBuffer);

    VulkanGraphicsPipelineDestroy(Context, &Shader->Pipeline);

    vkDestroyDescriptorPool(LogicalDevice, Shader->GlobalDescriptorPool, Context->Allocator);
    vkDestroyDescriptorSetLayout(LogicalDevice, Shader->GlobalDescriptorSetLayout, Context->Allocator);

    VulkanGraphicsPipelineDestroy(Context, &Shader->Pipeline);

    for(u32 StageIndex = 0;
        StageIndex < MATERIAL_SHADER_STAGE_COUNT;
        ++StageIndex)
    {
        vkDestroyShaderModule(Context->Device.LogicalDevice, Shader->Stages[StageIndex].Handle, Context->Allocator);
        Shader->Stages[StageIndex].Handle = 0;
    }
}

void VulkanMaterialShaderUse(vulkan_context* Context, vulkan_material_shader* Shader)
{
    u32 ImageIndex = Context->ImageIndex;
    VulkanPipelineBind(&Context->GraphicsCommandBuffers[ImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, &Shader->Pipeline);
}

void VulkanMaterialShaderUpdateGlobalState(vulkan_context* Context, vulkan_material_shader* Shader, r32 DeltaTime)
{
    u32 ImageIndex = Context->ImageIndex;
    VkCommandBuffer CommandBuffer = Context->GraphicsCommandBuffers[ImageIndex].Handle;
    VkDescriptorSet Descriptor = Shader->GlobalDescriptorSets[ImageIndex];

    u32 Range = sizeof(global_uniform_object);
    u64 Offset = 0;

    VulkanBufferLoadData(Context, &Shader->GlobalUniformBuffer, Offset, Range, 0, &Shader->GlobalUBO);

    VkDescriptorBufferInfo BufferInfo;
    BufferInfo.buffer = Shader->GlobalUniformBuffer.Handle;
    BufferInfo.offset = Offset;
    BufferInfo.range = Range;

    VkWriteDescriptorSet DescriptorWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    DescriptorWrite.dstSet = Shader->GlobalDescriptorSets[ImageIndex];
    DescriptorWrite.dstBinding = 0;
    DescriptorWrite.dstArrayElement = 0;
    DescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    DescriptorWrite.descriptorCount = 1;
    DescriptorWrite.pBufferInfo = &BufferInfo;

    vkUpdateDescriptorSets(Context->Device.LogicalDevice, 1, &DescriptorWrite, 0, 0);
    Shader->DescriptorUpdated[ImageIndex] = true;

    vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Shader->Pipeline.PipelineLayout, 0, 1, &Descriptor, 0, 0);
}

void VulkanMaterialShaderSetModel(vulkan_context* Context, struct vulkan_material_shader* Shader, mat4 Model)
{
    if(Context && Shader)
    {
        u32 ImageIndex = Context->ImageIndex;
        VkCommandBuffer CommandBuffer = Context->GraphicsCommandBuffers[ImageIndex].Handle;

        vkCmdPushConstants(CommandBuffer, Shader->Pipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), &Model);
    }
}

void VulkanMaterialShaderApplyMaterial(vulkan_context* Context, struct vulkan_material_shader* Shader, material* Material)
{
    if(Context && Shader)
    {
        u32 ImageIndex = Context->ImageIndex;
        VkCommandBuffer CommandBuffer = Context->GraphicsCommandBuffers[ImageIndex].Handle;

        vulkan_object_shader_object_state* ObjectState = &Shader->ObjectStates[Material->InternalID];
        VkDescriptorSet ObjectDescriptorSet = ObjectState->DescriptorSets[ImageIndex];

        VkWriteDescriptorSet DescriptorWrites[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT];
        ZeroMemory(DescriptorWrites, sizeof(VkWriteDescriptorSet) * VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT);
        u32 DescriptorCount = 0;
        u32 DescriptorIndex = 0;

        u32 Range  = sizeof(local_uniform_object);
        u32 Offset = sizeof(local_uniform_object) * Material->InternalID;
        local_uniform_object OBO;

        OBO.DiffuseColor = Material->DiffuseColor;

        VulkanBufferLoadData(Context, &Shader->ObjectUniformBuffer, Offset, Range, 0, &OBO);

        u32* GlobalUBOGeneration = &ObjectState->DescriptorStates[DescriptorIndex].Generations[ImageIndex];
        if(*GlobalUBOGeneration == INVALID_ID || *GlobalUBOGeneration != Material->InternalID)
        {
            VkDescriptorBufferInfo BufferInfo;
            BufferInfo.buffer = Shader->ObjectUniformBuffer.Handle;
            BufferInfo.offset = Offset;
            BufferInfo.range  = Range;

            VkWriteDescriptorSet Descriptor = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            Descriptor.dstSet = ObjectDescriptorSet;
            Descriptor.dstBinding = DescriptorIndex;
            Descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            Descriptor.descriptorCount = 1;
            Descriptor.pBufferInfo = &BufferInfo;

            DescriptorWrites[DescriptorCount] = Descriptor;
            DescriptorCount++;

            *GlobalUBOGeneration = Material->Generation;
        }
        DescriptorIndex++;

        const u32 SamplerCount = 1;
        VkDescriptorImageInfo ImageInfos[SamplerCount];
        for(u32 SamplerIndex = 0;
            SamplerIndex < SamplerCount;
            ++SamplerIndex)
        {
            texture_use Use = Shader->SamplerUses[SamplerIndex];
            texture* Texture = 0;

            switch(Use)
            {
                case TEXTURE_USE_MAP_DIFFUSE:
                    Texture = Material->DiffuseMap.Texture;
                    break;
                default:
                    VENG_FATAL("Unable to bind sampler to unknown use.");
                    return;
            }

            u32* DescriptorGeneration = &ObjectState->DescriptorStates[DescriptorIndex].Generations[ImageIndex];
            u32* DescriptorID = &ObjectState->DescriptorStates[DescriptorIndex].IDs[ImageIndex];

            if(Texture->Generation == INVALID_ID)
            {
                Texture = TextureSystemGetDefaultTexture();
                *DescriptorGeneration = INVALID_ID;
            }

            if(Texture && (*DescriptorID != Texture->ID || *DescriptorGeneration != Texture->Generation || *DescriptorGeneration == INVALID_ID))
            {
                vulkan_texture* TextureData = (vulkan_texture*)Texture->Data;

                ImageInfos[SamplerIndex].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                ImageInfos[SamplerIndex].imageView = TextureData->Image.View;
                ImageInfos[SamplerIndex].sampler = TextureData->Sampler;

                VkWriteDescriptorSet Descriptor = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
                Descriptor.dstSet = ObjectDescriptorSet;
                Descriptor.dstBinding = DescriptorIndex;
                Descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                Descriptor.descriptorCount = 1;
                Descriptor.pImageInfo = &ImageInfos[SamplerIndex];

                DescriptorWrites[DescriptorCount] = Descriptor;
                DescriptorCount++;

                if(Texture->Generation != INVALID_ID)
                {
                    *DescriptorGeneration = Texture->Generation;
                    *DescriptorID = Texture->ID;
                }
                DescriptorIndex++;
            }
        }

        if(DescriptorCount > 0)
        {
            vkUpdateDescriptorSets(Context->Device.LogicalDevice, DescriptorCount, DescriptorWrites, 0, 0);
        }

        vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Shader->Pipeline.PipelineLayout, 1, 1, &ObjectDescriptorSet, 0, 0);
    }
}

b8 VulkanMaterialShaderAcquireResources(vulkan_context* Context, vulkan_material_shader* Shader, material* Mat)
{
    Mat->InternalID = Shader->ObjectUniformBufferIndex;
    Shader->ObjectUniformBufferIndex++;

    vulkan_object_shader_object_state* ObjectState = &Shader->ObjectStates[Mat->InternalID];
    for(u32 ObjectIndex = 0;
        ObjectIndex < VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT;
        ++ObjectIndex)
    {
        for(u32 DescriptorIndex = 0;
            DescriptorIndex < 3;
            ++DescriptorIndex)
        {
            ObjectState->DescriptorStates[ObjectIndex].Generations[DescriptorIndex] = INVALID_ID;
            ObjectState->DescriptorStates[ObjectIndex].IDs[DescriptorIndex] = INVALID_ID;
        }
    }

    VkDescriptorSetLayout Layouts[3] = 
    {
        Shader->ObjectDescriptorSetLayout,
        Shader->ObjectDescriptorSetLayout,
        Shader->ObjectDescriptorSetLayout,
    };

    VkDescriptorSetAllocateInfo AllocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    AllocInfo.descriptorPool = Shader->ObjectDescriptorPool;
    AllocInfo.descriptorSetCount = 3;
    AllocInfo.pSetLayouts = Layouts;
    VkResult AllocResult = vkAllocateDescriptorSets(Context->Device.LogicalDevice, &AllocInfo, ObjectState->DescriptorSets);
    if(AllocResult != VK_SUCCESS)
    {
        VENG_ERROR("Error allocating descriptor sets in shader");
        return false;
    }

    return true;
}

void VulkanMaterialShaderReleaseResources(vulkan_context* Context, vulkan_material_shader* Shader, material* Mat)
{
    vulkan_object_shader_object_state* ObjectState = Shader->ObjectStates + Mat->InternalID;

    vkDeviceWaitIdle(Context->Device.LogicalDevice);

    const u32 DescriptorSetCount = 3;
    VkResult Result = vkFreeDescriptorSets(Context->Device.LogicalDevice, Shader->ObjectDescriptorPool, DescriptorSetCount, ObjectState->DescriptorSets);
    if(Result != VK_SUCCESS)
    {
        VENG_ERROR("Error freeing object shader descriptor sets");
    }

    for(u32 ObjectIndex = 0;
        ObjectIndex < VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT;
        ++ObjectIndex)
    {
        for(u32 DescriptorIndex = 0;
            DescriptorIndex < 3;
            ++DescriptorIndex)
        {
            ObjectState->DescriptorStates[ObjectIndex].Generations[DescriptorIndex] = INVALID_ID;
            ObjectState->DescriptorStates[ObjectIndex].IDs[DescriptorIndex] = INVALID_ID;
        }
    }

    Mat->InternalID = INVALID_ID;
}


