#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/vector3.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>

#include "engine.h"

// WINDOW SIZE
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
// VALIDATION LAYERS
const _Bool validationEnabled = 1;

// MIN/MAX
uint32_t min(uint32_t a, uint32_t b)
{
    return (a < b ? a : b);
}
uint32_t max(uint32_t a, uint32_t b)
{
    return (a > b ? a : b);
}

/*  -----------------------------
 *  --- Main engine functions ---
 *  -----------------------------   */
void EngineCreateGameObject(struct Engine* self, struct Mesh* mesh)
{
    if (self->gameObjectCount >= MAX_OBJECTS)
    {
        printf("Object limit reached.\n");
        return;
    }

    // Set mesh of new game object
    self->gameObjects[self->gameObjectCount].mesh = mesh;

    // Make identity matrix
    mat4x4_identity(self->gameObjects[self->gameObjectCount].model);

    // Allocate command buffers
    VkCommandBufferAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = NULL;
    allocInfo.commandPool = self->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocInfo.commandBufferCount = 1;

    VkResult result;
    result = vkAllocateCommandBuffers(
        self->device,
        &allocInfo,
        &(self->gameObjects[self->gameObjectCount].commandBuffer)
    );
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to allocate command buffer.\n");
        exit(-1);
    }

    self->gameObjectCount++;
}
void EngineDestroyGameObject(struct Engine* self, struct GameObject* gameObject)
{
    if (self->gameObjectCount <= 0)
    {
        printf("No game objects left to destroy.\n");
        return;
    }

    // Free command buffers
    vkFreeCommandBuffers(
        self->device,
        self->commandPool,
        1,
        &(gameObject->commandBuffer)
    );

    self->gameObjectCount--;
}
void EngineCreateMesh(struct Engine* self, struct Vertex* vertices, uint32_t vertexCount, uint32_t* indices, uint32_t indexCount)
{
    if (self->meshCount >= MAX_MESHES)
    {
        printf("Mesh limit reached.\n");
        return;
    }

    createVertexBuffer(
        self,
        vertices,
        vertexCount,
        &(self->meshes[self->meshCount].vertexBuffer),
        &(self->meshes[self->meshCount].vertexMemory)
    );

    self->meshes[self->meshCount].indexCount = indexCount;

    createIndexBuffer(
        self,
        indices,
        indexCount,
        &(self->meshes[self->meshCount].indexBuffer),
        &(self->meshes[self->meshCount].indexMemory)
    );

    self->meshCount++;
}
void EngineDestroyMesh(struct Engine* self, struct Mesh* mesh)
{
    if (self->meshCount <= 0)
    {
        printf("No meshes left to destroy.\n");
        return;
    }

    self->meshCount--;
    freeIndexBufferMemory(self, &(mesh->indexMemory));
    destroyIndexBuffer(self, &(mesh->indexBuffer));
    freeVertexBufferMemory(self, &(mesh->vertexMemory));
    destroyVertexBuffer(self, &(mesh->vertexBuffer));
    EngineDestroyDescriptor(self, &(mesh->descriptor));
}
void EngineCreateDescriptor(struct Engine* self, struct Descriptor* descriptor, const char* textureSrc)
{
    createTextureImage(
        self,
        textureSrc,
        &(descriptor->textureImage.image),
        &(descriptor->textureImage.memory)
    );
    createTextureImageView(
        self,
        &(descriptor->textureImage.image),
        &(descriptor->textureImage.imageView)
    );
    createTextureSampler(
        self,
        &(descriptor->textureImage.sampler)
    );
    createDescriptorSet(
        self,
        &(descriptor->descriptorSet),
        &(descriptor->textureImage.sampler),
        &(descriptor->textureImage.imageView)
    );
}
void EngineDestroyDescriptor(struct Engine* self, struct Descriptor* descriptor)
{
    destroyTextureSampler(
        self,
        &(descriptor->textureImage.sampler)
    );
    destroyTextureImageView(
        self,
        &(descriptor->textureImage.imageView)
    );
    destroyTextureImage(
        self,
        &(descriptor->textureImage.image),
        &(descriptor->textureImage.memory)
    );
}
void EngineLoadModel(struct Engine* self, const char* path)
{
    const struct aiScene* scene;
    scene = aiImportFile(
        path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs |
        aiProcess_JoinIdenticalVertices
    );

    if (!scene)
    {
        fprintf(stderr, "Failed to load model: %s\n", path);
        exit(-1);
    }

    if (scene->mNumMeshes != 1)
    {
        fprintf(stderr, "Not one mesh in model %s\n", path);
        exit(-1);
    }

    struct aiMesh* mesh = scene->mMeshes[0];

    struct Vertex* vertices;
    vertices = calloc(mesh->mNumVertices, sizeof(struct Vertex));
    uint32_t vertexCount = 0;

    uint32_t* indices;
    indices = calloc(mesh->mNumFaces * 3, sizeof(*indices));
    uint32_t indexCount = 0;

    uint32_t i;
    for (i=0; i<mesh->mNumVertices; i++)
    {
        vertices[vertexCount].position[0] = mesh->mVertices[i].x;
        vertices[vertexCount].position[1] = mesh->mVertices[i].y;
        vertices[vertexCount].position[2] = mesh->mVertices[i].z;
        vertices[vertexCount].texCoord[0] = mesh->mTextureCoords[0][i].x;
        vertices[vertexCount].texCoord[1] = mesh->mTextureCoords[0][i].y;
        vertexCount++;
    }

    for (i=0; i<mesh->mNumFaces; i++)
    {
        struct aiFace* face = &(mesh->mFaces[i]);
        assert(face->mNumIndices == 3);
        indices[indexCount] = face->mIndices[0];
        indexCount++;
        indices[indexCount] = face->mIndices[1];
        indexCount++;
        indices[indexCount] = face->mIndices[2];
        indexCount++;
    }

    EngineCreateMesh(
        self,
        vertices,
        vertexCount,
        indices,
        indexCount
    );

    free(vertices);
    free(indices);
    aiReleaseImport(scene);
}
void EngineInit(struct Engine* self)
{
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(
        WIDTH, HEIGHT,
        "Vulkan Window",
        NULL,
        NULL
    );
    glfwSetWindowUserPointer(window, self);
    glfwSetWindowSizeCallback(window, onWindowResized);
    glfwSetKeyCallback(window, keyCallback);

    self->window = window;

    createInstance(self);
    setupDebugCallback(self);
    createSurface(self);
    getPhysicalDevice(self);
    createLogicalDevice(self);
    createSwapChain(self);
    createImageViews(self);
    findDepthFormat(self);
    createRenderPass(self);
    createDescriptorSetLayout(self);
    createGraphicsPipeline(self);
    createCommandPool(self);
    createDepthResources(self);
    createFramebuffers(self);
    createUniformBuffer(self);
    createDescriptorPool(self);
    createCommandBuffers(self);
    createFence(self);
    createSemaphores(self);
}
void EngineUpdate(struct Engine* self)
{
    if (self->gameLoopCallback)
    {
        (*self->gameLoopCallback)(self->userPointer);
    }

    generateDrawCommands(self, self->currentBuffer);
    updateUniformBuffer(self);
    drawFrame(self);

    self->currentBuffer++;
    self->currentBuffer %= 3;
}
void EngineRun(struct Engine* self)
{
    // GLFW main loop
    while(!glfwWindowShouldClose(self->window)) {
        glfwPollEvents();
        EngineUpdate(self);
    }

    vkDeviceWaitIdle(self->device);
}
void EngineDestroy(struct Engine* self)
{
    uint32_t i;

    destroySemaphores(self);
    destroyFence(self);
    freeCommandBuffers(self);
    destroyDescriptorPool(self);
    freeUniformBufferMemory(self);
    destroyUniformBuffer(self);
    uint32_t meshCount = self->meshCount;
    for (i=0; i<meshCount; i++)
    {
        EngineDestroyMesh(self, &(self->meshes[i]));
    }
    uint32_t gameObjectCount = self->gameObjectCount;
    for (i=0; i<gameObjectCount; i++)
    {
        EngineDestroyGameObject(self, &(self->gameObjects[i]));
    }
    destroyFramebuffers(self);
    destroyDepthResources(self);
    destroyCommandPool(self);
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

    glfwDestroyWindow(self->window);
    glfwTerminate();
}

