#ifndef ENGINE_H_
#define ENGINE_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// From VulkanSDK examples
#include "linmath.h"

#define MAX_MESHES 20
#define MAX_OBJECTS 500

// MIN/MAX
uint32_t min(uint32_t a, uint32_t b);
uint32_t max(uint32_t a, uint32_t b);

// STRUCTURES
struct Vertex
{
    float position[3];
    float color[3];
    float texCoord[2];
};

struct UniformBufferObject
{
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

struct TextureImage
{
    VkImage image;
    VkDeviceMemory memory;
    VkImageView imageView;
    VkSampler sampler;
    uint32_t width, height;
    VkDescriptorImageInfo info;
};

struct Descriptor
{
    struct TextureImage textureImage;
    VkDescriptorSet descriptorSet;
};

struct Mesh
{
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexMemory;
    uint32_t indexCount;
    VkBuffer indexBuffer;
    VkDeviceMemory indexMemory;
    struct Descriptor descriptor;
};

struct GameObject
{
    struct Mesh* mesh;
    vec3 rotation;
    vec3 position;
    mat4x4 model;
    VkCommandBuffer commandBuffer;
    _Bool visible;
};

struct Camera
{
    float x, y, z;
    float xTarget, yTarget, zTarget;
    float angle;
};

struct Engine
{
    // Key callback
    void (*keyCallback)(void*, int, int);
    void (*gameLoopCallback)();

    // Camera
    struct Camera camera;

    // User pointer
    void* userPointer;

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
    VkPhysicalDeviceProperties physicalDeviceProperties;
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

	// Depth buffering
    VkFormat depthFormat;
	VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    // Game objects
    struct GameObject gameObjects[MAX_OBJECTS];
    uint32_t gameObjectCount;

    // Meshes
    struct Mesh meshes[MAX_MESHES];
    uint32_t meshCount;

    // Uniform buffer
    VkBuffer uniformStagingBuffer;
    VkDeviceMemory uniformStagingBufferMemory;
    VkBuffer uniformBuffer;
    VkDeviceMemory uniformBufferMemory;
    struct UniformBufferObject ubo;

    // Descriptor pool/set layout
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;

    // Command buffers
    VkCommandBuffer* commandBuffers;
    uint32_t currentBuffer;

    // Fence
    VkFence renderFence;

