#include<iostream>
//#include<vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
#include<vulkan/vulkan_core.h>
#include<exception>
#include<cstdlib>
#include<vector>
#include<cstring>
#include<optional>


const int HEIGHT = 800;
const int WIDTH = 600;

const std::vector<const char*> validationLayers{
"VK_LAYER_KHRONOS_validation"
};

#ifdef DEBUG
    bool enableValidation = true;
#else
    bool enableValidation = false;
#endif

struct QueueFamilyIndices{
    std::optional<uint32_t> graphicsFamily;
    bool isComplete()const { return graphicsFamily.has_value();}
};

class HelloTriangle{
public:
    void run(){
        initWindow();
        initVulkan();
        mainLoop();
        clearUp();
    }

private:
    void initWindow(){
        if(glfwInit() == GLFW_FALSE){
            throw std::exception();
        }

        // 使用vulkan 的时候需要关闭glfw对于opengl的支持
        glfwWindowHint(GLFW_CLIENT_API, GLFW_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        m_window = glfwCreateWindow(HEIGHT,WIDTH,"Vulkan Triangle", nullptr, nullptr);

    }
    void initVulkan(){
        createInstance();
        selectPhysicalDevice();
        createLogicDevice();
    }
    void mainLoop(){
        while(!glfwWindowShouldClose(m_window)){
            glfwPollEvents();
        }
    }
    void clearUp(){
        vkDestroyDevice(m_logicDevice, nullptr);
        vkDestroyInstance(m_instance, nullptr);
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }


    void createLogicDevice(){
        QueueFamilyIndices indices = findQueueFamilies(m_device);
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = indices.graphicsFamily.value();
        queueInfo.queueCount = 1;
        float queuePriority = 1.0;
        queueInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures features{};
    
        VkDeviceCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pEnabledFeatures = &features;
        createInfo.pQueueCreateInfos = &queueInfo;
        createInfo.queueCreateInfoCount = 1;
        if(enableValidation){
            createInfo.ppEnabledLayerNames = validationLayers.data();
            createInfo.enabledLayerCount = (uint32_t)validationLayers.size();
        }else{
            createInfo.enabledLayerCount = 0;
        }

        if(vkCreateDevice(m_device, &createInfo, nullptr, &m_logicDevice) != VK_SUCCESS){
            throw std::runtime_error("failed to create logic device.");
        }


    }

    void selectPhysicalDevice(){
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
        if(deviceCount == 0){
            throw std::runtime_error("can't find Device supporting vulkan.");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data()); 

        for(auto& device : devices){
            if(isPhysicalSuitable(device)){
                m_device = device;
                break;
            }
        }

        if(m_device == VK_NULL_HANDLE){
            throw std::runtime_error("There no exists a suitable device.");
        }
    }

    void createInstance(){
        vkSupportedExtensions(); 

        if(enableValidation && !vkSupportLayers()){
            throw std::runtime_error("validation layer is enabled, but not available");
        }
        // application info
        // extension info
        VkApplicationInfo info{};
        info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        info.pNext = nullptr;
        info.pApplicationName = "Vulkan";
        info.applicationVersion = VK_MAKE_VERSION(1, 1,0);
        info.pEngineName = "No Engine";
        info.engineVersion = VK_MAKE_VERSION(1, 1, 0);
        info.apiVersion = VK_API_VERSION_1_0; // api version 和 普通的version数据格式不一样

        VkInstanceCreateInfo instanceInfo;
        instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.pNext = nullptr;
        instanceInfo.pApplicationInfo = &info;
        
        uint32_t extensionCount = 0;
        const char** extensionName;
        extensionName = glfwGetRequiredInstanceExtensions(&extensionCount);

        std::vector<const char*> requiredExtensions;
        for(int i = 0; i < extensionCount; ++i){ // 为了解决驱动不兼容的问题
            requiredExtensions.emplace_back(extensionName[i]);
            std::cout<< extensionName[i]<<std::endl;
        }
        std::cout<< VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME<<std::endl;
        requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

        instanceInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        instanceInfo.enabledExtensionCount = requiredExtensions.size();
        instanceInfo.ppEnabledExtensionNames = requiredExtensions.data();

        if(enableValidation){
           instanceInfo.enabledLayerCount = (uint32_t)validationLayers.size(); 
           instanceInfo.ppEnabledLayerNames = validationLayers.data();
        }else{
            instanceInfo.enabledLayerCount = 0;
        }

        VkResult result = vkCreateInstance(&instanceInfo, nullptr, &m_instance);
        if(result != VK_SUCCESS){
            throw std::runtime_error("failed to create instance.");
        }


    }

    void vkSupportedExtensions(){
        // check for extensions support.
        uint32_t extSupportCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extSupportCount, nullptr);
        std::vector<VkExtensionProperties> extensionProperties(extSupportCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extSupportCount, extensionProperties.data());
        std::cout<< "Extensions suppored by vulkan"<<std::endl;
        std::cout<<"---------------------------------------------"<<std::endl;
        for(int i = 0; i < extSupportCount; ++i){
            std::cout<< extensionProperties[i].extensionName <<"\t" <<extensionProperties[i].specVersion<<std::endl;
        }

        std::cout<<"--------------------------------------------------------------"<<std::endl;
    }