// GLFW
void onWindowResized(GLFWwindow* window, int width, int height)
{
    if (width == 0 || height == 0)
        return;

    struct Engine* engine = (struct Engine*)glfwGetWindowUserPointer(window);
    recreateSwapChain(engine);
}

void keyCallback(
    GLFWwindow* window,
    int key,
    int scancode,
    int action,
    int mods)
{
    struct Engine* engine =
        (struct Engine*)glfwGetWindowUserPointer(window);

    if (engine->keyCallback)
    {
        (*engine->keyCallback)(engine->userPointer, key, action);
    }
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
        createImageView(
            engine,
            engine->swapChainImages[i],
            engine->swapChainImageFormat,
            VK_IMAGE_ASPECT_COLOR_BIT,
            &(engine->imageViews[i])
        );
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

// DEPTH FORMAT
void findDepthFormat(struct Engine* engine)
{
    VkFormat candidates[3] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    engine->depthFormat = findSupportedFormat(
        engine,
        candidates,
        3,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
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

    VkAttachmentDescription depthAttachment;
    depthAttachment.flags = 0;
    depthAttachment.format = engine->depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef;
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription attachments[] = {
        colorAttachment,
        depthAttachment
    };

    VkSubpassDescription subpass;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags = 0;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pResolveAttachments = NULL;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
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
    renderPassCreateInfo.attachmentCount = 2;
    renderPassCreateInfo.pAttachments = attachments;
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

    VkDescriptorSetLayoutBinding samplerLayoutBinding;
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = NULL;

    VkDescriptorSetLayoutBinding layoutBindings[2] = {
        uboLayoutBinding,
        samplerLayoutBinding
    };

    VkDescriptorSetLayoutCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.bindingCount = 2;
    createInfo.pBindings = layoutBindings;

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
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
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
    vertInputInfo.vertexAttributeDescriptionCount = 3;
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

    VkPushConstantRange pushConstantRange;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(mat4x4);

    memset(&pipelineLayoutInfo, 0, sizeof(pipelineLayoutInfo));
    pipelineLayoutInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    VkDescriptorSetLayout setLayouts[] = {engine->descriptorSetLayout};
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = setLayouts;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

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

    memset(&depthStencilInfo, 0, sizeof(depthStencilInfo));
    depthStencilInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.depthTestEnable = VK_TRUE;
    depthStencilInfo.depthWriteEnable = VK_TRUE;
    depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilInfo.minDepthBounds = 0.0f,
    depthStencilInfo.maxDepthBounds = 1.0f,
    depthStencilInfo.stencilTestEnable = VK_FALSE;

    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStageInfos;
    pipelineInfo.pVertexInputState = &vertInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pTessellationState = NULL;
    pipelineInfo.pViewportState = &viewportInfo;
    pipelineInfo.pRasterizationState = &rasterizationInfo;
    pipelineInfo.pMultisampleState = &multisampleInfo;
    pipelineInfo.pDepthStencilState = &depthStencilInfo;
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
    descriptions = calloc(3, sizeof(VkVertexInputAttributeDescription));

    // Position
    descriptions[0].location = 0;
    descriptions[0].binding = 0;
    descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    descriptions[0].offset = offsetof(struct Vertex, position);

    // Color
    descriptions[1].location = 1;
    descriptions[1].binding = 0;
    descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    descriptions[1].offset = offsetof(struct Vertex, color);

    // Texture
    descriptions[2].location = 2;
    descriptions[2].binding = 0;
    descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    descriptions[2].offset = offsetof(struct Vertex, texCoord);

    return descriptions;
}

// FRAMEBUFFERS
void createFramebuffers(struct Engine* engine)
{
    engine->framebuffers = malloc(
            engine->imageCount * sizeof(*(engine->framebuffers)));

    uint32_t i;
    for (i=0; i<engine->imageCount; i++)
    {
        VkImageView attachments[] = {
            engine->imageViews[i],
            engine->depthImageView
        };

        VkFramebufferCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.pNext = NULL;
        createInfo.flags = 0;
        createInfo.renderPass = engine->renderPass;
        createInfo.attachmentCount = 2;
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
    createInfo.flags = 2; // Relates to reset frequency of command buffers
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

// DEPTH RESOURCES
void createDepthResources(struct Engine* engine)
{
    createImage(
        engine,
        engine->depthFormat,
        engine->swapChainExtent.width,
        engine->swapChainExtent.height,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &(engine->depthImage),
        &(engine->depthImageMemory)
    );

    createImageView(
        engine,
        engine->depthImage,
        engine->depthFormat,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        &(engine->depthImageView)
    );

    transitionImageLayout(
        engine,
        engine->depthImage,
        engine->depthFormat,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    );
}

void destroyDepthResources(struct Engine* engine)
{
    vkDestroyImageView(engine->device, engine->depthImageView, NULL);
    vkFreeMemory(engine->device, engine->depthImageMemory, NULL);
    vkDestroyImage(engine->device, engine->depthImage, NULL);
}

_Bool hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
            format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat findSupportedFormat(struct Engine* engine, VkFormat* candidates, uint32_t candidateCount, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    uint32_t i;
    for (i=0; i<candidateCount; i++)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(
            engine->physicalDevice,
            candidates[i],
            &props
        );

        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features)
        {
            return candidates[i];
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                (props.optimalTilingFeatures & features) == features)
        {
            return candidates[i];
        }
    }

    fprintf(stderr, "Failed to find supported depth format.\n");
    exit(-1);
}

// TEXTURE IMAGE
void createTextureImage(struct Engine* engine, const char* textureSrc, VkImage* textureImage, VkDeviceMemory* textureImageMemory)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(
        textureSrc,
        &texWidth,
        &texHeight,
        &texChannels,
        STBI_rgb_alpha
    );
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels)
    {
        fprintf(stderr, "stb_image failed to load resource %s\n", textureSrc);
        exit(-1);
    }

    VkImage stagingImage;
    VkDeviceMemory stagingImageMemory;

    createImage(
        engine,
        VK_FORMAT_R8G8B8A8_UNORM,
        texWidth,
        texHeight,
        VK_IMAGE_TILING_LINEAR,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingImage,
        &stagingImageMemory
    );

    void* data;
    vkMapMemory(engine->device, stagingImageMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, (size_t)imageSize);
    vkUnmapMemory(engine->device, stagingImageMemory);

    stbi_image_free(pixels);

    createImage(
        engine,
        VK_FORMAT_R8G8B8A8_UNORM,
        texWidth,
        texHeight,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        textureImage,
        textureImageMemory
    );

    transitionImageLayout(
        engine,
        stagingImage,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_LAYOUT_PREINITIALIZED,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
    );
    transitionImageLayout(
        engine,
        *textureImage,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_LAYOUT_PREINITIALIZED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );
    copyImage(engine, stagingImage, *textureImage, texWidth, texHeight);

    transitionImageLayout(
        engine,
        *textureImage,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    vkDestroyImage(
        engine->device,
        stagingImage,
        NULL
    );

    vkFreeMemory(
        engine->device,
        stagingImageMemory,
        NULL
    );
}

void destroyTextureImage(struct Engine* engine, VkImage* textureImage, VkDeviceMemory* textureImageMemory)
{
    vkDestroyImage(
        engine->device,
        *textureImage,
        NULL
    );

    vkFreeMemory(
        engine->device,
        *textureImageMemory,
        NULL
    );
}

void createImage(struct Engine* engine, VkFormat format, uint32_t texWidth, uint32_t texHeight, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags propertyFlags, VkImage* stagingImage, VkDeviceMemory* stagingImageMemory)
{
    VkImageCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.imageType = VK_IMAGE_TYPE_2D;
    createInfo.format = format;
    createInfo.extent.width = texWidth;
    createInfo.extent.height = texHeight;
    createInfo.extent.depth = 1;
    createInfo.mipLevels = 1;
    createInfo.arrayLayers = 1;
    createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    createInfo.tiling = tiling;
    createInfo.usage = usage;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = NULL;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

    VkResult result;
    result = vkCreateImage(
        engine->device,
        &createInfo,
        NULL,
        stagingImage
    );
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create texture image.\n");
        exit(-1);
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(
        engine->device,
        *stagingImage,
        &memRequirements
    );

    VkMemoryAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = NULL;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemType(
        engine,
        memRequirements.memoryTypeBits,
        propertyFlags
    );

    result = vkAllocateMemory(
        engine->device,
        &allocInfo,
        NULL,
        stagingImageMemory
    );
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to allocate texture image memory.\n");
        exit(-1);
    }

    vkBindImageMemory(engine->device, *stagingImage, *stagingImageMemory, 0);
}

