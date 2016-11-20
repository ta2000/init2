#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// From VulkanSDK examples
#include "linmath.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const _Bool validationEnabled = 1;

struct Vertex
{
    float position[2];
    float color[3];
};
VkVertexInputBindingDescription getBindingDescription()
{
    VkVertexInputBindingDescription description;
    description.binding = 0;
    description.stride = sizeof(struct Vertex);
    description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return description;
}
VkVertexInputAttributeDescription* getAttributeDescriptions()
{
    VkVertexInputAttributeDescription* descriptions;
    descriptions = calloc(2, sizeof(VkVertexInputAttributeDescription));

    // Position
    descriptions[0].location = 0;
    descriptions[0].binding = 0;
    descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    descriptions[0].offset = offsetof(struct Vertex, position);

    // Color
    descriptions[1].location = 1;
    descriptions[1].binding = 0;
    descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    descriptions[1].offset = offsetof(struct Vertex, color);

    return descriptions;
}

struct UniformBufferObject
{
    mat4x4 model;
    mat4x4 view;
    mat4x4 proj;
};

struct QueueFamilyIndices
{
    int graphicsFamily;
    int presentFamily;
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    uint32_t formatCount;
    VkSurfaceFormatKHR* formats;
    uint32_t presentModeCount;
    VkPresentModeKHR* presentModes;
};
void freeSwapChainSupportDetails(struct SwapChainSupportDetails* details);

struct Engine
{
    // Window
    GLFWwindow* window;

    // Vulkan instance
    VkInstance instance;

    // Validation layers
    char* surfaceExtensions[64];
    uint32_t surfaceExtensionCount;
    char* deviceExtensions[64];
    uint32_t deviceExtensionCount;
    VkDebugReportCallbackEXT debugCallback;
    PFN_vkCreateDebugReportCallbackEXT createDebugCallback;
    PFN_vkDestroyDebugReportCallbackEXT destroyDebugCallback;

    // Surface
    VkSurfaceKHR surface;

    // Queues
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    struct QueueFamilyIndices queueFamilyIndices;

    // Physical/logical device
    VkPhysicalDevice physicalDevice;
    VkDevice device;

    // Swapchain/images
    VkSwapchainKHR swapChain;
    uint32_t imageCount;
    VkImage* swapChainImages;
    VkImageView* imageViews;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    struct SwapChainSupportDetails swapChainDetails;

    // Render pass
    VkRenderPass renderPass;

    // Graphics pipeline
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    // Frame buffers
    VkFramebuffer* framebuffers;

    // Command pool
    VkCommandPool commandPool;

    // Vertex buffer
    uint32_t vertexCount;
    struct Vertex* vertices;
    uint16_t* indices;
    uint32_t indexCount;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    // Uniform buffer
    VkBuffer uniformStagingBuffer;
    VkDeviceMemory uniformStagingBufferMemory;
    VkBuffer uniformBuffer;
    VkDeviceMemory uniformBufferMemory;

    // Descriptor pool/set
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet descriptorSet;
    VkDescriptorPool descriptorPool;

    // Command buffers
    VkCommandBuffer* commandBuffers;

    // Semaphores
    VkSemaphore imageAvailable;
    VkSemaphore renderFinished;
};

/*  -----------------------------
 *  --- Forward declarations ----
 *  -----------------------------   */
// INSTANCE
void createInstance(struct Engine* engine);
void destroyInstance(struct Engine* engine);

// VALIDATION SUPPORT
_Bool checkValidationSupport(
    uint32_t validationLayerCount,
    const char** validationLayers
);

// DEVICE EXTENSIONS
void getRequiredExtensions(struct Engine* engine);
void freeExtensions(struct Engine* engine);

// DEBUG CALLBACK
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objType,
    uint64_t obj,
    size_t location,
    int32_t code,
    const char* layerPrefix,
    const char* msg,
    void* userData
);
void setupDebugCallback(struct Engine* engine);
void destroyDebugCallback(struct Engine* engine);

// WINDOW SURFACE
void createSurface(struct Engine* engine);
void destroySurface(struct Engine* engine);

// PHYSICAL DEVICE (GPU)
void getPhysicalDevice(struct Engine* engine);
_Bool isDeviceSuitable(struct Engine* engine, VkPhysicalDevice* physicalDevice);
_Bool checkDeviceExtensionSupport(VkPhysicalDevice* physicalDevice);
struct SwapChainSupportDetails querySwapChainSupport(
    struct Engine* engine,
    VkPhysicalDevice* physicalDevice
);
struct QueueFamilyIndices findQueueFamilies(
    struct Engine* engine,
    VkPhysicalDevice* physicalDevice
);
_Bool queueFamilyComplete(struct QueueFamilyIndices* queueFamilyIndices);

// LOGICAL DEVICE
void createLogicalDevice(struct Engine* engine);
void destroyLogicalDevice(struct Engine* engine);

// SWAPCHAIN
void createSwapChain(struct Engine* engine);
void destroySwapChain(struct Engine* engine);
VkSurfaceFormatKHR chooseSwapSurfaceFormat(
    VkSurfaceFormatKHR* availableFormats,
    int formatCount
);
VkPresentModeKHR chooseSwapPresentMode(
    VkPresentModeKHR* availablePresentModes,
    int presentModeCount
);
VkExtent2D chooseSwapExtent(
    const VkSurfaceCapabilitiesKHR* capabilities
);

// IMAGE VIEWS
void createImageViews(struct Engine* engine);
void destroyImageViews(struct Engine* engine);

// RENDER PASS
void createRenderPass(struct Engine* engine);
void destroyRenderPass(struct Engine* engine);

// GRAPHICS PIPELINE
void createGraphicsPipeline(struct Engine* engine);
void destroyGraphicsPipeline(struct Engine* engine);
char* readFile(const char* fname, uint32_t* fsize);
void createShaderModule(
    struct Engine* engine,
    char* code,
    uint32_t codeSize,
    VkShaderModule* shaderModule
);

// FRAMEBUFFERS
void createFramebuffers(struct Engine* engine);
void destroyFramebuffers(struct Engine* engine);

// COMMAND POOL
void createCommandPool(struct Engine* engine);
void destroyCommandPool(struct Engine* engine);

// VERTEX BUFFER
void createVertexBuffer(struct Engine* engine);
void copyBuffer(
    struct Engine* engine,
    VkBuffer* src,
    VkBuffer* dst,
    VkDeviceSize size
);
void createBuffer(
    struct Engine* engine,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer* buffer,
    VkDeviceMemory* bufferMemory
);
void destroyVertexBuffer(struct Engine* engine);
void freeVertexBufferMemory(struct Engine* engine);
uint32_t findMemType(
    struct Engine* engine,
    uint32_t typeFilter,
    VkMemoryPropertyFlags properties
);

