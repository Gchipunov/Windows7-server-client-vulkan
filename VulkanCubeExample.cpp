#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <Windows.h>
#include <iostream>
using namespace std;
//#include "common.h"
#include <vector>

#define WIDTH 800
#define HEIGHT 600

// Global variables
VkInstance instance;
VkPhysicalDevice physicalDevice;
VkDevice device;
VkSurfaceKHR surface;
VkSwapchainKHR swapchain;
VkRenderPass renderPass;
VkPipeline pipeline;
VkCommandPool commandPool;
VkCommandBuffer commandBuffer;
VkBuffer vertexBuffer;
VkDeviceMemory vertexBufferMemory;
VkBuffer indexBuffer;
VkDeviceMemory indexBufferMemory;
VkQueue queue;

// Vertex structure
typedef struct {
    float x, y, z;
    float r, g, b;
} Vertex;

// Index buffer data
uint16_t indices[] = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
    0, 4, 5, 5, 1, 0,
    2, 6, 7, 7, 3, 2,
    0, 4, 7, 7, 3, 0,
    1, 5, 6, 6, 2, 1
};

// Vertex buffer data
Vertex vertices[] = {
    {-1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 0.0f},
    { 1.0f, -1.0f,  1.0f, 0.0f, 1.0f, 0.0f},
    { 1.0f,  1.0f,  1.0f, 0.0f, 0.0f, 1.0f},
    {-1.0f,  1.0f,  1.0f, 1.0f, 1.0f, 1.0f},
    {-1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f},
    { 1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f},
    { 1.0f,  1.0f, -1.0f, 1.0f, 1.0f, 0.0f},
    {-1.0f,  1.0f, -1.0f, 0.0f, 0.0f, 0.0f}
};

