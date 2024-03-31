#ifndef MODEL_H
#define MODEL_H

#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <string>

#include "Vertex.h"
#include "Buffer.h"

#include "tiny_obj_loader.h"

class Model
{
public:
    Model(std::string& modelPath, VkDevice& device, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue, VkCommandPool& commandPool);

    void destroyModel();

    void createVertexBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue, VkCommandPool& commandPool);
    void createIndexBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue, VkCommandPool& commandPool);

    std::vector<uint32_t> indices;

    VkBuffer vertexBuffer = NULL;
    VkBuffer indexBuffer = NULL;

private:
    VkDeviceMemory vertexBufferMemory = NULL;
    VkDeviceMemory indexBufferMemory = NULL;

    VkDevice* pDevice = nullptr;

    std::vector<Vertex> vertices;
};

Model::Model(std::string& modelPath, VkDevice& device, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue, VkCommandPool& commandPool)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str()))
    {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex{};

            vertex.pos =
            {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2],
            };

            vertex.normal =
            {
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2],
            };

            vertex.texCoord =
            {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = { 1.0f, 1.0f, 1.0f };


            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            indices.push_back(uniqueVertices[vertex]);
        }
    }

    createVertexBuffer(device, physicalDevice, graphicsQueue, commandPool);
    createIndexBuffer(device, physicalDevice, graphicsQueue, commandPool);
}

void Model::destroyModel()
{
    vkDestroyBuffer(*pDevice, indexBuffer, nullptr);
    vkFreeMemory(*pDevice, indexBufferMemory, nullptr);

    vkDestroyBuffer(*pDevice, vertexBuffer, nullptr);
    vkFreeMemory(*pDevice, vertexBufferMemory, nullptr);
}

void Model::createVertexBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue, VkCommandPool& commandPool)
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    Buffer::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    Buffer::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory, device, physicalDevice);

    Buffer::copyBuffer(stagingBuffer, vertexBuffer, bufferSize, graphicsQueue, commandPool, device);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    pDevice = &device;
}

void Model::createIndexBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue, VkCommandPool& commandPool)
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    Buffer::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    Buffer::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory, device, physicalDevice);

    Buffer::copyBuffer(stagingBuffer, indexBuffer, bufferSize, graphicsQueue, commandPool, device);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

#endif // MODEL_H
