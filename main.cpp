#include <algorithm>
#include <iostream>
#include <limits>
#include <set>
// #include<vulkan/vulkan.h>
#define VK_USE_PLATFORM_XCB_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>
#define GLFW_EXPOSE_NATIVE_XCB
#include <GLFW/glfw3native.h>
// #include<vulkan/vulkan_xcb.h>

// 结构体必须要通过花括号初始化，不然里面的值就会随机初始化
const int HEIGHT = 800;
const int WIDTH = 600;

const std::vector<const char *> validationLayers{"VK_LAYER_KHRONOS_validation"};
const std::vector<const char *> deviceExtensions{"VK_KHR_swapchain"};

#ifdef DEBUG
bool enableValidation = true;
#else
bool enableValidation = false;
#endif

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentations;
};

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentationFamily;
  bool isComplete() const {
    return graphicsFamily.has_value() && presentationFamily.has_value();
  }
};

class HelloTriangle {
public:
  void run() {
    initWindow();
    initVulkan();
    mainLoop();
    clearUp();
  }

private:
  void initWindow() {
    if (glfwInit() == GLFW_FALSE) {
      throw std::exception();
    }

    // 使用vulkan 的时候需要关闭glfw对于opengl的支持
    glfwWindowHint(GLFW_CLIENT_API,
                   GLFW_NO_API); // Client Api 是专门为了OpenGL准备的。
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // 禁止window窗口缩放。
    m_window =
        glfwCreateWindow(HEIGHT, WIDTH, "Vulkan Triangle", nullptr, nullptr);
  }
  void initVulkan() {
    createInstance();
    createWindowSurface(); // surface
                           // 的创建必须在instance之后，且在选择物理设备之前,因为选择物理设备的时候必须半段该物理设备是否支持surface显示。
    selectPhysicalDevice();
    createLogicDevice();
    createSwapChain();
  }
  void mainLoop() {
    while (!glfwWindowShouldClose(m_window)) {
      glfwPollEvents();
    }
  }
  void clearUp() {
    vkDestroySwapchainKHR(m_logicDevice, m_swapchain, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyDevice(m_logicDevice, nullptr);
    vkDestroyInstance(m_instance, nullptr);
    glfwDestroyWindow(m_window);
    glfwTerminate();
  }

  void createWindowSurface() {
    if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create window surface.");
    }
  }
  void createLogicDevice() {
    QueueFamilyIndices indices = findQueueFamilies(m_device);
    VkDeviceQueueCreateInfo queueInfo{};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = indices.graphicsFamily.value();
    queueInfo.queueCount = 1;
    float queuePriority = 1.0;
    queueInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures features{};
    vkGetPhysicalDeviceFeatures(m_device, &features);
    VkDeviceCreateInfo createInfo{}; // 必须使用花括号初始化，不然就会出问题
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pEnabledFeatures = &features;
    createInfo.pQueueCreateInfos = &queueInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.enabledExtensionCount = 0;
    createInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidation) {
      createInfo.ppEnabledLayerNames = validationLayers.data();
      createInfo.enabledLayerCount = (uint32_t)validationLayers.size();
    } else {
      createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_device, &createInfo, nullptr, &m_logicDevice) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create logic device.");
    }
  }

  void selectPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
      throw std::runtime_error("can't find Device supporting vulkan.");
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    for (auto &device : devices) {
      if (isPhysicalDeviceSuitable(device)) {
        m_device = device;
        break;
      }
    }

    if (m_device == VK_NULL_HANDLE) {
      throw std::runtime_error("There no exists a suitable device.");
    }
  }

  void createInstance() {
    supportedInstanceExtensions();

    if (enableValidation && !vkSupportLayers()) {
      throw std::runtime_error(
          "validation layer is enabled, but not available");
    }
    // application info
    // extension info
    VkApplicationInfo info{};
    info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    info.pNext = nullptr;
    info.pApplicationName = "Vulkan";
    info.applicationVersion = VK_MAKE_VERSION(1, 1, 0);
    info.pEngineName = "No Engine";
    info.engineVersion = VK_MAKE_VERSION(1, 1, 0);
    info.apiVersion =
        VK_API_VERSION_1_0; // api version 和 普通的version数据格式不一样

    VkInstanceCreateInfo instanceInfo;
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = nullptr;
    instanceInfo.pApplicationInfo = &info;

    uint32_t extensionCount = 0;
    const char **extensionName;
    extensionName = glfwGetRequiredInstanceExtensions(
        &extensionCount); // instance level extensions

    std::vector<const char *> requiredExtensions;
    for (int i = 0; i < extensionCount; ++i) { // 为了解决驱动不兼容的问题
      requiredExtensions.emplace_back(extensionName[i]);
      std::cout << extensionName[i] << std::endl;
    }
    std::cout << VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME << std::endl;
    requiredExtensions.emplace_back(
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME); // VK_KHR_portability_enumeration
                                                        // 专门为MacOS准备的

    instanceInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    instanceInfo.enabledExtensionCount = requiredExtensions.size();
    instanceInfo.ppEnabledExtensionNames = requiredExtensions.data();

    if (enableValidation) {
      instanceInfo.enabledLayerCount = (uint32_t)validationLayers.size();
      instanceInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
      instanceInfo.enabledLayerCount = 0;
    }

    VkResult result = vkCreateInstance(&instanceInfo, nullptr, &m_instance);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("failed to create instance.");
    }
  }

  void supportedInstanceExtensions() {
    // check for extensions support.
    uint32_t extSupportCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extSupportCount, nullptr);
    std::vector<VkExtensionProperties> extensionProperties(extSupportCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extSupportCount,
                                           extensionProperties.data());
    std::cout << "Extensions suppored by vulkan" << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
    for (int i = 0; i < extSupportCount; ++i) {
      std::cout << extensionProperties[i].extensionName << "\t"
                << extensionProperties[i].specVersion << std::endl;
    }

    std::cout
        << "--------------------------------------------------------------"
        << std::endl;
  }

  bool vkSupportLayers() {
    uint32_t layerSupportCount = 0;
    vkEnumerateInstanceLayerProperties(&layerSupportCount, nullptr);
    std::vector<VkLayerProperties> layerProperties(layerSupportCount);
    vkEnumerateInstanceLayerProperties(&layerSupportCount,
                                       layerProperties.data());

    std::cout << "Validation Layers :" << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
    for (auto item : validationLayers) {
      bool isMatch = false;
      for (auto lp : layerProperties) {
        std::cout << lp.layerName << std::endl;
        if (0 == strcmp(item, lp.layerName)) {
          isMatch = true;
          break;
        }
      }
      if (isMatch == false) {
        return false;
      }
    }
    std::cout << "---------------------------------------------" << std::endl;

    return true;
  }

  bool isPhysicalDeviceSuitable(VkPhysicalDevice &device) {
    QueueFamilyIndices indices = findQueueFamilies(device);
    if (!indices.isComplete())
      return false; // 如果该设备不支持图形处理操作，那么就直接返回false.

    bool isSwapChain = checkDeviceExtensionSupport(device);
    if (!isSwapChain)
      return false;

    SwapChainSupportDetails details = querySwapchainDetails(device);
    if (details.formats.empty() || details.presentations.empty())
      return false;

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    std::cout << "Device Informations : " << std::endl;
    std::cout << "Device ID : " << properties.deviceID << std::endl;
    std::cout << "Device Name : " << properties.deviceName << std::endl;
    std::cout << "Device Type : " << properties.deviceType << std::endl;
    std::cout << "Driver Version : " << properties.driverVersion << std::endl;

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);
    return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
           features.geometryShader;
  }

  bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         availableExtensions.data());
    std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                             deviceExtensions.end());
    std::cout << "Device Support Extensions :" << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
    for (const auto &ext : availableExtensions) {
      std::cout << ext.extensionName << std::endl;
      requiredExtensions.erase(ext.extensionName);
    }
    std::cout << "---------------------------------------------" << std::endl;
    return requiredExtensions.empty();
  }

  SwapChainSupportDetails querySwapchainDetails(VkPhysicalDevice device) {
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface,
                                              &details.capabilities);
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount,
                                         nullptr);
    if (formatCount != 0) {
      details.formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount,
                                           details.formats.data());
    }

    uint32_t presentCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentCount,
                                              nullptr);
    if (presentCount != 0) {
      details.presentations.resize(presentCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(
          device, m_surface, &presentCount, details.presentations.data());
    }

    return details;
  }

  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> availableFormats) {
    for (auto &format : availableFormats) {
      if (format.format == VK_FORMAT_R8G8B8A8_SRGB &&
          format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        return format;
      }
    }
    return availableFormats[0];
  }

  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresents) {
    for (auto &present : availablePresents) {
      if (present == VK_PRESENT_MODE_MAILBOX_KHR) {
        return present;
      }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
  }

  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width !=
        std::numeric_limits<uint32_t>::max()) {
      return capabilities.currentExtent;
    } else {
      int width, height;
      glfwGetFramebufferSize(m_window, &width, &height);

      VkExtent2D retExtent{(uint32_t)width, (uint32_t)height};
      retExtent.width =
          std::clamp(retExtent.width, capabilities.minImageExtent.width,
                     capabilities.maxImageExtent.width);
      retExtent.height =
          std::clamp(retExtent.height, capabilities.minImageExtent.height,
                     capabilities.maxImageExtent.height);
      return retExtent;
    }
  }

  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indecies;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             nullptr);
    std::vector<VkQueueFamilyProperties> properties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             properties.data());
    std::cout << "Queue Family Count : " << queueFamilyCount << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
    for (int i = 0; i < queueFamilyCount; ++i) {
      std::cout << "queue Count :" << properties[i].queueCount << std::endl;
      std::cout << "queue Type :" << properties[i].queueFlags << std::endl;
      if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        indecies.graphicsFamily = i;
        std::cout << i << "th queueFamily support graphics operation."
                  << std::endl;
      }
      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface,
                                           &presentSupport);
      if (presentSupport) {
        if (!indecies.presentationFamily.has_value() ||
            indecies.presentationFamily.value() !=
                indecies.graphicsFamily.value())
          indecies.presentationFamily = i;
        std::cout << i << "th queueFamily support present operation"
                  << std::endl;
      }
    }
    std::cout << "---------------------------------------------" << std::endl;

    return indecies;
  }

  void createSwapChain() {
    SwapChainSupportDetails details = querySwapchainDetails(m_device);

    VkSurfaceFormatKHR format = chooseSwapSurfaceFormat(details.formats);
    VkPresentModeKHR present = chooseSwapPresentMode(details.presentations);
    VkExtent2D extent = chooseSwapExtent(details.capabilities);

    uint32_t imageCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 &&
        imageCount > details.capabilities.maxImageCount) {
      imageCount = details.capabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.imageExtent = extent;
    createInfo.imageFormat = format.format;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.presentMode = present;
    createInfo.minImageCount = imageCount;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.clipped = VK_TRUE;
    createInfo.preTransform = details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    QueueFamilyIndices indices = findQueueFamilies(m_device);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                     indices.presentationFamily.value()};
    if (indices.graphicsFamily.value() != indices.presentationFamily.value()) {
      createInfo.queueFamilyIndexCount = 2;
      createInfo.imageSharingMode =
          VK_SHARING_MODE_CONCURRENT; // implicitly transfer the ownership of
                                      // images.
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
      createInfo.queueFamilyIndexCount = 0;
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      createInfo.pQueueFamilyIndices = nullptr;
    }

    if (vkCreateSwapchainKHR(m_logicDevice, &createInfo, nullptr,
                             &m_swapchain) != VK_SUCCESS) {
      throw std::runtime_error("failed to create swap chain");
    }

    vkGetSwapchainImagesKHR(m_logicDevice, m_swapchain, &imageCount, nullptr);
    images_.resize(imageCount);
    vkGetSwapchainImagesKHR(m_logicDevice, m_swapchain, &imageCount,
                            images_.data());
    m_swapchain_format = format.format;
    m_swapchain_extent = extent;
  }

private:
  GLFWwindow *m_window;
  VkInstance m_instance;
  VkDevice m_logicDevice;
  VkPhysicalDevice m_device = VK_NULL_HANDLE;
  VkSurfaceKHR m_surface;

  VkSwapchainKHR m_swapchain;
  std::vector<VkImage> images_;
  VkFormat m_swapchain_format;
  VkExtent2D m_swapchain_extent;
};

int main() {
  HelloTriangle app;
  try {
    app.run();
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
