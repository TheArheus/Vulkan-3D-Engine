#include "vulkan_shader_utils.h"

#include "core/vstring.h"
#include "core/vmemory.h"
#include "core/logger.h"

#include "platform/file_system.h"

b8 CreateShaderModule(vulkan_context* Context, 
                      const char* Name, const char* TypeStr, 
                      VkShaderStageFlagBits ShaderStageBit, 
                      u32 StageIndex, vulkan_shader_stage* ShaderStages)
{
    char FileName[512];
    StringFormat(FileName, "shaders/%s.%s.spv", Name, TypeStr);

    ZeroMemory(&ShaderStages[StageIndex].CreateInfo, sizeof(VkShaderModuleCreateInfo));
    ShaderStages[StageIndex].CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    file_handle Handle;
    if(!FileOpen(FileName, FILE_MODE_READ, true, &Handle))
    {
        VENG_ERROR("Unable to read shader module: %s", FileName);
        return false;
    }

    u64 Size = 0;
    u8* FileBuffer = 0;
    if(!FileReadAllBytes(&Handle, &FileBuffer, &Size))
    {
        return false;
    }

    ShaderStages[StageIndex].CreateInfo.codeSize = Size;
    ShaderStages[StageIndex].CreateInfo.pCode    = (u32*)FileBuffer;

    FileClose(&Handle);

    VK_CHECK(vkCreateShaderModule(Context->Device.LogicalDevice, &ShaderStages[StageIndex].CreateInfo, Context->Allocator, &ShaderStages[StageIndex].Handle));

    ZeroMemory(&ShaderStages[StageIndex].ShaderStageCreateInfo, sizeof(VkPipelineShaderStageCreateInfo));
    ShaderStages[StageIndex].ShaderStageCreateInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    ShaderStages[StageIndex].ShaderStageCreateInfo.stage  = ShaderStageBit;
    ShaderStages[StageIndex].ShaderStageCreateInfo.module = ShaderStages[StageIndex].Handle;
    ShaderStages[StageIndex].ShaderStageCreateInfo.pName  = "main";

    return true;
}