    bool vkSupportLayers(){
        uint32_t layerSupportCount = 0;
        vkEnumerateInstanceLayerProperties(&layerSupportCount, nullptr);
        std::vector<VkLayerProperties> layerProperties(layerSupportCount);
        vkEnumerateInstanceLayerProperties(&layerSupportCount, layerProperties.data());

        std::cout<<"Validation Layers :"<<std::endl;
        std::cout<<"---------------------------------------------"<<std::endl;
        for(auto item : validationLayers){
            bool isMatch = false;
            for(auto lp : layerProperties){
                std::cout<< lp.layerName<<std::endl;
                if(0 == strcmp(item, lp.layerName)){
                   isMatch = true;
                   break;
                }
            }
            if(isMatch == false){
                return false;
            }

        }
        std::cout<<"---------------------------------------------"<<std::endl;

        return true;
    }

    bool isPhysicalSuitable(VkPhysicalDevice& device){
        QueueFamilyIndices indices = findQueueFamilies(device); 
        if(!indices.isComplete())  return false; // 如果该设备不支持图形处理操作，那么就直接返回false.
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        std::cout<< "Device Informations : "<<std::endl;
        std::cout<< "Device ID : "<<properties.deviceID<<std::endl;
        std::cout<< "Device Name : "<<properties.deviceName<<std::endl;
        std::cout<< "Device Type : "<<properties.deviceType<<std::endl;
        std::cout<< "Driver Version : "<<properties.driverVersion<<std::endl;
         

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(device, &features);
        return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && features.geometryShader;
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device){
        QueueFamilyIndices indecies;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> properties(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, properties.data());
        std::cout<< "Queue Family Count : "<<queueFamilyCount<<std::endl;
        std::cout<<"---------------------------------------------"<<std::endl;
        for(int i = 0; i < queueFamilyCount; ++i){
            std::cout<< "queue Count :"<<properties[i].queueCount<<std::endl;
            std::cout<< "queue Type :" <<properties[i].queueFlags<<std::endl;
            if(properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
                indecies.graphicsFamily = i;
                std::cout<< i<<"th queueFamily support graphics operation."<<std::endl; 
            }
        }
        std::cout<<"---------------------------------------------"<<std::endl;

        return indecies;
    }

private:
    GLFWwindow *m_window;
    VkInstance m_instance;
    VkDevice m_logicDevice;
    VkPhysicalDevice m_device = VK_NULL_HANDLE;
};


int main(){
    HelloTriangle app;
    try {
        app.run();
    }catch(std::exception& e) {
        std::cerr<< e.what()<<std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


