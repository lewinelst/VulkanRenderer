#ifndef SWAP_CHAIN_H
#define SWAP_CHAIN_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <algorithm>

#include <stdexcept>

#include "QueueFamily.h"
#include "Image.h"
#include "Device.h"

class SwapChain
{
public:
    SwapChain(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkDevice& device, GLFWwindow* window);
    void createFramebuffers(VkRenderPass& renderPass);
    void recreateSwapChain(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkDevice& device, GLFWwindow* window, VkRenderPass renderPass);
    void cleanupSwapChain();

    VkFormat findDepthFormat();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candididates, VkImageTiling  tiling, VkFormatFeatureFlags features);

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    std::unique_ptr<Image> colorImage;
    std::unique_ptr<Image> depthImage;

    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

private:
    void createSwapChain(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkDevice& device, GLFWwindow* window);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);
    void createImageViews();
    void createColorResources();
    void createDepthResources();

    VkDevice* pDevice = nullptr;
    VkPhysicalDevice* pPhysicalDevice = nullptr;
    GLFWwindow* pWindow = nullptr;
    VkSurfaceKHR* pSurface = nullptr;
};

SwapChain::SwapChain(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkDevice& device, GLFWwindow* window)
{
    createSwapChain(physicalDevice, surface, device, window);
}

void SwapChain::createSwapChain(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkDevice& device, GLFWwindow* window)
{
    msaaSamples = Device::getMaxUsableSampleCount(physicalDevice);
    SwapChainSupportDetails swapChainSupport = Device::querySwapChainSupport(physicalDevice, surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

    unsigned int imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamily::QueueFamilyIndices indices = QueueFamily::findQueueFamilies(physicalDevice, surface);
    unsigned int queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain");
    }

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    pDevice = &device;
    pPhysicalDevice = &physicalDevice;
    pWindow = window;
    pSurface = &surface;

    createImageViews();

    createColorResources();
    createDepthResources();
}

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
    if (capabilities.currentExtent.width != std::numeric_limits<unsigned int>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent =
        {
            static_cast<unsigned int>(width),
            static_cast<unsigned int>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void SwapChain::createImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());

    for (uint32_t i = 0; i < swapChainImages.size(); i++)
    {
        swapChainImageViews[i] = ImageView::createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, *pDevice);
    }

}

void SwapChain::createColorResources()
{
    VkFormat colorFormat = swapChainImageFormat;

    colorImage = std::make_unique<Image>(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *pDevice, *pPhysicalDevice);
    colorImage->createImageView(VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void SwapChain::createDepthResources()
{
    VkFormat depthFormat = findDepthFormat();
    depthImage = std::make_unique<Image>(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *pDevice, *pPhysicalDevice);
    depthImage->createImageView(VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

VkFormat SwapChain::findDepthFormat()
{
    return findSupportedFormat(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

VkFormat SwapChain::findSupportedFormat(const std::vector<VkFormat>& candididates, VkImageTiling  tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candididates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(*pPhysicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format");
}

void SwapChain::recreateSwapChain(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkDevice& device, GLFWwindow* window, VkRenderPass renderPass)
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(pWindow, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(pWindow, &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(*pDevice);

    cleanupSwapChain();

    createSwapChain(physicalDevice, surface, device, window);
    createFramebuffers(renderPass);
}

void SwapChain::createFramebuffers(VkRenderPass& renderPass)
{
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (unsigned int i = 0; i < swapChainImageViews.size(); i++)
    {
        std::array<VkImageView, 3> attachments =
        {
           colorImage->imageView,
           depthImage->imageView,
           swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(*pDevice, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer");
        }
    }
}

void SwapChain::cleanupSwapChain()
{
    colorImage->destroyImage();
    depthImage->destroyImage();

    for (int i = 0; i < swapChainFramebuffers.size(); i++)
    {
        vkDestroyFramebuffer(*pDevice, swapChainFramebuffers[i], nullptr);
    }

    for (int i = 0; i < swapChainImageViews.size(); i++)
    {
        vkDestroyImageView(*pDevice, swapChainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(*pDevice, swapChain, nullptr);
}

#endif // SWAP_CHAIN_H