// DESCRIPTOR LAYOUT
void createDescriptorSetLayout(struct Engine* engine);
void destroyDescriptorSetLayout(struct Engine* engine);

// INDEX BUFFER
void createIndexBuffer(struct Engine* engine);
void destroyIndexBuffer(struct Engine* engine);
void freeIndexBufferMemory(struct Engine* engine);

// UNIFORM BUFFER
void createUniformBuffer(struct Engine* engine);
void updateUniformBuffer(struct Engine* engine);
void freeUniformBufferMemory(struct Engine* engine);
void destroyUniformBuffer(struct Engine* engine);

// DESCRIPTOR POOL
void createDescriptorPool(struct Engine* engine);
void destroyDescriptorPool(struct Engine* engine);

// DESCRIPTOR SET
void createDescriptorSet(struct Engine* engine);
// Automatically freed when pool is destroyed

// COMMAND BUFFERS
void createCommandBuffers(struct Engine* engine);
void freeCommandBuffers(struct Engine* engine);

// SEMAPHORES
void createSemaphores(struct Engine* engine);
void destroySemaphores(struct Engine* engine);

void drawFrame(struct Engine* engine);

void recreateSwapChain(struct Engine* engine);

/*  -----------------------------
 *  --- Main engine functions ---
 *  -----------------------------   */
void EngineInit(struct Engine* self, GLFWwindow* window)
{
    self->instance = VK_NULL_HANDLE;
    self->physicalDevice = VK_NULL_HANDLE;
    self->device = VK_NULL_HANDLE;

    self->surfaceExtensionCount = 0;
    self->deviceExtensionCount = 0;

    self->framebuffers = NULL;

    struct Vertex vertices[] = {
        {{-0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
        {{0.5f, -0.5f }, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}}
    };
    self->vertexCount = sizeof(vertices)/sizeof(vertices[0]);
    self->vertices = malloc(sizeof(vertices));
    memcpy(self->vertices, vertices, sizeof(vertices));

    uint16_t indices[] = {
        0, 1, 2, 2, 3, 0
    };
    self->indexCount = sizeof(indices)/sizeof(indices[0]);
    self->indices = malloc(sizeof(indices));
    memcpy(self->indices, indices, sizeof(indices));

    self->commandBuffers = NULL;

    self->swapChainImages = NULL;
    self->imageViews = NULL;
    self->imageCount = 0;

    self->window = window;
}
void EngineRun(struct Engine* self)
{
    // GLFW main loop
    while(!glfwWindowShouldClose(self->window)) {
        glfwPollEvents();

        updateUniformBuffer(self);
        drawFrame(self);
    }

    vkDeviceWaitIdle(self->device);
}
void EngineDestroy(struct Engine* self)
{
    destroySemaphores(self);
    freeCommandBuffers(self);
    destroyDescriptorPool(self);
    freeUniformBufferMemory(self);
    destroyUniformBuffer(self);
    freeIndexBufferMemory(self);
    destroyIndexBuffer(self);
    freeVertexBufferMemory(self);
    destroyVertexBuffer(self);
    destroyCommandPool(self);
    destroyFramebuffers(self);
    freeExtensions(self);
    destroyDescriptorSetLayout(self);
    destroyGraphicsPipeline(self);
    destroyRenderPass(self);
    destroyImageViews(self);
    destroySwapChain(self);
    destroyLogicalDevice(self);
    destroySurface(self);
    if (validationEnabled)
        destroyDebugCallback(self);
    destroyInstance(self);
}

/*  -----------------------------
 *  ----- Utility functions -----
 *  -----------------------------   */
uint32_t min(uint32_t a, uint32_t b)
{
    return (a < b ? a : b);
}
uint32_t max(uint32_t a, uint32_t b)
{
    return (a > b ? a : b);
}

static void onWindowResized(GLFWwindow* window, int width, int height)
{
    if (width == 0 || height == 0)
        return;

    struct Engine* engine = (struct Engine*)glfwGetWindowUserPointer(window);
    recreateSwapChain(engine);
}

int main() {
    struct Engine* engine = calloc(1, sizeof(*engine));

    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(
        WIDTH, HEIGHT,
        "Vulkan Window",
        NULL,
        NULL
    );
    glfwSetWindowUserPointer(window, engine);
    glfwSetWindowSizeCallback(window, onWindowResized);

    EngineInit(engine, window);

    createInstance(engine);
    setupDebugCallback(engine);
    createSurface(engine);
    getPhysicalDevice(engine);
    createLogicalDevice(engine);
    createSwapChain(engine);
    createImageViews(engine);
    createRenderPass(engine);
    createDescriptorSetLayout(engine);
    createGraphicsPipeline(engine);
    createFramebuffers(engine);
    createCommandPool(engine);
    createVertexBuffer(engine);
    createIndexBuffer(engine);
    createUniformBuffer(engine);
    createDescriptorPool(engine);
    createDescriptorSet(engine);
    createCommandBuffers(engine);
    createSemaphores(engine);

    EngineRun(engine);

    EngineDestroy(engine);
    free(engine);

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}

// INSTANCE
void createInstance(struct Engine* engine)
{
    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = NULL;
    appInfo.pApplicationName = "Game";
    appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.flags = 0;
    createInfo.pNext = NULL;
    createInfo.pApplicationInfo = &appInfo;

    getRequiredExtensions(engine);
    createInfo.enabledExtensionCount = engine->surfaceExtensionCount;
    createInfo.ppEnabledExtensionNames =
        (const char* const*)engine->surfaceExtensions;

    const char* validationLayers[] = {"VK_LAYER_LUNARG_standard_validation"};
    uint32_t validationLayerCount =
        sizeof(validationLayers)/sizeof(validationLayers[0]);

    if (validationEnabled &&
        checkValidationSupport(validationLayerCount, validationLayers))
    {
        createInfo.enabledLayerCount = validationLayerCount;
        createInfo.ppEnabledLayerNames = validationLayers;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    VkResult result = vkCreateInstance(&createInfo, NULL, &(engine->instance));

    if (result != VK_SUCCESS)
    {
        if (result == VK_ERROR_INCOMPATIBLE_DRIVER)
            fprintf(stderr, "Your drivers are incompatible with Vulkan.\n");
        else
            fprintf(stderr, "Failed to create Vulkan instance.\n");

        exit(-1);
    }
}

void destroyInstance(struct Engine* engine)
{
    vkDestroyInstance(engine->instance, NULL);
}

// VALIDATION SUPPORT
_Bool checkValidationSupport(uint32_t validationLayerCount, const char** validationLayers)
{
    uint32_t availableLayerCount;
    vkEnumerateInstanceLayerProperties(&availableLayerCount, NULL);
    VkLayerProperties* availableLayers =
        calloc(availableLayerCount, sizeof(*availableLayers));
    vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers);

    uint32_t i, j;
    for (i=0; i<validationLayerCount; i++)
    {
        _Bool layerFound = 0;
        for (j=0; j<availableLayerCount; j++)
        {
            if (strcmp(validationLayers[i], availableLayers[j].layerName) == 0)
            {
                layerFound = 1;
                break;
            }
        }

        if (!layerFound)
            return 0;
    }

    return 1;
}

// DEVICE EXTENSIONS
void getRequiredExtensions(struct Engine* engine)
{
    // Get OS specific extensions from glfw
    const char** glfwExtensions;
    uint32_t glfwExtensionCount;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // Copy glfwExtensions
    uint32_t i;
    for (i=0; i<glfwExtensionCount; i++)
    {
        engine->surfaceExtensions[i] = calloc(1, strlen(glfwExtensions[i])+1);
        strcpy(engine->surfaceExtensions[i], glfwExtensions[i]);
    }

    // Append debug report extension name
    engine->surfaceExtensions[i] =
        calloc(1, strlen(VK_EXT_DEBUG_REPORT_EXTENSION_NAME)+1);
    strcpy(engine->surfaceExtensions[i], VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    i++;

    engine->surfaceExtensionCount = i;
}

void freeExtensions(struct Engine* engine)
{
    uint32_t i;
    for (i=0; i< engine->surfaceExtensionCount; i++)
    {
        free(engine->surfaceExtensions[i]);
    }
    for (i=0; i< engine->deviceExtensionCount; i++)
    {
        free(engine->deviceExtensions[i]);
    }
}

// DEBUG CALLBACK
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objType,
    uint64_t obj,
    size_t location,
    int32_t code,
    const char* pLayerPrefix,
    const char* pMsg,
    void* pUserData)
{
    char* message = malloc(strlen(pMsg) + 100);
    assert(message);

    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        sprintf(message, "%s error, code %d: %s",
                pLayerPrefix, code, pMsg);
    else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        sprintf(message, "%s warning, code %d: %s",
                pLayerPrefix, code, pMsg);
    else
        return VK_FALSE;

    fprintf(stderr, "%s\n", message);

    free(message);

    return VK_FALSE;
}

void setupDebugCallback(struct Engine* engine)
{
    if (!validationEnabled) return;

    VkDebugReportCallbackCreateInfoEXT createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.pNext = NULL;
    createInfo.flags =
        VK_DEBUG_REPORT_ERROR_BIT_EXT |
        VK_DEBUG_REPORT_WARNING_BIT_EXT;
    createInfo.pfnCallback = &debugCallback;
    createInfo.pUserData = NULL;

    engine->createDebugCallback =
        (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
            engine->instance,
            "vkCreateDebugReportCallbackEXT"
        );
    if (!engine->createDebugCallback)
    {
        fprintf(stderr, "GetProcAddr vkCreateDebugReportCallback failed.\n");
        exit(-1);
    }

    engine->destroyDebugCallback =
        (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
            engine->instance,
            "vkDestroyDebugReportCallbackEXT"
        );
    if (!engine->destroyDebugCallback)
    {
        fprintf(stderr, "GetProcAddr vkDestroyDebugReportCallback failed.\n");
        exit(-1);
    }

    VkResult result = engine->createDebugCallback(
        engine->instance,
        &createInfo,
        NULL,
        &(engine->debugCallback)
    );

    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to set up debug callback.\n");
        exit(-1);
    }
}

