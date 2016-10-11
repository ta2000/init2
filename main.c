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

struct Engine
{
    GLFWwindow* window;
    VkInstance instance;
    char* extensionNames[64];
    uint32_t extensionCount;
    VkDebugReportCallbackEXT debugCallback;
    PFN_vkCreateDebugReportCallbackEXT createDebugCallback;
    PFN_vkDestroyDebugReportCallbackEXT destroyDebugCallback;
    VkSurfaceKHR surface;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSwapchainKHR swapChain;
    VkImage* swapChainImages;
    uint32_t imageCount;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    VkImageView* imageViews;
};
void EngineInit(struct Engine* self, GLFWwindow* window)
{
    self->physicalDevice = VK_NULL_HANDLE;
    self->device = VK_NULL_HANDLE;

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

    // Free extensions
    for (i=0; i<self->extensionCount; i++)
    {
        free(self->extensionNames[i]);
    }

    // Destroy all imageviews
    // TODO: Destroy as many image views as there are created
    // in case application fails in the middle of createing imageviews
    if (self->imageViews != NULL)
    {
        for (i=0; i<self->imageCount; i++)
        {
            vkDestroyImageView(
                self->device,
                *(self->imageViews+(i*sizeof(*self->imageViews))),
                NULL
            );
        }
    }

    vkDestroySwapchainKHR(self->device, self->swapChain, NULL);
    vkDestroyDevice(self->device, NULL);
    self->destroyDebugCallback(
        self->instance,
        self->debugCallback,
        NULL
    );
    vkDestroyInstance(self->instance, NULL);
}

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

    struct Engine engine;
    EngineInit(&engine, window);
    createInstance(&engine);
    setupDebugCallback(&engine);
    createSurface(&engine);
    getPhysicalDevice(&engine);
    createLogicalDevice(&engine);
    createSwapChain(&engine);
    //createImageViews(&engine);

    EngineRun(&engine);

    EngineDestroy(&engine);

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
    appInfo.pApplicationName = "Game";
    appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    getRequiredExtensions(engine);
    createInfo.enabledExtensionCount = engine->extensionCount;
    createInfo.ppEnabledExtensionNames =
        (const char* const*)engine->extensionNames;

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
    const char* validationLayerName;
    const char* availableLayerName;
    for (i=0; i<validationLayerCount; i++)
    {
        _Bool layerFound = 0;
        validationLayerName = validationLayers[i];
        for (j=0; j<availableLayerCount; j++)
        {
            availableLayerName =
                (availableLayers + i*sizeof(*availableLayers))->layerName;

            if (strcmp(validationLayerName, availableLayerName) == 0)
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
        engine->extensionNames[i] = malloc(strlen(glfwExtensions[i]));
        strcpy(engine->extensionNames[i], glfwExtensions[i]);
    }

    // Append debug report extension name
    engine->extensionNames[i] =
        malloc(strlen(VK_EXT_DEBUG_REPORT_EXTENSION_NAME));
    strcpy(engine->extensionNames[i], VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    i++;

    engine->extensionCount = i;
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
        sprintf(message, "Error from %s, code %d: %s.",
                pLayerPrefix, code, pMsg);
    else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        sprintf(message, "Warning from %s, code %d: %s.",
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
    VkPhysicalDevice* currentDevice;
    for (i=0; i<deviceCount; i++)
    {
        currentDevice = devices+(i*sizeof(*currentDevice));

        if (isDeviceSuitable(engine, currentDevice))
        {
            engine->physicalDevice = *currentDevice;
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

_Bool isDeviceSuitable(struct Engine* engine, VkPhysicalDevice* physicalDevice)
{
    struct QueueFamilyIndices indices;
    indices = findQueueFamilies(engine, physicalDevice);

    _Bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);

    _Bool swapChainAdequate = 0;
    if (extensionsSupported)
    {
        struct SwapChainSupportDetails swapChainSupport;
        swapChainSupport = querySwapChainSupport(engine, physicalDevice);
        if (swapChainSupport.formats != NULL &&
            swapChainSupport.presentModes != NULL)
        {
            swapChainAdequate = 1;
        }
        free(swapChainSupport.formats);
        free(swapChainSupport.presentModes);
    }

    if (queueFamilyComplete(&indices) &&
        extensionsSupported &&
        swapChainAdequate)
    {
        return 1;
    }

    return 0;
}

_Bool checkDeviceExtensionSupport(VkPhysicalDevice* physicalDevice)
{
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

    const char* deviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    size_t deviceExtensionCount;
    deviceExtensionCount = sizeof(deviceExtensions)/sizeof(deviceExtensions[0]);

    uint32_t i;
    for (i=0; i<deviceExtensionCount; i++)
    {
        if (!extensionAvailable(
                deviceExtensions[i],
                availableExtensions,
                extensionCount
        ))
        {
            fprintf(
                stderr,
                "Extension %s not supported.\n",
                deviceExtensions[i]
            );
            exit(-1);
        }
    }

    return 1;
}

_Bool extensionAvailable(const char* extensionName, VkExtensionProperties* availableExtensions, int extensionCount)
{
    int i;
    for (i=0; i<extensionCount; i++)
    {
        VkExtensionProperties* currentExtension;
        currentExtension = availableExtensions+(i*sizeof(*currentExtension));

        if (strcmp(extensionName, currentExtension->extensionName) == 0)
        {
            return 1;
        }
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
        VkSurfaceFormatKHR* currentFormat;
        currentFormat = availableFormats+(i*sizeof(*currentFormat));

        if (currentFormat->format == VK_FORMAT_B8G8R8A8_UNORM &&
            currentFormat->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return *currentFormat;
    }

    return *availableFormats;
}

VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR* availablePresentModes, int presentModeCount)
{
    int i;
    for (i=0; i<presentModeCount; i++)
    {
        VkPresentModeKHR* currentPresentMode;
        currentPresentMode = availablePresentModes+(i*sizeof(*currentPresentMode));

        if (*currentPresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return *currentPresentMode;
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
        // Current queue family
        VkQueueFamilyProperties queueFamily;
        queueFamily = *(queueFamilies+(i*sizeof(VkQueueFamilyProperties)));

        // Check if queue family supports VK_QUEUE_GRAPHICS_BIT
        if (queueFamily.queueCount > 0 &&
            queueFamily.queueFlags &
            VK_QUEUE_GRAPHICS_BIT)
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
    struct QueueFamilyIndices indices;
    indices = findQueueFamilies(engine, &(engine->physicalDevice));

    VkDeviceQueueCreateInfo queueCreateInfos[2];
    int uniqueQueueFamilies[2] = {
        indices.graphicsFamily,
        indices.presentFamily
    };

    float queuePriority = 1.0f;
    int i;
    for (i=0; i<2; i++)
    {
        VkDeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = uniqueQueueFamilies[i];
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos[i] = queueCreateInfo;
    }

    VkPhysicalDeviceFeatures deviceFeatures;
    VkDeviceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.queueCreateInfoCount = 2;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 0;
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
        indices.graphicsFamily,
        0,
        &(engine->graphicsQueue)
    );

    vkGetDeviceQueue(
        engine->device,
        indices.presentFamily,
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
    createInfo.surface = engine->surface;
    createInfo.minImageCount = engine->imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    struct QueueFamilyIndices indices;
    indices = findQueueFamilies(engine, &(engine->physicalDevice));
    uint32_t queueFamilyIndices[] = {
        indices.graphicsFamily,
        indices.presentFamily
    };

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
        createInfo.image = *(engine->swapChainImages + (i*sizeof(*engine->swapChainImages)));
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
            engine->imageViews + (i*sizeof(*(engine->imageViews)))
        );
        if (result != VK_SUCCESS)
        {
            fprintf(stderr, "Failed to create image view %d.\n", i);
            exit(-1);
        }
    }
}
