
#define VK_USE_PLATFORM_WAYLAND_KHR

#include <iostream>
#include <vector>

#include <wayland-client.h>
#include <vulkan/vulkan.h>

int main() {
    auto display = wl_display_connect(nullptr);
    if (!display) {
        std::cerr << "wl_display_connect failed..." << std::endl;
        return -1;
    }
    auto registry = wl_display_get_registry(display);
    if ( !registry) {
        std::cerr << "wl_display_get_registry failed..." << std::endl;
        return -1;
    }
    wl_compositor* compositor = nullptr;
    wl_registry_listener listener = {
        .global = [](auto data, auto registry, auto name, auto interface, auto version) noexcept {
            if (std::string_view(interface) == wl_compositor_interface.name) {
                *reinterpret_cast<void**>(data) = wl_registry_bind(registry,
                                                                   name,
                                                                   &wl_compositor_interface,
                                                                   version);
            }
        },
        .global_remove = [](auto...) noexcept { },
    };
    wl_registry_add_listener(registry, &listener, &compositor);
    wl_display_roundtrip(display);
    if (!compositor) {
        std::cerr << "registry_bind(compositor) failed..." << std::endl;
        return -1;
    }
    auto surface = wl_compositor_create_surface(compositor);
    if (!surface) {
        std::cerr << "wl_compositor_create_surface failed..." << std::endl;
        return -1;
    }
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "7171c4bd-43fa-4013-8570-f4649586b618",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_MAKE_VERSION(1, 0, 0),
    };
    char const* extensions[] = {
        "VK_KHR_wayland_surface",
    };
    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = std::size(extensions),
        .ppEnabledExtensionNames = extensions,
    };
    VkInstance instance = nullptr;
    if (VK_SUCCESS != vkCreateInstance(&createInfo, nullptr, &instance)) {
        std::cerr << "vkCreateInstance failed..." << std::endl;
        return -1;
    }
    uint32_t nof_devices = 0;
    if (VK_SUCCESS != vkEnumeratePhysicalDevices(instance, &nof_devices, nullptr)) {
        std::cerr << "vkEnumeratePhysicalDevices failed..." << std::endl;
        return -1;
    }
    std::vector<VkPhysicalDevice> devices(nof_devices);
    if (VK_SUCCESS != vkEnumeratePhysicalDevices(instance, &nof_devices, devices.data())) {
        std::cerr << "vkEnumeratePhysicalDevices failed..." << std::endl;
        return -1;
    }
    for(auto device : devices) {
        VkPhysicalDeviceProperties prop;
        vkGetPhysicalDeviceProperties(device, &prop);
        std::cout << prop.apiVersion << std::endl;
        std::cout << prop.driverVersion << std::endl;
        std::cout << prop.vendorID << std::endl;
        std::cout << prop.deviceID << std::endl;
        std::cout << prop.deviceType << std::endl;
        std::cout << prop.deviceName << std::endl;
        std::cout << prop.pipelineCacheUUID << std::endl;
        //std::cout << prop.limits << std::endl;
        std::cout << "-------------------------------------------------------" << std::endl;
    }

    VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo = {
        VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        nullptr,
        0,
        display,
        surface,
    };
    VkSurfaceKHR* vkSurface = nullptr;
    if (VK_SUCCESS != vkCreateWaylandSurfaceKHR(instance, &surfaceCreateInfo, nullptr, vkSurface)) {
        std::cerr << "vkCreateWaylandSurfaceKHR failed..." << std::endl;
        return -1;
    }
    wl_surface_destroy(surface);
    wl_compositor_destroy(compositor);
    wl_registry_destroy(registry);
    wl_display_disconnect(display);
    return 0;
}