void destroyDebugCallback(struct Engine* engine)
{
    engine->destroyDebugCallback(
        engine->instance,
        engine->debugCallback,
        NULL
    );
}

// WINDOW SURFACE
void createSurface(struct Engine* engine)
{
    VkResult result;
    result = glfwCreateWindowSurface(
        engine->instance,
        engine->window,
        NULL,
        &(engine->surface)
    );

    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create window surface.\n");
        exit(-1);
    }

}

void destroySurface(struct Engine* engine)
{
    vkDestroySurfaceKHR(engine->instance, engine->surface, NULL);
}

// PHYSICAL DEVICE (GPU)
void getPhysicalDevice(struct Engine* engine)
{
    // Determine number of available devices (GPUs)
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(engine->instance, &deviceCount, NULL);

    // Exit if no devices with Vulkan support are found
    if (deviceCount == 0)
    {
        fprintf(stderr, "No Vulkan compatible GPUs found.\n");
        exit(-1);
    }

    // Determine if any devices are usable
    VkPhysicalDevice* devices = calloc(deviceCount, sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(engine->instance, &deviceCount, devices);

    uint32_t i;
    for (i=0; i<deviceCount; i++)
    {
        if (isDeviceSuitable(engine, &devices[i]))
        {
            engine->physicalDevice = devices[i];
            break;
        }
    }

    free(devices);

    // Exit if the physical device was unchanged
    if (engine->physicalDevice == VK_NULL_HANDLE)
    {
        fprintf(stderr, "No usable devices found.\n");
        exit(-1);
    }
}

// Determines whether a physical device has proper support, returns true (1)
// if it does and sets queue family indices of engine->queueFamilyIndices
_Bool isDeviceSuitable(struct Engine* engine, VkPhysicalDevice* physicalDevice)
{
    // Device extensions
    _Bool extensionsSupported = 0;

    engine->deviceExtensions[engine->deviceExtensionCount] =
        calloc(1, strlen(VK_KHR_SWAPCHAIN_EXTENSION_NAME)+1);
    strcpy(
        engine->deviceExtensions[engine->deviceExtensionCount++],
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    );

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(
        *physicalDevice,
        NULL,
        &extensionCount,
        NULL
    );
    VkExtensionProperties* availableExtensions;
    availableExtensions = calloc(extensionCount, sizeof(*availableExtensions));
    vkEnumerateDeviceExtensionProperties(
        *physicalDevice,
        NULL,
        &extensionCount,
        availableExtensions
    );

    uint32_t i, j;
    for (i=0; i<engine->deviceExtensionCount; i++)
    {
        for (j=0; j<extensionCount; j++)
        {
            if (strcmp(
                    engine->deviceExtensions[i],
                    availableExtensions[j].extensionName
                ) == 0)
            {
                extensionsSupported = 1;
                break;
            }
        }
        if (extensionsSupported)
            break;
    }


    // Swapchain
    _Bool swapChainAdequate = 0;
    if (extensionsSupported)
    {
        engine->swapChainDetails = querySwapChainSupport(
            engine,
            physicalDevice
        );

        if (engine->swapChainDetails.formats != NULL &&
            engine->swapChainDetails.presentModes != NULL)
        {
            swapChainAdequate = 1;
        }
    }

    // Queue families
    engine->queueFamilyIndices = findQueueFamilies(engine, physicalDevice);

    if (queueFamilyComplete(&(engine->queueFamilyIndices)) &&
        extensionsSupported &&
        swapChainAdequate)
    {
        return 1;
    }

    return 0;
}

struct SwapChainSupportDetails querySwapChainSupport(struct Engine* engine, VkPhysicalDevice* physicalDevice)
{
    struct SwapChainSupportDetails details;
    details.formats = NULL;
    details.presentModes = NULL;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        *physicalDevice,
        engine->surface,
        &(details.capabilities)
    );

    vkGetPhysicalDeviceSurfaceFormatsKHR(
        *physicalDevice,
        engine->surface,
        &(details.formatCount),
        NULL
    );

    if (details.formatCount != 0)
    {
        details.formats = calloc(
            details.formatCount,
            sizeof(*(details.formats))
        );
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            *physicalDevice,
            engine->surface,
            &(details.formatCount),
            details.formats
        );
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(
        *physicalDevice,
        engine->surface,
        &(details.presentModeCount),
        NULL
    );
    if (details.presentModeCount != 0)
    {
        details.presentModes = calloc(
            details.presentModeCount,
            sizeof(*(details.presentModes))
        );
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            *physicalDevice,
            engine->surface,
            &(details.presentModeCount),
            details.presentModes
        );
    }

    return details;
}

