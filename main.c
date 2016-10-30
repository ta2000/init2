#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const _Bool validationEnabled = 1;

struct QueueFamilyIndices
{
    int graphicsFamily;
    int presentFamily;
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* formats;
    uint32_t formatCount;
    VkPresentModeKHR* presentModes;
    uint32_t presentModeCount;
};

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
    struct QueueFamilyIndices indices;

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

    // Command pool/buffers
    VkCommandPool commandPool;
    VkCommandBuffer* commandBuffers;
};
void EngineInit(struct Engine* self, GLFWwindow* window)
{
    self->physicalDevice = VK_NULL_HANDLE;
    self->device = VK_NULL_HANDLE;

    self->surfaceExtensionCount = 0;
    self->deviceExtensionCount = 0;

    self->framebuffers = NULL;
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
    }
}
void EngineDestroy(struct Engine* self)
{
    uint32_t i;

    vkFreeCommandBuffers(
        self->device,
        self->commandPool,
        self->imageCount,
        self->commandBuffers
    );

    free(self->commandBuffers);

    // Destroy command pool
    vkDestroyCommandPool(self->device, self->commandPool, NULL);

    // Destroy frame buffers, same number as swapchain images
    if (self->framebuffers != NULL)
    {
        for (i=0; i<self->imageCount; i++)
        {
            vkDestroyFramebuffer(self->device, self->framebuffers[i], NULL);
        }
    }

    // Free extensions
    for (i=0; i<self->surfaceExtensionCount; i++)
    {
        free(self->surfaceExtensions[i]);
    }
    for (i=0; i<self->deviceExtensionCount; i++)
    {
        free(self->deviceExtensions[i]);
    }

    vkDestroyPipeline(self->device, self->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(self->device, self->pipelineLayout, NULL);
    vkDestroyRenderPass(self->device, self->renderPass, NULL);

    // Destroy all imageviews
    // TODO: Destroy as many image views as there are created
    // in case application fails in the middle of createing imageviews
    if (self->imageViews != NULL)
    {
        for (i=0; i<self->imageCount; i++)
        {
            vkDestroyImageView(self->device, self->imageViews[i], NULL);
        }
    }

    free(self->swapChainDetails.formats);
    free(self->swapChainDetails.presentModes);

    vkDestroySwapchainKHR(self->device, self->swapChain, NULL);
    vkDestroyDevice(self->device, NULL);
    vkDestroySurfaceKHR(self->instance, self->surface, NULL);
    self->destroyDebugCallback(
        self->instance,
        self->debugCallback,
        NULL
    );
    vkDestroyInstance(self->instance, NULL);
}

void createInstance(struct Engine* engine);
_Bool checkValidationSupport(
    uint32_t validationLayerCount,
    const char** validationLayers
);
void getRequiredExtensions(struct Engine* engine);
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
void createSurface(struct Engine* engine);
void getPhysicalDevice(struct Engine* engine);
_Bool isDeviceSuitable(struct Engine* engine, VkPhysicalDevice* physicalDevice);
_Bool checkDeviceExtensionSupport(VkPhysicalDevice* physicalDevice);
_Bool extensionAvailable(
    const char* extensionName,
    VkExtensionProperties* availableExtensions,
    int extensionCount
);
struct SwapChainSupportDetails querySwapChainSupport(
    struct Engine* engine,
    VkPhysicalDevice* physicalDevice
);
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
struct QueueFamilyIndices findQueueFamilies(
    struct Engine* engine,
    VkPhysicalDevice* physicalDevice
);
_Bool queueFamilyComplete(struct QueueFamilyIndices* indices);
void createLogicalDevice(struct Engine* engine);
void createSwapChain(struct Engine* engine);
void createImageViews(struct Engine* engine);
void createRenderPass(struct Engine* engine);
char* readFile(const char* fname, uint32_t* fsize);
void createGraphicsPipeline(struct Engine* engine);
void createShaderModule(
    struct Engine* engine,
    char* code,
    uint32_t codeSize,
    VkShaderModule* shaderModule
);
void createFrameBuffers(struct Engine* engine);
void createCommandPool(struct Engine* engine);
void createCommandBuffers(struct Engine* engine);

uint32_t min(uint32_t a, uint32_t b)
{
    return (a < b ? a : b);
}

uint32_t max(uint32_t a, uint32_t b)
{
    return (a > b ? a : b);
}

int main() {
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(
        WIDTH, HEIGHT,
        "Vulkan Window",
        NULL,
        NULL
    );

    struct Engine* engine = malloc(sizeof(*engine));
    EngineInit(engine, window);
    createInstance(engine);
    setupDebugCallback(engine);
    createSurface(engine);
    getPhysicalDevice(engine);
    createLogicalDevice(engine);
    createSwapChain(engine);
    createImageViews(engine);
    createRenderPass(engine);
    createGraphicsPipeline(engine);
    createFrameBuffers(engine);
    createCommandPool(engine);
    createCommandBuffers(engine);

    EngineRun(engine);

    EngineDestroy(engine);

    // Close window
    glfwDestroyWindow(window);

    // Stop GLFW
    glfwTerminate();

    return 0;
}

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
// if it does and sets queue family indices of engine->indices
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
    engine->indices = findQueueFamilies(engine, physicalDevice);

    if (queueFamilyComplete(&(engine->indices)) &&
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

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        *physicalDevice,
        engine->surface,
        &formatCount,
        NULL
    );
    details.formatCount = formatCount;
    if (formatCount != 0)
    {
        details.formats = calloc(
            formatCount,
            sizeof(*(details.formats))
        );
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            *physicalDevice,
            engine->surface,
            &formatCount,
            details.formats
        );
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        *physicalDevice,
        engine->surface,
        &presentModeCount,
        NULL
    );
    details.presentModeCount = presentModeCount;
    if (presentModeCount != 0)
    {
        details.presentModes = calloc(
            presentModeCount,
            sizeof(*(details.presentModes))
        );
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            *physicalDevice,
            engine->surface,
            &presentModeCount,
            details.presentModes
        );
    }

    return details;
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

