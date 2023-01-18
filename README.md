# Vulkan

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
