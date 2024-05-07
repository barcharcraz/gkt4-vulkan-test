#include <vulkan/vulkan_core.h>
#define VK_USE_PLATFORM_WIN32_KHR
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <vulkan/vulkan.h>

typedef struct vkt_data {
  VkInstance inst;
  VkDevice dev;
  VkQueue q;
} vkt_data;

struct vkt_surface_data {
  VkSurfaceKHR surface;
  VkSwapchainKHR swapchain;
};

struct vkt_data init_instance_and_device() {
  VkInstance instance;
  vkCreateInstance(
      &(VkInstanceCreateInfo){
          .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
          .pApplicationInfo =
              &(VkApplicationInfo){.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                   .pApplicationName = "gtk-vulkan-test",
                                   .apiVersion = VK_API_VERSION_1_3},
          .ppEnabledExtensionNames =
              (const char *[]){"VK_KHR_win32_surface, VK_KHR_surface"},
          .enabledExtensionCount = 2,
          .ppEnabledLayerNames =
              (const char *[]){"VK_LAYER_KHRONOS_validation"},
          .enabledLayerCount = 1},
      nullptr, &instance);
  VkDevice dev;
  uint32_t dev_count;
  VkPhysicalDevice *devices;
  vkEnumeratePhysicalDevices(instance, &dev_count, nullptr);
  if (!dev_count)
    abort();
  devices = calloc(dev_count, sizeof(VkPhysicalDevice));
  vkEnumeratePhysicalDevices(instance, &dev_count, devices);
  vkCreateDevice(
      devices[0],
      &(VkDeviceCreateInfo){
          .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
          .queueCreateInfoCount = 1,
          .pQueueCreateInfos =
              &(VkDeviceQueueCreateInfo){
                  .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                  .queueFamilyIndex = 0,
                  .queueCount = 1,
                  .pQueuePriorities = (float[]){1.0}},
          .ppEnabledExtensionNames =
              (const char *[]){
                  "VK_KHR_swapchain",
              },
          .enabledExtensionCount = 1,
      },
      nullptr, &dev);
  VkQueue q;
  vkGetDeviceQueue(dev, 0, 0, &q);
  return (struct vkt_data){instance, dev, q};
}

struct vkt_surface_data init_surface_data(struct vkt_data *ctx, void *hwnd) {
  VkSurfaceKHR result;
  vkCreateWin32SurfaceKHR(
      ctx->inst,
      &(VkWin32SurfaceCreateInfoKHR){
          .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
          .hinstance = GetModuleHandleW(nullptr),
          .hwnd = hwnd},
      nullptr, &result);
  RECT r;
  if(!GetWindowRect(hwnd, &r)) abort();

//   vkCreateSwapchainKHR(ctx->dev, &(VkSwapchainCreateInfoKHR) {
//     .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, 
//     .surface = result,
//     .imageExtent = (VkExtent2D) {
//         .height = r.bottom - r.top,
//         .width = r.right - r.left
//     },
//     .imageFormat = VK_FORMAT_B8G8R8_UNORM,
//     .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,

//   }, const VkAllocationCallbacks *pAllocator, VkSwapchainKHR *pSwapchain)
  
}