void transitionImageLayout(struct Engine* engine, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(engine);

    VkImageMemoryBarrier barrier;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = NULL;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (hasStencilComponent(format))
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED &&
             newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
             newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }
    else
    {
        fprintf(stderr, "Unsupported layout transition.\n");
        exit(-1);
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        0,
        0,
        NULL,
        0,
        NULL,
        1,
        &barrier
    );

    endSingleTimeCommands(engine, commandBuffer);
}

void copyImage(struct Engine* engine, VkImage src, VkImage dst, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(engine);

    VkImageSubresourceLayers subresource;
    subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource.baseArrayLayer = 0;
    subresource.mipLevel = 0;
    subresource.layerCount = 1;

    VkImageCopy region;
    region.srcSubresource = subresource;
    region.srcOffset = (VkOffset3D){0, 0, 0};
    region.dstSubresource = subresource;
    region.dstOffset = (VkOffset3D){0, 0, 0};
    region.extent.width = width;
    region.extent.height = height;
    region.extent.depth = 1;

    vkCmdCopyImage(
        commandBuffer,
        src,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dst,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    endSingleTimeCommands(engine, commandBuffer);
}

// TEXTURE IMAGE VIEW
void createTextureImageView(struct Engine* engine, VkImage* textureImage, VkImageView* textureImageView)
{
    createImageView(
        engine,
        *textureImage,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_ASPECT_COLOR_BIT,
        textureImageView
    );
}

void destroyTextureImageView(struct Engine* engine, VkImageView* textureImageView)
{
    vkDestroyImageView(engine->device, *textureImageView, NULL);
}

void createImageView(struct Engine* engine, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* imageView)
{
    VkImageViewCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = aspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    VkResult result;
    result = vkCreateImageView(
        engine->device,
        &createInfo,
        NULL,
        imageView
    );
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create image view for texture.\n");
        exit(-1);
    }
}