// Create instance
VkResult createInstance() {
    VkApplicationInfo appInfo; // = {0};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Cube";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo; // = {0};
	memset(&createInfo, 0, sizeof(VkInstanceCreateInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
   // createInfo.pApplicationInfo = &appInfo;
   // createInfo.enabledExtensionCount = 0;
	const char* enabledExtensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };
    createInfo.enabledExtensionCount = 2;
    createInfo.ppEnabledExtensionNames = enabledExtensions;

    return vkCreateInstance(&createInfo, NULL, &instance);
}

// Create surface
/*
VkResult createSurface(HWND hwnd) {
    VkWin32SurfaceCreateInfoKHR createInfo;// = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = hwnd;
    createInfo.hinstance = GetModuleHandle(NULL);

    return vkCreateWin32SurfaceKHR(instance, &createInfo, NULL, &surface);
}*/
VkResult createSurface(HWND hwnd) {
    if (!IsWindow(hwnd)) {
        std::cerr << "Invalid HWND" << std::endl;
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.hwnd = hwnd;
    createInfo.hinstance = GetModuleHandle(nullptr);

    VkResult result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create surface: " << result << std::endl;
    }
    return result;
}


// Create device
VkResult createDevice() {
    uint32_t deviceCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (result != VK_SUCCESS || deviceCount == 0) {
        std::cerr << "Failed to enumerate physical devices." << std::endl;
        return result == VK_SUCCESS ? VK_ERROR_INITIALIZATION_FAILED : result;
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    // In a real application, you would have logic to select the best physical device
    physicalDevice = devices[0]; // Placeholder: Replace with proper selection

    // Find a suitable queue family index (for graphics and potentially presentation)
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    int graphicsQueueFamilyIndex = -1;
    for (int i = 0; i < queueFamilies.size(); ++i) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsQueueFamilyIndex = i;
            break;
        }
    }

    if (graphicsQueueFamilyIndex == -1) {
        std::cerr << "Failed to find a graphics queue family." << std::endl;
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    // Enable device extensions
    const char* deviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.enabledExtensionCount = 1; // Now it's 1
    createInfo.ppEnabledExtensionNames = deviceExtensions;

    return vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
}

/*
VkResult createDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    VkPhysicalDevice devices[36];
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);
    physicalDevice = devices[0];

    float priority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = 0;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &priority;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	///////////////////////page 2
	//createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
createInfo.queueCreateInfoCount = 1;
createInfo.pQueueCreateInfos = &queueCreateInfo;
createInfo.enabledExtensionCount = 0;

return vkCreateDevice(physicalDevice, &createInfo, NULL, &device);
}
*/
// Create swapchain
/*
VkResult createSwapchain(HWND hwnd) {
VkSurfaceCapabilitiesKHR surfaceCapabilities;
vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

VkSwapchainCreateInfoKHR createInfo = {};
createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
createInfo.surface = surface;
createInfo.minImageCount = surfaceCapabilities.minImageCount;
createInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
createInfo.imageExtent = surfaceCapabilities.currentExtent;
createInfo.imageArrayLayers = 1;
createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

return vkCreateSwapchainKHR(device, &createInfo, NULL, &swapchain);
}*/
VkResult createSwapchain(HWND hwnd) {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to get surface capabilities: " << result << std::endl;
        return result;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = surfaceCapabilities.minImageCount;
    createInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = surfaceCapabilities.currentExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    return vkCreateSwapchainKHR(device, &createInfo, NULL, &swapchain);
}
// Create render pass
VkResult createRenderPass() {
VkAttachmentDescription attachmentDescription = {};
attachmentDescription.format = VK_FORMAT_B8G8R8A8_UNORM;
attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

VkAttachmentReference attachmentReference = {};
attachmentReference.attachment = 0;
attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

VkSubpassDescription subpassDescription = {};
subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
subpassDescription.colorAttachmentCount = 1;
subpassDescription.pColorAttachments = &attachmentReference;

VkRenderPassCreateInfo createInfo = {};
createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
createInfo.attachmentCount = 1;
createInfo.pAttachments = &attachmentDescription;
createInfo.subpassCount = 1;
createInfo.pSubpasses = &subpassDescription;

return vkCreateRenderPass(device, &createInfo, NULL, &renderPass);
}

// Create pipeline
VkResult createPipeline2() {
VkGraphicsPipelineCreateInfo createInfo = {};
createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
VkVertexInputBindingDescription bindingDescription = {};
bindingDescription.binding = 0;
bindingDescription.stride = sizeof(Vertex);
bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
VkVertexInputAttributeDescription attributeDescriptions[2] = {};
attributeDescriptions[0].binding = 0;
attributeDescriptions[0].location = 0;
attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
attributeDescriptions[0].offset = offsetof(Vertex, x);
attributeDescriptions[1].binding = 0;
attributeDescriptions[1].location = 1;
attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
attributeDescriptions[1].offset = offsetof(Vertex, r);
vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 2;
vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions;
createInfo.pVertexInputState = &vertexInputStateCreateInfo;

VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
createInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;

VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
viewportStateCreateInfo.viewportCount = 1;
VkViewport viewport = {};
viewport.x = 0.0f;
viewport.y = 0.0f;
viewport.width = WIDTH;
viewport.height = HEIGHT;
viewport.minDepth = 0.0f;
viewport.maxDepth = 1.0f;
viewportStateCreateInfo.pViewports = &viewport;
viewportStateCreateInfo.scissorCount = 1;
VkRect2D scissor = {};
scissor.offset.x = 0;
scissor.offset.y = 0;
//scissor.extent.width =
// ////////   page 3
scissor.extent.width = WIDTH;
scissor.extent.height = HEIGHT;
viewportStateCreateInfo.pScissors = &scissor;
createInfo.pViewportState = &viewportStateCreateInfo;

VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
rasterizationStateCreateInfo.lineWidth = 1.0f;
createInfo.pRasterizationState = &rasterizationStateCreateInfo;

VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
createInfo.pMultisampleState = &multisampleStateCreateInfo;

VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
colorBlendStateCreateInfo.attachmentCount = 1;
colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
createInfo.pColorBlendState = &colorBlendStateCreateInfo;

VkPipelineLayout pipelineLayout;
VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, NULL, &pipelineLayout);
createInfo.layout = pipelineLayout;

createInfo.renderPass = renderPass;
createInfo.subpass = 0;

//VkPipelineCache pipelineCache;

//return vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, NULL, &pipeline);
// pipelineCache
return vkCreateGraphicsPipelines(device, (VkPipelineCache)NULL, 1, &createInfo, NULL, &pipeline);
}
/*
// Create vertex buffer
VkResult createVertexBuffer() {
VkBufferCreateInfo bufferCreateInfo = {};
bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
bufferCreateInfo.size = sizeof(vertices);
bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

VkMemoryRequirements memoryRequirements;
vkGetBufferMemoryRequirements(device, vertexBuffer, &memoryRequirements);

VkMemoryAllocateInfo memoryAllocateInfo = {};
memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
memoryAllocateInfo.allocationSize = memoryRequirements.size;
memoryAllocateInfo.memoryTypeIndex = 0;

vkAllocateMemory(device, &memoryAllocateInfo, NULL, &vertexBufferMemory);
vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);

void* data;
vkMapMemory(device, vertexBufferMemory, 0, sizeof(vertices), 0, &data);
memcpy(data, vertices, sizeof(vertices));
vkUnmapMemory(device, vertexBufferMemory);

return VK_SUCCESS;
}*/
/*
You've made progress in the createVertexBuffer function, but there are still a few critical issues that could lead to the access violation you were encountering and prevent it from working correctly.

Here's a breakdown of the problems and how to fix them:

Issues and Solutions:

    vertexBuffer is Not Created:
        Problem: You are calling vkGetBufferMemoryRequirements on vertexBuffer before you have actually created the buffer using vkCreateBuffer. vertexBuffer will likely be uninitialized or VK_NULL_HANDLE at this point.
        Solution: You need to call vkCreateBuffer first to get a valid VkBuffer handle.

    vertexBuffer is a Local Variable:
        Problem: It seems vertexBuffer is likely a local variable within the function where you intend to create it. You need it to persist outside this function so you can use it for binding memory and in drawing commands.
        Solution: Declare vertexBuffer as a global variable (like instance, physicalDevice, device, etc.) or as a member of a structure that manages your Vulkan resources.

    Memory Type Index is Hardcoded to 0:
        Problem: You are setting memoryAllocateInfo.memoryTypeIndex = 0;. This assumes that the very first memory type available on your system is suitable for a vertex buffer that needs to be host-visible for data upload. This is almost certainly incorrect.
        Solution: You need to query the physical device's memory properties (VkPhysicalDeviceMemoryProperties) and find a memory type that satisfies both the buffer's memory requirements (memoryRequirements.memoryTypeBits) and the desired memory properties (e.g., VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT and VK_MEMORY_PROPERTY_HOST_COHERENT_BIT for direct mapping).

    Error Handling: You are not checking the return values of any of the Vulkan functions. This makes it very difficult to diagnose errors.
        Solution: Check the VkResult returned by vkCreateBuffer, vkAllocateMemory, and vkMapMemory. If any of them are not VK_SUCCESS, handle the error appropriately (log it, return an error code, and potentially clean up any resources that were partially created).

Corrected createVertexBuffer Function:
C++
	*/
