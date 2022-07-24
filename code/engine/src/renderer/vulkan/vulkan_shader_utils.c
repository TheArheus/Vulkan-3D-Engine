#include "vulkan_shader_utils.h"

#include "core/vstring.h"
#include "core/vmemory.h"
#include "core/logger.h"

#include "systems/resource_system.h"

b8 CreateShaderModule(vulkan_context* Context, 
                      const char* Name, const char* TypeStr, 
                      VkShaderStageFlagBits ShaderStageBit, 
                      u32 StageIndex, vulkan_shader_stage* ShaderStages)
{
    char FileName[512];
    StringFormat(FileName, "shaders/%s.%s.spv", Name, TypeStr);

    resource BinaryResource;
    if(!ResourceSystemLoad(FileName, RESOURCE_TYPE_BINARY, &BinaryResource))
    {
        VENG_ERROR("Unable to read shader module");
        return false;
    }

    ZeroMemory(&ShaderStages[StageIndex].CreateInfo, sizeof(VkShaderModuleCreateInfo));
    ShaderStages[StageIndex].CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ShaderStages[StageIndex].CreateInfo.codeSize = BinaryResource.DataSize;
    ShaderStages[StageIndex].CreateInfo.pCode = (u32*)BinaryResource.Data;

    VK_CHECK(vkCreateShaderModule(Context->Device.LogicalDevice, &ShaderStages[StageIndex].CreateInfo, Context->Allocator, &ShaderStages[StageIndex].Handle));

    ResourceSystemUnload(&BinaryResource);

    ZeroMemory(&ShaderStages[StageIndex].ShaderStageCreateInfo, sizeof(VkPipelineShaderStageCreateInfo));
    ShaderStages[StageIndex].ShaderStageCreateInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    ShaderStages[StageIndex].ShaderStageCreateInfo.stage  = ShaderStageBit;
    ShaderStages[StageIndex].ShaderStageCreateInfo.module = ShaderStages[StageIndex].Handle;
    ShaderStages[StageIndex].ShaderStageCreateInfo.pName  = "main";

    return true;
}

