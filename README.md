# Vulkan

```
1. instance
    |
    |
    |---applicationInfo
    |
    |---createInfo -- |  validation(VK_KHR_validation) & extension(glfwGetRequiredExtensions)

2. Surface
    |
    | --- glfwCreateWindowSurface 直接创建和window相关的surface

3. swapchain
    | 
    | --- 基于physical device and logical device --> 获得swapchain的细节，为了兼容surface.
    |
    |

4. phsical device
    |
    |--- vkEnumeratePhsicalDevice  : 找到所有的物理设备
    |--- find queue families that match graphic queue family and present queue family for each device ,check for requried extensions of device.Then determine if it's suitable for device's property , features, .

5. logical device
    |
    | --- 
```

## 1. Instance

+ step 1: 创建glfw窗口, 建立mainLoop
+ step 2: 创建vulkan instance. 

创建vulkan instance 的步骤基本固定，声明create info， 调用vkCreatexxx.
创建instance的目的有两个:
1. 连接 application 和 vulkan library
2. 告诉driver应用的设置

创建instance 时，需要通过结构体VkInstanceCreateInfo将拓展和validation告诉driver。

## 2. Physical Device
选择合适的物理设备，通过vkEnumeratePhysicalDevice函数找到系统中的Device个数和每个设备的信息。
再根据设备的property和features以及队列组(queue family)来判断使用哪个device。

## 3. Logical Device
Logical Device是用来和Physical Device进行通信的。根据队列信息，拓展和验证layer等信息，
我们创建出可以和物理设备通话的逻辑设备。


## 4. Swap Chain
交换链的创建流程:
1. 判断是否支持Swap chain的拓展 -- VK_KHR_swapchain
2. 修改选择物理设备physical device的条件
3. 为了让swapchain 和surface兼容，我们需要知道surface的具体细节 capability(width, height), format(format_type and color_type) , PresentMode(fifo/mailbox)
4. 创建swapchain
