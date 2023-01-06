#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
// #include<vulkan/vulkan.h>
#include<iostream>
#include<stdexcept>
#include<cstdlib>


const int HEIGHT = 800;
const int WIDTH  = 600;


class HelloTriangleApplication{
  public:
    void run(){
      initWindow();
      initVulkan();
      mainLoop();
      cleanup();
    }
  private:
    void initWindow(){
      glfwInit(); // init glfw library.
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // prevent creating context for opengl.
      glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // prevent resize the window for now.

      window = glfwCreateWindow(HEIGHT, WIDTH, "Vulkan Window", NULL, NULL);
    }

    void initVulkan(){
      createInstance();
    }

    void mainLoop(){
      while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
      }
    }

    void cleanup(){
      vkDestroyInstance(instance, nullptr);
      glfwDestroyWindow(window);
      glfwTerminate();
    }

    void createInstance(){
      VkApplicationInfo appInfo;
      appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      appInfo.pApplicationName = "Hello Triangle";
      appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
      appInfo.pNext = nullptr;
      appInfo.pEngineName = "No Engine";
      appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
      appInfo.apiVersion = VK_API_VERSION_1_0;

      uint32_t extensionCount = 0;
      const char** extensionName;
      extensionName = glfwGetRequiredInstanceExtensions(&extensionCount);
      VkInstanceCreateInfo createInfo;
      createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      createInfo.pApplicationInfo = &appInfo; 
      createInfo.enabledExtensionCount = extensionCount;
      createInfo.ppEnabledExtensionNames = extensionName;
      createInfo.enabledLayerCount = 0; // 必须指定validation layer. 即使为空

      if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS){
        throw std::runtime_error("Can't create Instance.");
      }

    }
  private:
    GLFWwindow* window;
    VkInstance instance;
};


int main(){
  HelloTriangleApplication app;

  try{
    app.run();
  }catch(const std::exception& err){
    std::cout<< err.what() <<std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}