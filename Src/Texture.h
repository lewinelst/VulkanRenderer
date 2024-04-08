#ifndef TEXTURE_H
#define TEXTURE_H

#include "stb_image.h"

#include "Image.h"

class Texture
{
public:
	Texture(std::string baseColorPath, VkDevice& device, VkPhysicalDevice& physicalDevice, VkCommandPool commandPool, VkQueue& graphicsQueue);
    void createTextureSampler(VkDevice& device, VkPhysicalDevice& physicalDevice);
    void destroyTexture();

    uint32_t mipLevels;

    VkSampler textureSampler;

    std::unique_ptr<Image> textureImage;

private:
};

Texture::Texture(std::string baseColorPath, VkDevice& device, VkPhysicalDevice& physicalDevice, VkCommandPool commandPool, VkQueue& graphicsQueue)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(baseColorPath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    VkDeviceSize imageSize = texWidth * texHeight * 4;

    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if (!pixels)
    {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    Buffer::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);

    stbi_image_free(pixels);

    textureImage = std::make_unique<Image>(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, device, physicalDevice);
    textureImage->transitionImageLayout(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels, commandPool, device, graphicsQueue);
    textureImage->copyBufferToImage(stagingBuffer, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), commandPool, device, graphicsQueue);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    textureImage->generateMipMaps(VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels, physicalDevice, device, commandPool, graphicsQueue);
    textureImage->createImageView(VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);

    createTextureSampler(device, physicalDevice);

}

void Texture::destroyTexture()
{
    vkDestroySampler(*textureImage->pDevice, textureSampler, nullptr);
    textureImage->destroyImage();

}

void Texture::createTextureSampler(VkDevice& device, VkPhysicalDevice& physicalDevice)
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    samplerInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(mipLevels);

    if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler");
    }

}

#endif // TEXTURE_H