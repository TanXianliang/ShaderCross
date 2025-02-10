#include <QStringList>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include "SPIRV-Reflect/spirv_reflect.h"
#include "spirvUtils.h"

bool DumpSpirVReflectionInfo(const QString &spvFilePath, QString &outputReflectionInfo)
{
    outputReflectionInfo = "";// 初始化空的spirvCode

    // 打开spv文件
    std::vector<uint32_t> spirvCode;

    QFile file(spvFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false; 
    }

    QByteArray byteArray = file.readAll();
    file.close();

    // 将字节数组转换为uint32_t向量
    const uint32_t *data = reinterpret_cast<const uint32_t *>(byteArray.data());
    size_t wordCount = byteArray.size() / sizeof(uint32_t);
    spirvCode.assign(data, data + wordCount);

    // 初始化
    SpvReflectShaderModule module;
    SpvReflectResult result = spvReflectCreateShaderModule(spirvCode.size() * sizeof(uint32_t), spirvCode.data(), &module);
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        return false;
    }

    // 获取描述符绑定信息
    uint32_t count = 0;
    spvReflectEnumerateDescriptorBindings(&module, &count, nullptr);
    std::vector<SpvReflectDescriptorBinding*> bindings(count);
    spvReflectEnumerateDescriptorBindings(&module, &count, bindings.data());

    outputReflectionInfo += "; Descriptor Bindings:\n";
    for (const auto* binding : bindings) {
        QString typeName;
        switch (binding->descriptor_type)
        {
            case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
                typeName = "Sampler";
                break;
            case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                typeName = "Combined Image Sampler";
                break;
            case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                typeName = "Sampled Image";
                break;
            case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                typeName = "Storage Image";
                break;
            case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                typeName = "Uniform Texel Buffer";
                break;
            case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                typeName = "Storage Texel Buffer";
                break;
            case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                typeName = "Uniform Buffer";
                break;
            case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                typeName = "Storage Buffer";
                break;
            case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                typeName = "Input Attachment";
                break;
            default:
                typeName = "Unknown";
                break;
        }

        if (binding->type_description->type_name == nullptr)
        {
            outputReflectionInfo += QString(";    Name: %1, Binding: %2, Set: %3, Descriptor Type: %4\n")
                .arg(binding->name)
                .arg(binding->binding)
                .arg(binding->set)
                .arg(typeName);
        }
        else
        {
            outputReflectionInfo += QString(";    Name: %1, Binding: %2, Set: %3, Descriptor Type: %4, Type Name: %5\n")
                .arg(binding->name)
                .arg(binding->binding)
                .arg(binding->set)
                .arg(typeName)
                .arg(binding->type_description->type_name);
        }
    }

    // 清理模块
    spvReflectDestroyShaderModule(&module);

    return true;
}