//VkBuffer vertexBuffer;        // Declare as a global or member variable
//VkDeviceMemory vertexBufferMemory; // Declare as a global or member variable

VkResult createVertexBuffer() {
    // 1. Create the Vertex Buffer
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = sizeof(vertices); // Assuming 'vertices' is defined elsewhere
    bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(device, &bufferCreateInfo, NULL, &vertexBuffer);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create vertex buffer: " << result << std::endl;
        return result;
    }

    // 2. Get Memory Requirements
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, vertexBuffer, &memoryRequirements);

    // 3. Find a Suitable Memory Type
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    uint32_t memoryTypeIndex = -1;
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        if ((memoryRequirements.memoryTypeBits & (1 << i)) &&
            (memoryProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))) {
            memoryTypeIndex = i;
            break;
        }
    }

    if (memoryTypeIndex == -1) {
        std::cerr << "Failed to find suitable memory type for vertex buffer." << std::endl;
        vkDestroyBuffer(device, vertexBuffer, nullptr); // Cleanup
        return VK_ERROR_FEATURE_NOT_PRESENT; // Or a more appropriate error code
    }

    // 4. Allocate Memory
    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

    result = vkAllocateMemory(device, &memoryAllocateInfo, NULL, &vertexBufferMemory);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to allocate vertex buffer memory: " << result << std::endl;
        vkDestroyBuffer(device, vertexBuffer, nullptr); // Cleanup
        return result;
    }

    // 5. Bind Memory to Buffer
    result = vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to bind vertex buffer memory: " << result << std::endl;
        vkFreeMemory(device, vertexBufferMemory, nullptr); // Cleanup
        vkDestroyBuffer(device, vertexBuffer, nullptr);   // Cleanup
        return result;
    }

    // 6. Map Memory and Copy Data
    void* data;
    result = vkMapMemory(device, vertexBufferMemory, 0, sizeof(vertices), 0, &data);
    if (result == VK_SUCCESS) {
        memcpy(data, vertices, sizeof(vertices));
        vkUnmapMemory(device, vertexBufferMemory);
    } else {
        std::cerr << "Failed to map vertex buffer memory for writing: " << result << std::endl;
        vkFreeMemory(device, vertexBufferMemory, nullptr); // Cleanup
        vkDestroyBuffer(device, vertexBuffer, nullptr);   // Cleanup
        return result;
    }

    return VK_SUCCESS;
}
/*
// Create index buffer
VkResult createIndexBuffer() {
VkBufferCreateInfo bufferCreateInfo = {};
bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
bufferCreateInfo.size = sizeof(indices);
bufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

VkMemoryRequirements memoryRequirements;
vkGetBufferMemoryRequirements(device, indexBuffer, &memoryRequirements);

VkMemoryAllocateInfo memoryAllocateInfo = {};
memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
memoryAllocateInfo.allocationSize = memoryRequirements.size;
memoryAllocateInfo.memoryTypeIndex = 0;

vkAllocateMemory(device, &memoryAllocateInfo, NULL, &indexBufferMemory);
vkBindBufferMemory(device, indexBuffer, indexBufferMemory, 0);

void* data;
vkMapMemory(device, indexBufferMemory, 0, sizeof(indices), 0, &data);
memcpy(data, indices, sizeof(indices));
vkUnmapMemory(device, indexBufferMemory);

return VK_SUCCESS;
}*/
/*
The "Access violation reading location 0xFFFFFFFFFFFFFFFF" error on vkUnmapMemory strongly suggests that the memory you are trying to unmap (indexBufferMemory) was either not successfully mapped in the first place (meaning the pointer data might be invalid), or the device handle is invalid.

Let's analyze your createIndexBuffer function, which looks very similar to the problematic createVertexBuffer you previously had, and pinpoint the likely issues:

Likely Issues (Similar to Vertex Buffer):

    indexBuffer Not Created: You are calling vkGetBufferMemoryRequirements on indexBuffer before creating it with vkCreateBuffer. indexBuffer will likely be uninitialized or VK_NULL_HANDLE.

    indexBuffer is a Local Variable: Similar to the vertex buffer, indexBuffer needs to persist outside this function.

    Memory Type Index Hardcoded to 0: You are still assuming the first memory type is suitable for an index buffer that needs to be host-visible for data upload.

    Missing Error Handling: You are not checking the return values of any Vulkan functions, making debugging difficult.

Corrected createIndexBuffer Function:
C++
*/
//VkBuffer indexBuffer;        // Declare as a global or member variable
//VkDeviceMemory indexBufferMemory; // Declare as a global or member variable