struct QueueFamilyIndices findQueueFamilies(struct Engine* engine, VkPhysicalDevice* physicalDevice)
{
    struct QueueFamilyIndices indices = {
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
            indices.graphicsFamily = i;
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
            indices.presentFamily = i;
        }

        // Stop once a usable queues are found
        if (queueFamilyComplete(&indices))
            break;
    }

    free(queueFamilies);

    return indices;
}

_Bool queueFamilyComplete(struct QueueFamilyIndices* indices)
{
    return indices->graphicsFamily >= 0 && indices->presentFamily >= 0;
}

void createLogicalDevice(struct Engine* engine)
{
    VkDeviceQueueCreateInfo queueCreateInfos[2];
    int uniqueQueueFamilies[2] = {
        engine->indices.graphicsFamily,
        engine->indices.presentFamily
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
    if (engine->indices.graphicsFamily == engine->indices.presentFamily)
        createInfo.queueCreateInfoCount = 1;
    else
        createInfo.queueCreateInfoCount = 2;
    createInfo.pEnabledFeatures = NULL;
    createInfo.enabledExtensionCount = engine->deviceExtensionCount;
    createInfo.ppEnabledExtensionNames = (const char* const*)engine->deviceExtensions;

    // Deprecated, but currently causes segfault if missing
    createInfo.enabledLayerCount = 0;

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
        engine->indices.graphicsFamily,
        0,
        &(engine->graphicsQueue)
    );

    vkGetDeviceQueue(
        engine->device,
        engine->indices.presentFamily,
        0,
        &(engine->presentQueue)
    );
}

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
        engine->indices.graphicsFamily,
        engine->indices.presentFamily
    };

    if (engine->indices.graphicsFamily != engine->indices.presentFamily)
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
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult result;
    result = vkCreateSwapchainKHR(
        engine->device,
        &createInfo,
        NULL,
        &(engine->swapChain)
    );
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create swapchain.\n");
        exit(-1);
    }

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
}

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

    VkRenderPassCreateInfo renderPassCreateInfo;
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.pNext = NULL;
    renderPassCreateInfo.flags = 0;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachment;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 0;
    renderPassCreateInfo.pDependencies = NULL;

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

    memset(&inputAssemblyInfo, 0, sizeof(inputAssemblyInfo));
    inputAssemblyInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    memset(&rasterizationInfo, 0, sizeof(rasterizationInfo));
    rasterizationInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
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
    multisampleInfo.minSampleShading = 1.0f;
    multisampleInfo.pSampleMask = NULL;
    multisampleInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleInfo.alphaToOneEnable = VK_FALSE;

    memset(&pipelineLayoutInfo, 0, sizeof(pipelineLayoutInfo));
    pipelineLayoutInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = NULL;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = 0;

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
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask = 0;

    memset(&colorBlendInfo, 0, sizeof(colorBlendInfo));
    colorBlendInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    // TODO: investigate logicOp
    // Validation error when logicOpEnable is set to
    // VK_TRUE
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
    pipelineInfo.basePipelineIndex = -1;

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

void createFrameBuffers(struct Engine* engine)
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

void createCommandPool(struct Engine* engine)
{
    VkCommandPoolCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0; // Relates to reset frequency of command buffers
    createInfo.queueFamilyIndex = engine->indices.graphicsFamily;

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

        vkCmdDraw(engine->commandBuffers[i], 3, 1, 0, 0);

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