    // Semaphores
    VkSemaphore imageAvailable;
    VkSemaphore renderFinished;
};
struct GameObject* EngineCreateGameObject(
    struct Engine* self,
    struct Mesh* mesh
);
void EngineDestroyGameObject(
    struct Engine* self,
    struct GameObject* gameObject
);
struct Mesh* EngineCreateMesh(
    struct Engine* self,
    struct Vertex* vertices,
    uint32_t vertexCount,
    uint32_t* indices,
    uint32_t indexCount
);
void EngineDestroyMesh(
    struct Engine* self,
    struct Mesh* mesh
);
void EngineCreateDescriptor(
    struct Engine* self,
    struct Descriptor* descriptor,
    const char* textureSrc
);
void EngineDestroyDescriptor(
    struct Engine* self,
    struct Descriptor* descriptor
);
struct Mesh* EngineLoadModel(
    struct Engine* self,
    const char* path
);
void EngineInit(struct Engine* self);
void EngineUpdate(struct Engine* self);
void EngineRun(struct Engine* self);
void EngineDestroy(struct Engine* self);

// GLFW
void onWindowResized(
    GLFWwindow* window,
    int width,
    int height
);
void keyCallback(
    GLFWwindow* window,
    int key,
    int scancode,
    int action,
    int mods
);

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
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
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

// DEPTH FORMAT
void findDepthFormat(struct Engine* engine);

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
VkVertexInputBindingDescription getBindingDescription();
VkVertexInputAttributeDescription* getAttributeDescriptions();

// FRAMEBUFFERS
void createFramebuffers(struct Engine* engine);
void destroyFramebuffers(struct Engine* engine);

// COMMAND POOL
void createCommandPool(struct Engine* engine);
void destroyCommandPool(struct Engine* engine);

// DEPTH RESOURCES
void createDepthResources(struct Engine* engine);
VkFormat findSupportedFormat(
    struct Engine* engine,
    VkFormat* candidates,
    uint32_t candidateCount,
    VkImageTiling tiling,
    VkFormatFeatureFlags features
);
_Bool hasStencilComponent(VkFormat format);
void destroyDepthResources(struct Engine* engine);

// TEXTURE IMAGE
void createTextureImage(
    struct Engine* engine,
    const char* textureSrc,
    VkImage* textureImage,
    VkDeviceMemory* textureImageMemory
);
void destroyTextureImage(
    struct Engine* engine,
    VkImage* textureImage,
    VkDeviceMemory* textureImageMemory
);
void createImage(
    struct Engine* engine,
    VkFormat format,
    uint32_t texWidth,
    uint32_t texHeight,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags propertyFlags,
    VkImage* stagingImage,
    VkDeviceMemory* stagingImageMemory
);
void transitionImageLayout(
    struct Engine* engine,
    VkImage image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout
);
void copyImage(
    struct Engine* engine,
    VkImage src,
    VkImage dst,
    uint32_t width,
    uint32_t height
);

// TEXTURE IMAGE VIEW
void createTextureImageView(
    struct Engine* engine,
    VkImage* textureImage,
    VkImageView* textureImageView
);
void destroyTextureImageView(
    struct Engine* engine,
    VkImageView* textureImageView
);
void createImageView(
    struct Engine* engine,
    VkImage image,
    VkFormat format,
    VkImageAspectFlags aspectFlags,
    VkImageView* imageView
);

// TEXTURE SAMPLER
void createTextureSampler(struct Engine* engine, VkSampler* textureSampler);
void destroyTextureSampler(struct Engine* engine, VkSampler* textureSampler);

// VERTEX BUFFER
void createVertexBuffer(
    struct Engine* engine,
    struct Vertex* vertices,
    uint32_t vertexCount,
    VkBuffer* vertexBuffer,
    VkDeviceMemory* vertexBufferMemory
);
void copyBuffer(
    struct Engine* engine,
    VkBuffer* src,
    VkBuffer* dst,
    VkDeviceSize size
);
VkCommandBuffer beginSingleTimeCommands(struct Engine* engine);
void endSingleTimeCommands(
    struct Engine* engine,
    VkCommandBuffer commandBuffer
);
void createBuffer(
    struct Engine* engine,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer* buffer,
    VkDeviceMemory* bufferMemory
);
void destroyVertexBuffer(struct Engine* engine, VkBuffer* vertexBuffer);
void freeVertexBufferMemory(
    struct Engine* engine,
    VkDeviceMemory* vertexBufferMemory
);
uint32_t findMemType(
    struct Engine* engine,
    uint32_t typeFilter,
    VkMemoryPropertyFlags properties
);

// DESCRIPTOR LAYOUT
void createDescriptorSetLayout(struct Engine* engine);
void destroyDescriptorSetLayout(struct Engine* engine);

// INDEX BUFFER
void createIndexBuffer(
    struct Engine* engine,
    uint32_t* indices,
    uint32_t indexCount,
    VkBuffer* indexBuffer,
    VkDeviceMemory* indexBufferMemory
);
void destroyIndexBuffer(struct Engine* engine, VkBuffer* indexBuffer);
void freeIndexBufferMemory(
    struct Engine* engine,
    VkDeviceMemory* indexBufferMemory
);

// UNIFORM BUFFER
void createUniformBuffer(struct Engine* engine);
void updateUniformBuffer(struct Engine* engine);
void freeUniformBufferMemory(struct Engine* engine);
void destroyUniformBuffer(struct Engine* engine);

// DESCRIPTOR POOL
void createDescriptorPool(struct Engine* engine);
void destroyDescriptorPool(struct Engine* engine);

// DESCRIPTOR SET
void createDescriptorSet(
    struct Engine* engine,
    VkDescriptorSet* descriptorSet,
    VkSampler* textureSampler,
    VkImageView* textureImageView
);
// Automatically freed when pool is destroyed

// COMMAND BUFFERS
void createCommandBuffers(struct Engine* engine);
void freeCommandBuffers(struct Engine* engine);
void generateDrawCommands(
    struct Engine* engine,
    uint32_t currentBuffer
);

// FENCE
void createFence(struct Engine* engine);
void destroyFence(struct Engine* engine);

// SEMAPHORES
void createSemaphores(struct Engine* engine);
void destroySemaphores(struct Engine* engine);

// DRAW FRAME
void drawFrame(struct Engine* engine);

// RECREATE SWAPCHAIN
void recreateSwapChain(struct Engine* engine);

#endif