VkResult createIndexBuffer() {
    // 1. Create the Index Buffer
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = sizeof(indices); // Assuming 'indices' is defined elsewhere
    bufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(device, &bufferCreateInfo, NULL, &indexBuffer);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create index buffer: " << result << std::endl;
        return result;
    }

    // 2. Get Memory Requirements
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, indexBuffer, &memoryRequirements);

    // 3. Find a Suitable Memory Type
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    uint32_t memoryTypeIndex = -1;
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        if ((memoryRequirements.memoryTypeBits & (1 << i)) &&
            (memoryProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))) {
            memoryTypeIndex = i;
            break;
        }
    }

    if (memoryTypeIndex == -1) {
        std::cerr << "Failed to find suitable memory type for index buffer." << std::endl;
        vkDestroyBuffer(device, indexBuffer, nullptr); // Cleanup
        return VK_ERROR_FEATURE_NOT_PRESENT; // Or a more appropriate error code
    }

    // 4. Allocate Memory
    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

    result = vkAllocateMemory(device, &memoryAllocateInfo, NULL, &indexBufferMemory);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to allocate index buffer memory: " << result << std::endl;
        vkDestroyBuffer(device, indexBuffer, nullptr); // Cleanup
        return result;
    }

    // 5. Bind Memory to Buffer
    result = vkBindBufferMemory(device, indexBuffer, indexBufferMemory, 0);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to bind index buffer memory: " << result << std::endl;
        vkFreeMemory(device, indexBufferMemory, nullptr); // Cleanup
        vkDestroyBuffer(device, indexBuffer, nullptr);   // Cleanup
        return result;
    }

    // 6. Map Memory and Copy Data
    void* data;
    result = vkMapMemory(device, indexBufferMemory, 0, sizeof(indices), 0, &data);
    if (result == VK_SUCCESS) {
        memcpy(data, indices, sizeof(indices));
        vkUnmapMemory(device, indexBufferMemory);
    } else {
        std::cerr << "Failed to map index buffer memory for writing: " << result << std::endl;
        vkFreeMemory(device, indexBufferMemory, nullptr); // Cleanup
        vkDestroyBuffer(device, indexBuffer, nullptr);   // Cleanup
        return result;
    }

    return VK_SUCCESS;
}
// VkPipeline pipeline;
 VkPipelineLayout pipelineLayout; // You'll need to create this