// TEXTURE SAMPLER
void createTextureSampler(struct Engine* engine, VkSampler* textureSampler)
{
    VkSamplerCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.magFilter = VK_FILTER_LINEAR;
    createInfo.minFilter = VK_FILTER_LINEAR;
    createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.mipLodBias =0.0f;
    createInfo.anisotropyEnable = VK_TRUE;
    createInfo.maxAnisotropy = 16;
    createInfo.compareEnable = VK_FALSE;
    createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    createInfo.minLod = 0.0f;
    createInfo.maxLod = 0.0f;
    createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    createInfo.unnormalizedCoordinates = VK_FALSE;

    VkResult result;
    result = vkCreateSampler(
        engine->device,
        &createInfo,
        NULL,
        textureSampler
    );
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create texture sampler.\n");
        exit(-1);
    }
}

void destroyTextureSampler(struct Engine* engine, VkSampler* textureSampler)
{
    vkDestroySampler(
        engine->device,
        *textureSampler,
        NULL
    );
}

// VERTEX BUFFER
void createVertexBuffer(struct Engine* engine, struct Vertex* vertices, uint32_t vertexCount, VkBuffer* vertexBuffer, VkDeviceMemory* vertexBufferMemory)
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
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
    memcpy(data, vertices, (size_t)bufferSize);
    vkUnmapMemory(engine->device, stagingBufferMemory);

    createBuffer(
        engine,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vertexBuffer,
        vertexBufferMemory
    );
    copyBuffer(
        engine,
        &(stagingBuffer),
        vertexBuffer,
        bufferSize
    );

    vkDestroyBuffer(engine->device, stagingBuffer, NULL);
    vkFreeMemory(engine->device, stagingBufferMemory, NULL);
}