void freeSwapChainSupportDetails(struct SwapChainSupportDetails* details)
{
    if (details->formats) {
        free(details->formats);
    }
    if (details->presentModes) {
        free(details->presentModes);
    }
}

struct QueueFamilyIndices findQueueFamilies(struct Engine* engine, VkPhysicalDevice* physicalDevice)
{
    struct QueueFamilyIndices queueFamilyIndices = {
        .graphicsFamily = -1,
        .presentFamily = -1
    };

    // Get number of queue families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
            *physicalDevice,
            &queueFamilyCount,
            NULL
    );

    // Create and fill array of VkQueueFamilyProperties
    VkQueueFamilyProperties* queueFamilies = calloc(
            queueFamilyCount,
            sizeof(VkQueueFamilyProperties)
    );
    vkGetPhysicalDeviceQueueFamilyProperties(
            *physicalDevice,
            &queueFamilyCount,
            queueFamilies
    );

    // Iterate over queue families to determine if any are usable
    uint32_t i;
    for (i=0; i<queueFamilyCount; i++)
    {
        // Check if queue family supports VK_QUEUE_GRAPHICS_BIT
        if (queueFamilies[i].queueCount > 0 &&
            (queueFamilies[i].queueFlags &
            VK_QUEUE_GRAPHICS_BIT))
        {
            queueFamilyIndices.graphicsFamily = i;
        }

        // Check if queue family is capable of presenting to window surface
        VkBool32 presentSupport = 0;
        vkGetPhysicalDeviceSurfaceSupportKHR(
            *physicalDevice,
            i,
            engine->surface,
            &presentSupport
        );
        if (queueFamilyCount > 0 && presentSupport)
        {
            queueFamilyIndices.presentFamily = i;
        }

        // Stop once a usable queues are found
        if (queueFamilyComplete(&queueFamilyIndices))
            break;
    }

    free(queueFamilies);

    return queueFamilyIndices;
}

_Bool queueFamilyComplete(struct QueueFamilyIndices* queueFamilyIndices)
{
    return queueFamilyIndices->graphicsFamily >= 0 && queueFamilyIndices->presentFamily >= 0;
}

// LOGICAL DEVICE
void createLogicalDevice(struct Engine* engine)
{
    VkDeviceQueueCreateInfo queueCreateInfos[2];
    int uniqueQueueFamilies[2] = {
        engine->queueFamilyIndices.graphicsFamily,
        engine->queueFamilyIndices.presentFamily
    };

    float queuePriority = 1.0f;
    int i;
    for (i=0; i<2; i++)
    {
        queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[i].pNext = NULL;
        queueCreateInfos[i].flags = 0;
        queueCreateInfos[i].queueFamilyIndex = uniqueQueueFamilies[i];
        queueCreateInfos[i].queueCount = 1;
        queueCreateInfos[i].pQueuePriorities = &queuePriority;
    }

    VkDeviceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    // Passing both pQueueCreateInfos when the indices are the
    // same will result in a validation error
    if (engine->queueFamilyIndices.graphicsFamily == engine->queueFamilyIndices.presentFamily)
        createInfo.queueCreateInfoCount = 1;
    else
        createInfo.queueCreateInfoCount = 2;
    createInfo.pEnabledFeatures = NULL;
    createInfo.enabledExtensionCount = engine->deviceExtensionCount;
    createInfo.ppEnabledExtensionNames = (const char* const*)engine->deviceExtensions;

    // Deprecated, but causes valgrind error if missing
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = NULL;

    VkResult result;
    result = vkCreateDevice(
        engine->physicalDevice,
        &createInfo,
        NULL,
        &(engine->device)
    );

    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create logical device.\n");
        exit(-1);
    }

    vkGetDeviceQueue(
        engine->device,
        engine->queueFamilyIndices.graphicsFamily,
        0,
        &(engine->graphicsQueue)
    );

    vkGetDeviceQueue(
        engine->device,
        engine->queueFamilyIndices.presentFamily,
        0,
        &(engine->presentQueue)
    );
}

void destroyLogicalDevice(struct Engine* engine)
{
    vkDestroyDevice(engine->device, NULL);
}