VkResult createPipeline() {
    // Assume shader modules (vertShaderModule, fragShaderModule) and renderPass are already created

    // Vertex Input State (example - adjust based on your vertex data)
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    // Define vertex bindings and attributes here

    // Input Assembly State
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewport and Scissor (dynamic or fixed)
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)WIDTH;
    viewport.height = (float)HEIGHT;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

   // VkRect2D scissor = {};
   // scissor.offset = { 0, 0 };
   // scissor.extent = { WIDTH, HEIGHT };
	VkRect2D scissor = {};
scissor.offset.x = 0;
scissor.offset.y = 0;
scissor.extent.width = WIDTH;
scissor.extent.height = HEIGHT;



    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Rasterization State
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    // Multisampling State
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Color Blending State
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // Pipeline Layout (assuming you have created pipelineLayout)
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // Define descriptors and push constants here
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        std::cerr << "Failed to create pipeline layout!" << std::endl;
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Graphics Pipeline Creation
    VkPipelineShaderStageCreateInfo shaderStages[2];
    // Initialize shaderStages with your vertex and fragment shader modules

	// Graphics Pipeline Creation
   // VkPipelineShaderStageCreateInfo shaderStages[2];

    // Assuming vertShaderModule and fragShaderModule are global VkShaderModule handles
    if (vertShaderModule == VK_NULL_HANDLE || fragShaderModule == VK_NULL_HANDLE) {
        std::cerr << "Error: Vertex or fragment shader module is VK_NULL_HANDLE!" << std::endl;
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    shaderStages[0] = vertShaderStageInfo;
    shaderStages[1] = fragShaderStageInfo;

    // ... (rest of pipelineInfo setup) ...


    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Optional
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = (VkPipeline)VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(device, (VkPipelineCache)VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
        std::cerr << "Failed to create graphics pipeline!" << std::endl;
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    return VK_SUCCESS;
}

int main() {

	uint32_t extensionCount = 0;
vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
std::vector<VkExtensionProperties> extensions(extensionCount);
vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
for (const auto& ext : extensions) {
    std::cout << ext.extensionName << std::endl;
}

    // Create window
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = DefWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
   // wc
	//     ////// page 4
	wc.lpszMenuName = NULL;
wc.lpszClassName = "VulkanCube";
wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
RegisterClassEx(&wc);

HWND hwnd = CreateWindowEx(
    0,
    "VulkanCube",
    "Vulkan Cube",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    WIDTH,
    HEIGHT,
    NULL,
    NULL,
    hInstance,
    NULL
);

ShowWindow(hwnd, SW_SHOW);
UpdateWindow(hwnd);

// Create Vulkan instance
VkResult vkResultcreateInstance = createInstance();
VkResult vkResultcreateSurface = createSurface(hwnd);
VkResult vkResultcreateDevice = createDevice();
VkResult vkResultcreateSwapchain = createSwapchain(hwnd);
VkResult vkResultcreateRenderPass = createRenderPass();
VkResult vkResultcreatePipeline = createPipeline();

// Create vertex buffer
VkBufferCreateInfo vertexBufferCreateInfo = {};
vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
vertexBufferCreateInfo.size = sizeof(vertices);
vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
vertexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
vkCreateBuffer(device, &vertexBufferCreateInfo, NULL, &vertexBuffer);
createVertexBuffer();

// Create index buffer
VkBufferCreateInfo indexBufferCreateInfo = {};
indexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
indexBufferCreateInfo.size = sizeof(indices);
indexBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
indexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
vkCreateBuffer(device, &indexBufferCreateInfo, NULL, &indexBuffer);
createIndexBuffer();

// Create command pool
VkCommandPoolCreateInfo commandPoolCreateInfo = {};
commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
commandPoolCreateInfo.queueFamilyIndex = 0;
vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, &commandPool);

// Create command buffer
VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
commandBufferAllocateInfo.commandPool = commandPool;
commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
commandBufferAllocateInfo.commandBufferCount = 1;
vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer);

// Get queue
vkGetDeviceQueue(device, 0, 0, &queue);

// Main loop
while (1) {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Begin command buffer
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

    // Render pass
    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
	//VkFramebuffer vVkFramebuffer;
   // renderPassBeginInfo.framebuffer = VK_NULL_HANDLE; // You need to create a framebuffer
	 renderPassBeginInfo.framebuffer = (VkFramebuffer)NULL ;//vVkFramebuffer;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = WIDTH;
    renderPassBeginInfo.renderArea.extent.height = HEIGHT;
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Bind pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    // Bind vertex buffer
    VkBuffer vertexBuffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    // Bind index buffer
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    // Draw
    vkCmdDrawIndexed(commandBuffer, 36, 1, 0, 0, 0);

    // End render pass
    vkCmdEndRenderPass(commandBuffer);

    // End command buffer
    vkEndCommandBuffer(commandBuffer);

    // Submit command buffer
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
	VkFence vVkFence;

    vkQueueSubmit(queue, 1, &submitInfo, vVkFence);

    // Wait for queue to finish
    vkQueueWaitIdle(queue);
}

return 0;
}