void copyBuffer(struct Engine* engine, VkBuffer* src, VkBuffer* dst, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(engine);

    VkBufferCopy copyRegion;
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, *src, *dst, 1, &copyRegion);

    endSingleTimeCommands(engine, commandBuffer);
}

VkCommandBuffer beginSingleTimeCommands(struct Engine* engine)
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

    return commandBuffer;
}

void endSingleTimeCommands(struct Engine* engine, VkCommandBuffer commandBuffer)
{
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

void destroyVertexBuffer(struct Engine* engine, VkBuffer* vertexBuffer)
{
    vkDestroyBuffer(engine->device, *vertexBuffer, NULL);
}

void freeVertexBufferMemory(struct Engine* engine, VkDeviceMemory* vertexBufferMemory)
{
    vkFreeMemory(engine->device, *vertexBufferMemory, NULL);
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
void createIndexBuffer(struct Engine* engine, uint32_t* indices, uint32_t indexCount, VkBuffer* indexBuffer, VkDeviceMemory* indexBufferMemory)
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
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
    memcpy(data, indices, (size_t)bufferSize);
    vkUnmapMemory(engine->device, stagingBufferMemory);

    createBuffer(
        engine,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        indexBuffer,
        indexBufferMemory
    );
    copyBuffer(
        engine,
        &(stagingBuffer),
        indexBuffer,
        bufferSize
    );

    vkDestroyBuffer(engine->device, stagingBuffer, NULL);
    vkFreeMemory(engine->device, stagingBufferMemory, NULL);
}

void destroyIndexBuffer(struct Engine* engine, VkBuffer* indexBuffer)
{
    vkDestroyBuffer(engine->device, *indexBuffer, NULL);
}

void freeIndexBufferMemory(struct Engine* engine, VkDeviceMemory* indexBufferMemory)
{
    vkFreeMemory(engine->device, *indexBufferMemory, NULL);
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
    vec3 eye = {
        engine->camera.x,
        engine->camera.y,
        engine->camera.z
    };
    vec3 center = {
        engine->camera.xTarget,
        engine->camera.yTarget,
        engine->camera.zTarget
    };
    vec3 up = {0.0f, 0.0f, 1.0f};
    mat4x4_look_at(engine->ubo.view, eye, center, up);

    mat4x4_perspective(
        engine->ubo.proj,
        (float)degreesToRadians(45.0f),
        engine->swapChainExtent.width/(float)engine->swapChainExtent.height,
        0.1f,
        100.0f
    );

    engine->ubo.proj[1][1] *= -1;

    void* data;
    vkMapMemory(
        engine->device,
        engine->uniformStagingBufferMemory,
        0,
        sizeof(engine->ubo),
        0,
        &data
    );
    memcpy(data, &(engine->ubo), sizeof(engine->ubo));
    vkUnmapMemory(engine->device, engine->uniformStagingBufferMemory);

    copyBuffer(
        engine,
        &(engine->uniformStagingBuffer),
        &(engine->uniformBuffer),
        sizeof(engine->ubo)
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
    VkDescriptorPoolSize poolSizes[2];
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 1;

    VkDescriptorPoolCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.maxSets = 1;
    createInfo.poolSizeCount = 2;
    createInfo.pPoolSizes = poolSizes;

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
void createDescriptorSet(struct Engine* engine, VkDescriptorSet* descriptorSet, VkSampler* textureSampler, VkImageView* textureImageView)
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
        descriptorSet
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

	VkDescriptorImageInfo imageInfo;
    imageInfo.sampler = *textureSampler;
    imageInfo.imageView = *textureImageView;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet descriptorWrites[2];
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].pNext = NULL;
    descriptorWrites[0].dstSet = *descriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].descriptorType =
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].pImageInfo = NULL;
    descriptorWrites[0].pBufferInfo = &bufferInfo;
    descriptorWrites[0].pTexelBufferView = NULL;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].pNext = NULL;
    descriptorWrites[1].dstSet = *descriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].pImageInfo = &imageInfo;
    descriptorWrites[1].pBufferInfo = NULL;
    descriptorWrites[1].pTexelBufferView = NULL;

    vkUpdateDescriptorSets(engine->device, 2, descriptorWrites, 0, NULL);
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