// SWAPCHAIN
void createSwapChain(struct Engine* engine)
{
    struct SwapChainSupportDetails swapChainSupport;
    swapChainSupport = querySwapChainSupport(engine, &(engine->physicalDevice));

    VkSurfaceFormatKHR surfaceFormat;
    surfaceFormat = chooseSwapSurfaceFormat(
        swapChainSupport.formats,
        swapChainSupport.formatCount
    );

    VkPresentModeKHR presentMode;
    presentMode = chooseSwapPresentMode(
        swapChainSupport.presentModes,
        swapChainSupport.presentModeCount
    );

    VkExtent2D extent = chooseSwapExtent(&(swapChainSupport.capabilities));

    // 0 max image count == no limit
    engine->imageCount = swapChainSupport.capabilities.minImageCount+1;
    if (swapChainSupport.capabilities.minImageCount > 0 &&
        engine->imageCount > swapChainSupport.capabilities.maxImageCount)
            engine->imageCount = swapChainSupport.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.surface = engine->surface;
    createInfo.minImageCount = engine->imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = {
        engine->queueFamilyIndices.graphicsFamily,
        engine->queueFamilyIndices.presentFamily
    };

    if (engine->queueFamilyIndices.graphicsFamily != engine->queueFamilyIndices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = NULL;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    VkSwapchainKHR oldSwapChain = engine->swapChain;
    createInfo.oldSwapchain = oldSwapChain;

    VkSwapchainKHR newSwapChain;
    VkResult result;
    result = vkCreateSwapchainKHR(
        engine->device,
        &createInfo,
        NULL,
        &newSwapChain
    );
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create swapchain.\n");
        exit(-1);
    }

    engine->swapChain = newSwapChain;

    // Destroy the old swapchain when recreating
    if (oldSwapChain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(engine->device, oldSwapChain, NULL);

    vkGetSwapchainImagesKHR(
        engine->device,
        engine->swapChain,
        &(engine->imageCount),
        NULL
    );
    engine->swapChainImages = calloc(
        engine->imageCount,
        sizeof(*engine->swapChainImages)
    );
    vkGetSwapchainImagesKHR(
        engine->device,
        engine->swapChain,
        &(engine->imageCount),
        engine->swapChainImages
    );

    engine->swapChainImageFormat = surfaceFormat.format;
    engine->swapChainExtent = extent;

    freeSwapChainSupportDetails(&swapChainSupport);
}

void destroySwapChain(struct Engine* engine)
{
    freeSwapChainSupportDetails(&(engine->swapChainDetails));
    vkDestroySwapchainKHR(engine->device, engine->swapChain, NULL);
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkSurfaceFormatKHR* availableFormats, int formatCount)
{
    if (formatCount == 1 && availableFormats->format == VK_FORMAT_UNDEFINED)
    {
        VkSurfaceFormatKHR noPreferredFormat = {
            .format = VK_FORMAT_B8G8R8A8_UNORM,
            .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        };
        return noPreferredFormat;
    }

    int i;
    for (i=0; i<formatCount; i++)
    {
        if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return availableFormats[i];
    }

    return *availableFormats;
}

VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR* availablePresentModes, int presentModeCount)
{
    int i;
    for (i=0; i<presentModeCount; i++)
    {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            return availablePresentModes[i];
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR* capabilities)
{
    if (capabilities->currentExtent.width != UINT32_MAX)
    {
        return capabilities->currentExtent;
    }

    VkExtent2D actualExtent = {.width = WIDTH, .height = HEIGHT};
    actualExtent.width = max(
        capabilities->minImageExtent.width,
        min(capabilities->maxImageExtent.width, actualExtent.width)
    );
    actualExtent.height = max(
        capabilities->minImageExtent.height,
        min(capabilities->maxImageExtent.height, actualExtent.height)
    );

    return actualExtent;
}

// IMAGE VIEWS
void createImageViews(struct Engine* engine)
{
    engine->imageViews = calloc(
        engine->imageCount,
        sizeof(*(engine->imageViews))
    );

    uint32_t i;
    for (i=0; i<engine->imageCount; i++)
    {
        VkImageViewCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext = NULL;
        createInfo.flags = 0;
        createInfo.image = engine->swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = engine->swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VkResult result;
        result = vkCreateImageView(
            engine->device,
            &createInfo,
            NULL,
            &(engine->imageViews[i])
        );
        if (result != VK_SUCCESS)
        {
            fprintf(stderr, "Failed to create image view %d.\n", i);
            exit(-1);
        }
    }
}

void destroyImageViews(struct Engine* engine)
{
    uint32_t i;
    for (i=0; i< engine->imageCount; i++)
    {
        vkDestroyImageView(engine->device, engine->imageViews[i], NULL);
    }
}

// RENDER PASS
void createRenderPass(struct Engine* engine)
{
    VkAttachmentDescription colorAttachment;
    colorAttachment.flags = 0;
    colorAttachment.format = engine->swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags = 0;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pResolveAttachments = NULL;
    subpass.pDepthStencilAttachment = NULL;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;

    VkSubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    VkRenderPassCreateInfo renderPassCreateInfo;
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.pNext = NULL;
    renderPassCreateInfo.flags = 0;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachment;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;

    VkResult result;
    result = vkCreateRenderPass(
        engine->device,
        &renderPassCreateInfo,
        NULL,
        &(engine->renderPass)
    );
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create render pass.\n");
        exit(-1);
    }
}

void destroyRenderPass(struct Engine* engine)
{
    vkDestroyRenderPass(engine->device, engine->renderPass, NULL);
}

// DESCRIPTOR LAYOUT
void createDescriptorSetLayout(struct Engine* engine)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding;
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = NULL;

    VkDescriptorSetLayoutCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.bindingCount = 1;
    createInfo.pBindings = &uboLayoutBinding;

    VkResult result;
    result = vkCreateDescriptorSetLayout(
        engine->device,
        &createInfo,
        NULL,
        &(engine->descriptorSetLayout)
    );
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create descriptor set layout.\n");
        exit(-1);
    }
}

void destroyDescriptorSetLayout(struct Engine* engine)
{
    vkDestroyDescriptorSetLayout(
        engine->device,
        engine->descriptorSetLayout,
        NULL
    );
}

