# Vulkan

## 1. Instance

+ step 1: 创建glfw窗口, 建立mainLoop
+ step 2: 创建vulkan instance. 

创建vulkan instance 的步骤基本固定，声明create info， 调用vkCreatexxx.
创建instance的目的有两个:
1. 连接 application 和 vulkan library
2. 告诉driver应用的设置