void recordSecondaryCommands(struct Engine* engine, struct GameObject* gameObject)
{
    VkCommandBufferInheritanceInfo inheritanceInfo;
    inheritanceInfo.sType =
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.pNext = NULL;
    inheritanceInfo.renderPass = engine->renderPass;
    inheritanceInfo.subpass = 0;
    inheritanceInfo.framebuffer = engine->framebuffers[engine->currentBuffer];
    inheritanceInfo.occlusionQueryEnable = VK_FALSE;
    inheritanceInfo.queryFlags = 0;
    inheritanceInfo.pipelineStatistics = 0;

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = NULL;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    beginInfo.pInheritanceInfo = &inheritanceInfo;

    vkBeginCommandBuffer(
        gameObject->commandBuffer,
        &beginInfo
    );

    vkCmdBindPipeline(
        gameObject->commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        engine->graphicsPipeline
    );

    // VBO
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(
        gameObject->commandBuffer,
        0,
        1,
        &(gameObject->mesh->vertexBuffer),
        offsets
    );

    // IBO
    vkCmdBindIndexBuffer(
        gameObject->commandBuffer,
        gameObject->mesh->indexBuffer,
        0,
        VK_INDEX_TYPE_UINT32
    );

    // Texture
    vkCmdBindDescriptorSets(
        gameObject->commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        engine->pipelineLayout,
        0,
        1,
        &(gameObject->mesh->descriptor.descriptorSet),
        0,
        NULL
    );

    // Transform
    mat4x4 vp, mvp;
    mat4x4_mul(vp, engine->ubo.proj, engine->ubo.view);
    mat4x4_mul(mvp, vp, gameObject->model);
    vkCmdPushConstants(
        gameObject->commandBuffer,
        engine->pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(mvp),
        &mvp
    );

    // Draw
    vkCmdDrawIndexed(
        gameObject->commandBuffer,
        gameObject->mesh->indexCount,
        1,
        0,
        0,
        0
    );

    vkEndCommandBuffer(gameObject->commandBuffer);
}