// GRAPHICS PIPELINE
void createGraphicsPipeline(struct Engine* engine)
{
    char* vertShaderFname = "shaders/vert.spv";
    char* fragShaderFname = "shaders/frag.spv";
    char* vertShader = NULL; uint32_t vertShaderSize;
    char* fragShader = NULL; uint32_t fragShaderSize;

    vertShader = readFile(vertShaderFname, &vertShaderSize);
    if (!vertShader)
    {
        fprintf(stderr, "Reading file %s failed.\n", vertShaderFname);
        exit(-1);
    }

    fragShader = readFile(fragShaderFname, &fragShaderSize);
    if (!fragShader)
    {
        fprintf(stderr, "Reading file %s failed.\n", fragShaderFname);
        free(vertShader);
        exit(-1);
    }

    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;
    createShaderModule(engine, vertShader, vertShaderSize, &vertShaderModule);
    createShaderModule(engine, fragShader, fragShaderSize, &fragShaderModule);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    VkGraphicsPipelineCreateInfo pipelineInfo;
    VkPipelineShaderStageCreateInfo shaderStageInfos[2];
    VkPipelineVertexInputStateCreateInfo vertInputInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    //VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    VkPipelineViewportStateCreateInfo viewportInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    //VkDynamicState dynamicStates[2];
    //VkPipelineDynamicStateCreateInfo dyanamicStateInfo;

    /*memset(&dynamicStates, 0, sizeof(dynamicStates));
    dynamicStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicStates[1] = VK_DYNAMIC_STATE_LINE_WIDTH;
    memset(&dynamicStateInfo, 0, sizeof(dynamicStateInfo));
    dynamicStateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = 2;
    dynamicStateInfo.pDynamicStates = dynamicStates;*/

    memset(&pipelineInfo, 0, sizeof(pipelineInfo));
    pipelineInfo.sType =
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = engine->pipelineLayout;
    pipelineInfo.stageCount = 2;

    memset(shaderStageInfos, 0, 2*sizeof(shaderStageInfos[0]));
    shaderStageInfos[0].sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfos[0].pNext = NULL;
    shaderStageInfos[0].flags = 0;
    shaderStageInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStageInfos[0].module = vertShaderModule;
    shaderStageInfos[0].pName = "main";
    shaderStageInfos[0].pSpecializationInfo = NULL;
    shaderStageInfos[1].sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfos[1].pNext = NULL;
    shaderStageInfos[1].flags = 0;
    shaderStageInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStageInfos[1].module = fragShaderModule;
    shaderStageInfos[1].pName = "main";
    shaderStageInfos[1].pSpecializationInfo = NULL;

    memset(&vertInputInfo, 0, sizeof(vertInputInfo));
    vertInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertInputInfo.pNext = NULL;
    vertInputInfo.flags = 0;
    VkVertexInputBindingDescription binding = getBindingDescription();
    VkVertexInputAttributeDescription* attribute = getAttributeDescriptions();
    vertInputInfo.vertexBindingDescriptionCount = 1;
    vertInputInfo.pVertexBindingDescriptions = &binding;
    vertInputInfo.vertexAttributeDescriptionCount = 2;
    vertInputInfo.pVertexAttributeDescriptions = attribute;

    memset(&inputAssemblyInfo, 0, sizeof(inputAssemblyInfo));
    inputAssemblyInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    memset(&rasterizationInfo, 0, sizeof(rasterizationInfo));
    rasterizationInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationInfo.depthClampEnable = VK_FALSE;
    rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationInfo.depthBiasEnable = VK_FALSE;
    rasterizationInfo.depthBiasConstantFactor = 0.0f;
    rasterizationInfo.depthBiasClamp = 0.0f;
    rasterizationInfo.depthBiasSlopeFactor = 0.0f;
    rasterizationInfo.lineWidth = 1.0f;

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) engine->swapChainExtent.width;
    viewport.height = (float) engine->swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = engine->swapChainExtent;

    memset(&viewportInfo, 0, sizeof(viewportInfo));
    viewportInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.viewportCount = 1;
    viewportInfo.pViewports = &viewport;
    viewportInfo.scissorCount = 1;
    viewportInfo.pScissors = &scissor;

    memset(&multisampleInfo, 0, sizeof(multisampleInfo));
    multisampleInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleInfo.sampleShadingEnable = VK_FALSE;
    multisampleInfo.minSampleShading = 0.0f;
    multisampleInfo.pSampleMask = NULL;
    multisampleInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleInfo.alphaToOneEnable = VK_FALSE;

    memset(&pipelineLayoutInfo, 0, sizeof(pipelineLayoutInfo));
    pipelineLayoutInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    VkDescriptorSetLayout setLayouts[] = {engine->descriptorSetLayout};
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = setLayouts;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = NULL;

    if (vkCreatePipelineLayout(
            engine->device,
            &pipelineLayoutInfo,
            NULL,
            &(engine->pipelineLayout)
    ) != VK_SUCCESS )
    {
        fprintf(stderr, "Failed to create pipeline layout.\n");
        exit(-1);
    }

    memset(&colorBlendAttachment, 0, sizeof(colorBlendAttachment));
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    memset(&colorBlendInfo, 0, sizeof(colorBlendInfo));
    colorBlendInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo.logicOpEnable = VK_FALSE;
    colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendInfo.attachmentCount = 1;
    colorBlendInfo.pAttachments = &colorBlendAttachment;
    colorBlendInfo.blendConstants[0] = 0.0f;
    colorBlendInfo.blendConstants[1] = 0.0f;
    colorBlendInfo.blendConstants[2] = 0.0f;
    colorBlendInfo.blendConstants[3] = 0.0f;

    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStageInfos;
    pipelineInfo.pVertexInputState = &vertInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pTessellationState = NULL;
    pipelineInfo.pViewportState = &viewportInfo;
    pipelineInfo.pRasterizationState = &rasterizationInfo;
    pipelineInfo.pMultisampleState = &multisampleInfo;
    pipelineInfo.pDepthStencilState = NULL;
    pipelineInfo.pColorBlendState = &colorBlendInfo;
    pipelineInfo.pDynamicState = NULL;
    pipelineInfo.layout = engine->pipelineLayout;
    pipelineInfo.renderPass = engine->renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(
            engine->device,
            VK_NULL_HANDLE,
            1,
            &pipelineInfo,
            NULL,
            &(engine->graphicsPipeline)
    ) != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create graphics pipeline.\n");
        exit(-1);
    }

    // Free memory, shader modules no longer needed
    free(vertShader);
    free(fragShader);
    vkDestroyShaderModule(engine->device, vertShaderModule, NULL);
    vkDestroyShaderModule(engine->device, fragShaderModule, NULL);
    free(attribute);
}

void destroyGraphicsPipeline(struct Engine* engine)
{
    vkDestroyPipeline(engine->device, engine->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(engine->device, engine->pipelineLayout, NULL);
}

char* readFile(const char* fname, uint32_t* fsize)
{
    FILE *fp = fopen(fname, "r");

    if (!fp)
    {
        fprintf(stderr, "Failed to load file %s.\n", fname);
        return NULL;
    }

    fseek(fp, 0L, SEEK_END);
    uint32_t length = ftell(fp);
    rewind(fp);

    char* buffer = malloc(length);
    fread(buffer, length, 1, fp);
    *fsize = length;

    fclose(fp);

    return buffer;
}

void createShaderModule(struct Engine* engine, char* code, uint32_t codeSize, VkShaderModule* shaderModule)
{
    VkShaderModuleCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.codeSize = codeSize;
    createInfo.pCode = (uint32_t*)code;

    VkResult result;
    result = vkCreateShaderModule(
        engine->device,
        &createInfo,
        NULL,
        shaderModule
    );

    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Shader module creation failed.\n");
        exit(-1);
    }
}

// FRAMEBUFFERS
void createFramebuffers(struct Engine* engine)
{
    engine->framebuffers = malloc(
            engine->imageCount * sizeof(*(engine->framebuffers)));

    uint32_t i;
    for (i=0; i<engine->imageCount; i++)
    {
        VkImageView attachments[] = { engine->imageViews[i] };

        VkFramebufferCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.pNext = NULL;
        createInfo.flags = 0;
        createInfo.renderPass = engine->renderPass;
        createInfo.attachmentCount = 1;
        createInfo.pAttachments = attachments;
        createInfo.width = engine->swapChainExtent.width;
        createInfo.height = engine->swapChainExtent.height;
        createInfo.layers = 1;

        VkResult result;
        result = vkCreateFramebuffer(
            engine->device,
            &createInfo,
            NULL,
            &(engine->framebuffers[i])
        );

        if (result != VK_SUCCESS)
        {
            fprintf(stderr, "Error during framebuffer creation.\n");
            exit(-1);
        }
    }
}

void destroyFramebuffers(struct Engine* engine)
{
    uint32_t i;
    for (i=0; i< engine->imageCount; i++)
    {
        vkDestroyFramebuffer(engine->device, engine->framebuffers[i], NULL);
    }
}

// COMMAND POOL
void createCommandPool(struct Engine* engine)
{
    VkCommandPoolCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0; // Relates to reset frequency of command buffers
    createInfo.queueFamilyIndex = engine->queueFamilyIndices.graphicsFamily;

    VkResult result;
    result = vkCreateCommandPool(
        engine->device,
        &createInfo,
        NULL,
        &(engine->commandPool)
    );

    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create command pool.\n");
        exit(-1);
    }
}

void destroyCommandPool(struct Engine* engine)
{
    vkDestroyCommandPool(engine->device, engine->commandPool, NULL);
}

// VERTEX BUFFER
void createVertexBuffer(struct Engine* engine)
{
    VkDeviceSize bufferSize = sizeof(engine->vertices[0]) * engine->vertexCount;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(
        engine,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &(stagingBuffer),
        &(stagingBufferMemory)
    );

    void* data;
    vkMapMemory(
        engine->device,
        stagingBufferMemory,
        0,
        bufferSize,
        0,
        &data
    );
    memcpy(data, engine->vertices, (size_t)bufferSize);
    vkUnmapMemory(engine->device, stagingBufferMemory);

    createBuffer(
        engine,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &(engine->vertexBuffer),
        &(engine->vertexBufferMemory)
    );
    copyBuffer(
        engine,
        &(stagingBuffer),
        &(engine->vertexBuffer),
        bufferSize
    );

    vkDestroyBuffer(engine->device, stagingBuffer, NULL);
    vkFreeMemory(engine->device, stagingBufferMemory, NULL);
}

void copyBuffer(struct Engine* engine, VkBuffer* src, VkBuffer* dst, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = NULL;
    allocInfo.commandPool = engine->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(engine->device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = NULL;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = NULL;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion;
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, *src, *dst, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = NULL;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = NULL;
    submitInfo.pWaitDstStageMask = NULL;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = NULL;

    vkQueueSubmit(engine->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(engine->graphicsQueue);

    vkFreeCommandBuffers(
        engine->device,
        engine->commandPool,
        1,
        &commandBuffer
    );
}

void createBuffer(struct Engine* engine, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory)
{
    VkBufferCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.size = size;
    createInfo.usage = usage;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    // Only required for sharing mode concurrent
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = NULL;

    vkCreateBuffer(engine->device, &createInfo, NULL, buffer);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(
        engine->device,
        *buffer,
        &memRequirements
    );

    VkMemoryAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = NULL;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemType(
        engine,
        memRequirements.memoryTypeBits,
        properties
    );

    VkResult result;
    result = vkAllocateMemory(
        engine->device,
        &allocInfo,
        NULL,
        bufferMemory
    );
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to allocate memory for vertex buffer.\n");
        exit(-1);
    }

    vkBindBufferMemory(
        engine->device,
        *buffer,
        *bufferMemory,
        0
    );
}

void destroyVertexBuffer(struct Engine* engine)
{
    vkDestroyBuffer(engine->device, engine->vertexBuffer, NULL);
}

void freeVertexBufferMemory(struct Engine* engine)
{
    vkFreeMemory(engine->device, engine->vertexBufferMemory, NULL);
}

uint32_t findMemType(struct Engine* engine, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(engine->physicalDevice, &memProperties);

    uint32_t i;
    for (i=0; i<memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    fprintf(stderr, "Failed to find a suitable memory type.\n");
    exit(-1);
}

// INDEX BUFFER
void createIndexBuffer(struct Engine* engine)
{
    VkDeviceSize bufferSize = sizeof(engine->indices[0]) * engine->indexCount;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(
        engine,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &(stagingBuffer),
        &(stagingBufferMemory)
    );

    void* data;
    vkMapMemory(
        engine->device,
        stagingBufferMemory,
        0,
        bufferSize,
        0,
        &data
    );
    memcpy(data, engine->indices, (size_t)bufferSize);
    vkUnmapMemory(engine->device, stagingBufferMemory);

    createBuffer(
        engine,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &(engine->indexBuffer),
        &(engine->indexBufferMemory)
    );
    copyBuffer(
        engine,
        &(stagingBuffer),
        &(engine->indexBuffer),
        bufferSize
    );

    vkDestroyBuffer(engine->device, stagingBuffer, NULL);
    vkFreeMemory(engine->device, stagingBufferMemory, NULL);
}

void destroyIndexBuffer(struct Engine* engine)
{
    vkDestroyBuffer(engine->device, engine->indexBuffer, NULL);
}

void freeIndexBufferMemory(struct Engine* engine)
{
    vkFreeMemory(engine->device, engine->indexBufferMemory, NULL);
}

// UNIFORM BUFFER
void createUniformBuffer(struct Engine* engine)
{
    VkDeviceSize bufferSize = sizeof(struct UniformBufferObject);

    createBuffer(
        engine,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &(engine->uniformStagingBuffer),
        &(engine->uniformStagingBufferMemory)
    );

    createBuffer(
        engine,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &(engine->uniformBuffer),
        &(engine->uniformBufferMemory)
    );
}

void updateUniformBuffer(struct Engine* engine)
{
    struct UniformBufferObject ubo;
    memset(&ubo, 0, sizeof(ubo));

    mat4x4 empty;
    memset(&empty, 0, sizeof(empty));
    mat4x4_identity(empty);

    mat4x4_identity(ubo.model);
    mat4x4_rotate(
        ubo.model, empty,
        0.0f, 0.0f, 1.0f,
        (float)degreesToRadians(22.5)
    );

    vec3 eye = {2.0f, 2.0f, 2.0f};
    vec3 center = {0.0f, 0.0f, 0.0f};
    vec3 up = {0.0f, 0.0f, 1.0f};
    mat4x4_look_at(ubo.view, eye, center, up);

    mat4x4_perspective(
        ubo.proj,
        (float)degreesToRadians(45.0f),
        engine->swapChainExtent.width/(float)engine->swapChainExtent.height,
        0.1f,
        100.0f
    );

    ubo.proj[1][1] *= -1;

    void* data;
    vkMapMemory(
        engine->device,
        engine->uniformStagingBufferMemory,
        0,
        sizeof(ubo),
        0,
        &data
    );
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(engine->device, engine->uniformStagingBufferMemory);

    copyBuffer(
        engine,
        &(engine->uniformStagingBuffer),
        &(engine->uniformBuffer),
        sizeof(ubo)
    );
}

void destroyUniformBuffer(struct Engine* engine)
{
    vkDestroyBuffer(engine->device, engine->uniformStagingBuffer, NULL);
    vkDestroyBuffer(engine->device, engine->uniformBuffer, NULL);
}

void freeUniformBufferMemory(struct Engine* engine)
{
    vkFreeMemory(engine->device, engine->uniformStagingBufferMemory, NULL);
    vkFreeMemory(engine->device, engine->uniformBufferMemory, NULL);
}

// DESCRIPTOR POOL
void createDescriptorPool(struct Engine* engine)
{
    VkDescriptorPoolSize poolSize;
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.maxSets = 1;
    createInfo.poolSizeCount = 1;
    createInfo.pPoolSizes = &poolSize;

    VkResult result;
    result = vkCreateDescriptorPool(
        engine->device,
        &createInfo,
        NULL,
        &(engine->descriptorPool)
    );
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create descriptor pool.\n");
        exit(-1);
    }
}

void destroyDescriptorPool(struct Engine* engine)
{
    vkDestroyDescriptorPool(engine->device, engine->descriptorPool, NULL);
}

// DESCRIPTOR SET
void createDescriptorSet(struct Engine* engine)
{
    VkDescriptorSetLayout layouts[] = {engine->descriptorSetLayout};

    VkDescriptorSetAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = NULL;
    allocInfo.descriptorPool = engine->descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts;

    VkResult result;
    result = vkAllocateDescriptorSets(
        engine->device,
        &allocInfo,
        &(engine->descriptorSet)
    );
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to allocate descriptor sets.\n");
        exit(-1);
    }

    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = engine->uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(struct UniformBufferObject);

    VkWriteDescriptorSet descriptorWrite;
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.pNext = NULL;
    descriptorWrite.dstSet = engine->descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.pImageInfo = NULL;
    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.pTexelBufferView = NULL;

    vkUpdateDescriptorSets(engine->device, 1, &descriptorWrite, 0, NULL);
}