void generateDrawCommands(struct Engine* engine, uint32_t currentBuffer)
{
    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = NULL;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = NULL;

    vkBeginCommandBuffer(engine->commandBuffers[currentBuffer], &beginInfo);

    VkRenderPassBeginInfo renderPassInfo;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.pNext = NULL;
    renderPassInfo.renderPass = engine->renderPass;
    renderPassInfo.framebuffer = engine->framebuffers[currentBuffer];
    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent = engine->swapChainExtent;

    VkClearValue clearValues[2];
    clearValues[0].color.float32[0] = 0.9f;
    clearValues[0].color.float32[1] = 1.0f;
    clearValues[0].color.float32[2] = 0.8f;
    clearValues[0].color.float32[3] = 1.0f;
    clearValues[1].depthStencil.depth = 1.0f;
    clearValues[1].depthStencil.stencil = 0;

    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;

    uint32_t i;
    for (i=0; i<engine->gameObjectCount; i++)
    {
        recordSecondaryCommands(engine, &(engine->gameObjects[i]));
    }

    vkCmdBeginRenderPass(
        engine->commandBuffers[currentBuffer],
        &renderPassInfo,
        VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
    );

    VkCommandBuffer secondaryCommands[MAX_OBJECTS];
    for (i=0; i<engine->gameObjectCount; i++)
    {
        secondaryCommands[i] = engine->gameObjects[i].commandBuffer;
    }

    if (engine->gameObjectCount > 0)
    {
        vkCmdExecuteCommands(
            engine->commandBuffers[currentBuffer],
            engine->gameObjectCount,
            secondaryCommands
        );
    }

    vkCmdEndRenderPass(engine->commandBuffers[currentBuffer]);

    VkResult result;
    result = vkEndCommandBuffer(engine->commandBuffers[currentBuffer]);
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to record command buffers.\n");
        exit(-1);
    }
}

// FENCE
void createFence(struct Engine* engine)
{
    VkFenceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.flags = 0;

    VkResult result;
    result = vkCreateFence(
        engine->device,
        &createInfo,
        NULL,
        &(engine->renderFence)
    );
    if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create fence.\n");
        exit(-1);
    }
}

void destroyFence(struct Engine* engine)
{
    vkDestroyFence(engine->device, engine->renderFence, NULL);
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
        engine->renderFence
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

    do
    {
        result = vkWaitForFences(
            engine->device,
            1,
            &(engine->renderFence),
            VK_TRUE,
            100000000
        );
    }
    while (result == VK_TIMEOUT);
    vkResetFences(engine->device, 1, &(engine->renderFence));

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

    destroyDepthResources(engine);
    createDepthResources(engine);

    destroyFramebuffers(engine);
    createFramebuffers(engine);

    freeCommandBuffers(engine);
    createCommandBuffers(engine);
}