// COMMAND BUFFERS
void createCommandBuffers(struct Engine* engine)
{
    engine->commandBuffers = malloc(
            engine->imageCount * sizeof(*(engine->commandBuffers)));

    VkCommandBufferAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = NULL;
    allocInfo.commandPool = engine->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = engine->imageCount;

    VkResult result;
    result = vkAllocateCommandBuffers(
        engine->device,
        &allocInfo,
        engine->commandBuffers
    );
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create command buffers.\n");
        exit(-1);
    }

    uint32_t i;
    for (i=0; i<engine->imageCount; i++)
    {
        VkCommandBufferBeginInfo beginInfo;
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = NULL;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = NULL;

        vkBeginCommandBuffer(engine->commandBuffers[i], &beginInfo);

        VkRenderPassBeginInfo renderPassInfo;
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.pNext = NULL;
        renderPassInfo.renderPass = engine->renderPass;
        renderPassInfo.framebuffer = engine->framebuffers[i];
        renderPassInfo.renderArea.offset.x = 0;
        renderPassInfo.renderArea.offset.y = 0;
        renderPassInfo.renderArea.extent = engine->swapChainExtent;

        VkClearValue clearColor = {
            .color.float32 = {0.0f, 0.0f, 0.0f, 1.0f}
        };

        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(
            engine->commandBuffers[i],
            &renderPassInfo,
            VK_SUBPASS_CONTENTS_INLINE
        );

        vkCmdBindPipeline(
            engine->commandBuffers[i],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            engine->graphicsPipeline
        );

        VkBuffer vertexBuffers[] = {engine->vertexBuffer};
        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(
            engine->commandBuffers[i],
            0,
            1,
            vertexBuffers,
            offsets
        );

        vkCmdBindIndexBuffer(
            engine->commandBuffers[i],
            engine->indexBuffer,
            0,
            VK_INDEX_TYPE_UINT16
        );

        vkCmdBindDescriptorSets(
            engine->commandBuffers[i],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            engine->pipelineLayout,
            0,
            1,
            &(engine->descriptorSet),
            0,
            NULL
        );

        vkCmdDrawIndexed(
            engine->commandBuffers[i],
            engine->indexCount,
            1,
            0,
            0,
            0
        );

        vkCmdEndRenderPass(engine->commandBuffers[i]);

        VkResult result;
        result = vkEndCommandBuffer(engine->commandBuffers[i]);
        if (result != VK_SUCCESS)
        {
            fprintf(stderr, "Failed to record command buffers.\n");
            exit(-1);
        }
    }
}

void freeCommandBuffers(struct Engine* engine)
{
    vkFreeCommandBuffers(
        engine->device,
        engine->commandPool,
        engine->imageCount,
        engine->commandBuffers
    );

    free(engine->commandBuffers);
}

// SEMAPHORES
void createSemaphores(struct Engine* engine)
{
    VkSemaphoreCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0;

    VkResult result;
    result = vkCreateSemaphore(
        engine->device,
        &createInfo,
        NULL,
        &(engine->imageAvailable)
    );
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create semaphore.\n");
        exit(-1);
    }

    result = vkCreateSemaphore(
        engine->device,
        &createInfo,
        NULL,
        &(engine->renderFinished)
    );
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create semaphore.\n");
        exit(-1);
    }
}

void destroySemaphores(struct Engine* engine)
{
    vkDestroySemaphore(engine->device, engine->imageAvailable, NULL);
    vkDestroySemaphore(engine->device, engine->renderFinished, NULL);
}

void drawFrame(struct Engine* engine)
{
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        engine->device,
        engine->swapChain,
        UINT64_MAX, // Wait for next image indefinitely (ns)
        engine->imageAvailable,
        VK_NULL_HANDLE,
        &imageIndex
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain(engine);
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        fprintf(stderr, "Swapchain image could not be acquired.\n");
        exit(-1);
    }

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = NULL;

    VkSemaphore waitSemaphores[] = { engine->imageAvailable };
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &(engine->commandBuffers[imageIndex]);

    VkSemaphore signalSemaphores[] = { engine->renderFinished };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    result = vkQueueSubmit(
        engine->graphicsQueue,
        1,
        &submitInfo,
        VK_NULL_HANDLE
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        recreateSwapChain(engine);
    }
    else if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Error while submitting queue.\n");
        exit(-1);
    }

    VkPresentInfoKHR presentInfo;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { engine->swapChain };

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = NULL;

    vkQueuePresentKHR(engine->presentQueue, &presentInfo);
}

void recreateSwapChain(struct Engine* engine)
{
    vkDeviceWaitIdle(engine->device);

    // Swapchain deletion is handled in createSwapChain
    createSwapChain(engine);

    destroyImageViews(engine);
    createImageViews(engine);

    destroyRenderPass(engine);
    createRenderPass(engine);

    destroyGraphicsPipeline(engine);
    createGraphicsPipeline(engine);

    destroyFramebuffers(engine);
    createFramebuffers(engine);

    freeCommandBuffers(engine);
    createCommandBuffers(engine);
}
