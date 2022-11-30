
#include <concepts>
#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>
#include <tuple>
#include <utility>
#include <initializer_list>

#include <wayland-client.h>
#include "xdg-shell-v6-client.h"
#include "zwp-tablet-v2-client.h"

#define VK_USE_PLATFORM_WAYLAND_KHR
#include <vulkan/vulkan.h>

inline namespace ext
{
    template <class Ch, class Tuple, size_t... I>
    void print(std::basic_ostream<Ch>& output, Tuple const& t, std::index_sequence<I...>) noexcept {
        (void) std::initializer_list<int>{ (void(output << (I==0 ? "" : " ") << std::get<I>(t)), 0)... };
    }
    template <class Ch, class... Args>
    auto& operator<<(std::basic_ostream<Ch>& output, std::tuple<Args...> const& t) noexcept {
        output.put('(');
        print(output, t, std::make_index_sequence<sizeof... (Args)>());
        output.put(')');
        return output;
    }
    template <class Ch, class K, class V>
    auto& operator<<(std::basic_ostream<Ch>& output, std::pair<K, V> const& p) noexcept {
        return output << '(' << p.first << ' ' << p.second << ')';
    }

} // ::ext


template <class> constexpr std::nullptr_t wl_interface_ptr = nullptr;
#define INTERN_WL_INTERFACE(wl_client)                                  \
    template <> constexpr wl_interface const *const wl_interface_ptr<wl_client> = &wl_client##_interface;

template <class T, class D> auto safe_ptr(T* ptr, D del) {
    if (nullptr == ptr) throw std::runtime_error("bad pointer...");
    return std::unique_ptr<T, D>(ptr, del);
}
template <class T, void (*D)(T*)> auto safe_ptr() {
    return std::unique_ptr<T, decltype (D)>(nullptr, D);
}
#define INTERN_WL_SAFE_PTR(wl_client)                   \
    inline auto safe_ptr(wl_client* ptr) {              \
        return safe_ptr(ptr, wl_client##_destroy);      \
    }

INTERN_WL_INTERFACE(wl_display)
inline auto safe_ptr(wl_display* ptr) { return safe_ptr(ptr, wl_display_disconnect); }

#define INTERN_WL_2(wl_client)                  \
    INTERN_WL_INTERFACE(wl_client)              \
    INTERN_WL_SAFE_PTR(wl_client)

INTERN_WL_2(wl_registry)
INTERN_WL_2(wl_compositor)
INTERN_WL_2(wl_surface)
INTERN_WL_2(zxdg_shell_v6);

inline namespace vulkan
{
    inline auto layers() {
        uint32_t count = 0;
        vkEnumerateInstanceLayerProperties(&count, nullptr);
        std::vector<VkLayerProperties> props(count);
        vkEnumerateInstanceLayerProperties(&count, props.data());
        return props;
    }
    inline auto layers(VkPhysicalDevice pdev) {
        uint32_t count = 0;
        vkEnumerateDeviceLayerProperties(pdev, &count, nullptr);
        std::vector<VkLayerProperties> props(count);
        vkEnumerateDeviceLayerProperties(pdev, &count, props.data());
        return props;
    }
    template <class Ch>
    inline auto& operator<<(std::basic_ostream<Ch>& output, VkLayerProperties const& prop) noexcept {
        auto const& [name, spec, impl, desc] = prop;
        return output << name << ',' << spec << ',' << impl << ',' << desc;
    }

    inline auto extensions(char const* layer_name = nullptr) {
        uint32_t count = 0;
        vkEnumerateInstanceExtensionProperties(layer_name, &count, nullptr);
        std::vector<VkExtensionProperties> props(count);
        vkEnumerateInstanceExtensionProperties(layer_name, &count, props.data());
        return props;
    }
    inline auto extensions(VkPhysicalDevice pdev, char const* layer_name = nullptr) {
        uint32_t count = 0;
        vkEnumerateDeviceExtensionProperties(pdev, layer_name, &count, nullptr);
        std::vector<VkExtensionProperties> props(count);
        vkEnumerateInstanceExtensionProperties(layer_name, &count, props.data());
        return props;
    }
    template <class Ch>
    inline auto& operator<<(std::basic_ostream<Ch>& output, VkExtensionProperties const& prop) noexcept {
        auto const [name, spec] = prop;
        return output << name << ',' << spec;
    }

    inline auto physical_devices(VkInstance instance) {
        uint32_t count = 0;
        vkEnumeratePhysicalDevices(instance, &count, nullptr);
        std::vector<VkPhysicalDevice> devices(count);
        vkEnumeratePhysicalDevices(instance, &count, devices.data());
        return devices;
    }
    inline auto properties(VkPhysicalDevice pdev) {
        VkPhysicalDeviceProperties prop { };
        vkGetPhysicalDeviceProperties(pdev, &prop);
        return prop;
    }
    inline auto capabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) noexcept {
        VkSurfaceCapabilitiesKHR caps = { };
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &caps);
        return caps;
    }
    template <class Ch>
    inline auto& operator<<(std::basic_ostream<Ch>& output, VkExtent2D const& extent) noexcept {
        return output << '(' << extent.width << ' ' << extent.height << ')';
    }
    template <class Ch>
    inline auto& operator<<(std::basic_ostream<Ch>& output, VkSurfaceCapabilitiesKHR const& caps) noexcept {
        output << "(capabilities" << std::endl;
#define PRINT_SIG_VAL_PAIR(x) (output << " (" #x " " << caps.x << ")" << std::endl)
        PRINT_SIG_VAL_PAIR(minImageCount);
        PRINT_SIG_VAL_PAIR(maxImageCount);
        PRINT_SIG_VAL_PAIR(currentExtent);
        PRINT_SIG_VAL_PAIR(minImageExtent);
        PRINT_SIG_VAL_PAIR(maxImageExtent);
        PRINT_SIG_VAL_PAIR(maxImageArrayLayers);
        PRINT_SIG_VAL_PAIR(supportedTransforms);
        PRINT_SIG_VAL_PAIR(currentTransform);
        PRINT_SIG_VAL_PAIR(supportedCompositeAlpha);
        PRINT_SIG_VAL_PAIR(supportedUsageFlags);
#undef PRINT_SIG_VAL_PAIR
        return output << ")";
    }
    template <class Ch, class T, size_t N> requires (sizeof (T) > 1)
    inline auto& operator<<(std::basic_ostream<Ch>& output, T (&arr)[N]) noexcept {
        output << "#(";
        for (size_t i = 0; i < std::size(arr); ++i) {
            if (i != 0)
                output.put(' ');
            output << arr[i];
        }
        return output << ")";
    }
    template <class Ch>
    inline auto& operator<<(std::basic_ostream<Ch>& output, VkPhysicalDeviceLimits const& limits) noexcept {
        output << "\n'(physical-device-limits" << std::endl;
#define PRINT_SIG_VAL_PAIR(x) (output << " (" #x " " << limits.x << ")" << std::endl)
        PRINT_SIG_VAL_PAIR(maxImageDimension1D);
        PRINT_SIG_VAL_PAIR(maxImageDimension2D);
        PRINT_SIG_VAL_PAIR(maxImageDimension3D);
        PRINT_SIG_VAL_PAIR(maxImageDimensionCube);
        PRINT_SIG_VAL_PAIR(maxImageArrayLayers);
        PRINT_SIG_VAL_PAIR(maxTexelBufferElements);
        PRINT_SIG_VAL_PAIR(maxUniformBufferRange);
        PRINT_SIG_VAL_PAIR(maxStorageBufferRange);
        PRINT_SIG_VAL_PAIR(maxPushConstantsSize);
        PRINT_SIG_VAL_PAIR(maxMemoryAllocationCount);
        PRINT_SIG_VAL_PAIR(maxSamplerAllocationCount);
        PRINT_SIG_VAL_PAIR(bufferImageGranularity);
        PRINT_SIG_VAL_PAIR(sparseAddressSpaceSize);
        PRINT_SIG_VAL_PAIR(maxBoundDescriptorSets);
        PRINT_SIG_VAL_PAIR(maxPerStageDescriptorSamplers);
        PRINT_SIG_VAL_PAIR(maxPerStageDescriptorUniformBuffers);
        PRINT_SIG_VAL_PAIR(maxPerStageDescriptorStorageBuffers);
        PRINT_SIG_VAL_PAIR(maxPerStageDescriptorSampledImages);
        PRINT_SIG_VAL_PAIR(maxPerStageDescriptorStorageImages);
        PRINT_SIG_VAL_PAIR(maxPerStageDescriptorInputAttachments);
        PRINT_SIG_VAL_PAIR(maxPerStageResources);
        PRINT_SIG_VAL_PAIR(maxDescriptorSetSamplers);
        PRINT_SIG_VAL_PAIR(maxDescriptorSetUniformBuffers);
        PRINT_SIG_VAL_PAIR(maxDescriptorSetUniformBuffersDynamic);
        PRINT_SIG_VAL_PAIR(maxDescriptorSetStorageBuffers);
        PRINT_SIG_VAL_PAIR(maxDescriptorSetStorageBuffersDynamic);
        PRINT_SIG_VAL_PAIR(maxDescriptorSetSampledImages);
        PRINT_SIG_VAL_PAIR(maxDescriptorSetStorageImages);
        PRINT_SIG_VAL_PAIR(maxDescriptorSetInputAttachments);
        PRINT_SIG_VAL_PAIR(maxVertexInputAttributes);
        PRINT_SIG_VAL_PAIR(maxVertexInputBindings);
        PRINT_SIG_VAL_PAIR(maxVertexInputAttributeOffset);
        PRINT_SIG_VAL_PAIR(maxVertexInputBindingStride);
        PRINT_SIG_VAL_PAIR(maxVertexOutputComponents);
        PRINT_SIG_VAL_PAIR(maxTessellationGenerationLevel);
        PRINT_SIG_VAL_PAIR(maxTessellationPatchSize);
        PRINT_SIG_VAL_PAIR(maxTessellationControlPerVertexInputComponents);
        PRINT_SIG_VAL_PAIR(maxTessellationControlPerVertexOutputComponents);
        PRINT_SIG_VAL_PAIR(maxTessellationControlPerPatchOutputComponents);
        PRINT_SIG_VAL_PAIR(maxTessellationControlTotalOutputComponents);
        PRINT_SIG_VAL_PAIR(maxTessellationEvaluationInputComponents);
        PRINT_SIG_VAL_PAIR(maxTessellationEvaluationOutputComponents);
        PRINT_SIG_VAL_PAIR(maxGeometryShaderInvocations);
        PRINT_SIG_VAL_PAIR(maxGeometryInputComponents);
        PRINT_SIG_VAL_PAIR(maxGeometryOutputComponents);
        PRINT_SIG_VAL_PAIR(maxGeometryOutputVertices);
        PRINT_SIG_VAL_PAIR(maxGeometryTotalOutputComponents);
        PRINT_SIG_VAL_PAIR(maxFragmentInputComponents);
        PRINT_SIG_VAL_PAIR(maxFragmentOutputAttachments);
        PRINT_SIG_VAL_PAIR(maxFragmentDualSrcAttachments);
        PRINT_SIG_VAL_PAIR(maxFragmentCombinedOutputResources);
        PRINT_SIG_VAL_PAIR(maxComputeSharedMemorySize);
        PRINT_SIG_VAL_PAIR(maxComputeWorkGroupCount);
        PRINT_SIG_VAL_PAIR(maxComputeWorkGroupInvocations);
        PRINT_SIG_VAL_PAIR(maxComputeWorkGroupSize);
        PRINT_SIG_VAL_PAIR(subPixelPrecisionBits);
        PRINT_SIG_VAL_PAIR(subTexelPrecisionBits);
        PRINT_SIG_VAL_PAIR(mipmapPrecisionBits);
        PRINT_SIG_VAL_PAIR(maxDrawIndexedIndexValue);
        PRINT_SIG_VAL_PAIR(maxDrawIndirectCount);
        PRINT_SIG_VAL_PAIR(maxSamplerLodBias);
        PRINT_SIG_VAL_PAIR(maxSamplerAnisotropy);
        PRINT_SIG_VAL_PAIR(maxViewports);
        PRINT_SIG_VAL_PAIR(maxViewportDimensions);
        PRINT_SIG_VAL_PAIR(viewportBoundsRange);
        PRINT_SIG_VAL_PAIR(viewportSubPixelBits);
        PRINT_SIG_VAL_PAIR(minMemoryMapAlignment);
        PRINT_SIG_VAL_PAIR(minTexelBufferOffsetAlignment);
        PRINT_SIG_VAL_PAIR(minUniformBufferOffsetAlignment);
        PRINT_SIG_VAL_PAIR(minStorageBufferOffsetAlignment);
        PRINT_SIG_VAL_PAIR(minTexelOffset);
        PRINT_SIG_VAL_PAIR(maxTexelOffset);
        PRINT_SIG_VAL_PAIR(minTexelGatherOffset);
        PRINT_SIG_VAL_PAIR(maxTexelGatherOffset);
        PRINT_SIG_VAL_PAIR(minInterpolationOffset);
        PRINT_SIG_VAL_PAIR(maxInterpolationOffset);
        PRINT_SIG_VAL_PAIR(subPixelInterpolationOffsetBits);
        PRINT_SIG_VAL_PAIR(maxFramebufferWidth);
        PRINT_SIG_VAL_PAIR(maxFramebufferHeight);
        PRINT_SIG_VAL_PAIR(maxFramebufferLayers);
        PRINT_SIG_VAL_PAIR(framebufferColorSampleCounts);
        PRINT_SIG_VAL_PAIR(framebufferDepthSampleCounts);
        PRINT_SIG_VAL_PAIR(framebufferStencilSampleCounts);
        PRINT_SIG_VAL_PAIR(framebufferNoAttachmentsSampleCounts);
        PRINT_SIG_VAL_PAIR(maxColorAttachments);
        PRINT_SIG_VAL_PAIR(sampledImageColorSampleCounts);
        PRINT_SIG_VAL_PAIR(sampledImageIntegerSampleCounts);
        PRINT_SIG_VAL_PAIR(sampledImageDepthSampleCounts);
        PRINT_SIG_VAL_PAIR(sampledImageStencilSampleCounts);
        PRINT_SIG_VAL_PAIR(storageImageSampleCounts);
        PRINT_SIG_VAL_PAIR(maxSampleMaskWords);
        PRINT_SIG_VAL_PAIR(timestampComputeAndGraphics);
        PRINT_SIG_VAL_PAIR(timestampPeriod);
        PRINT_SIG_VAL_PAIR(maxClipDistances);
        PRINT_SIG_VAL_PAIR(maxCullDistances);
        PRINT_SIG_VAL_PAIR(maxCombinedClipAndCullDistances);
        PRINT_SIG_VAL_PAIR(discreteQueuePriorities);
        PRINT_SIG_VAL_PAIR(pointSizeRange);
        PRINT_SIG_VAL_PAIR(lineWidthRange);
        PRINT_SIG_VAL_PAIR(pointSizeGranularity);
        PRINT_SIG_VAL_PAIR(lineWidthGranularity);
        PRINT_SIG_VAL_PAIR(strictLines);
        PRINT_SIG_VAL_PAIR(standardSampleLocations);
        PRINT_SIG_VAL_PAIR(optimalBufferCopyOffsetAlignment);
        PRINT_SIG_VAL_PAIR(optimalBufferCopyRowPitchAlignment);
        PRINT_SIG_VAL_PAIR(nonCoherentAtomSize);
        return output << ")";
    }
    template <class Ch>
    auto& operator<<(std::basic_ostream<Ch>& output, VkPhysicalDeviceProperties const& prop) noexcept {
        output << prop.apiVersion << ',';
        output << prop.driverVersion << ',';
        output << prop.vendorID << ',';
        output << prop.deviceID << ',';
        output << prop.deviceType << ',';
        output << prop.deviceName << ',';
        for(auto item : prop.pipelineCacheUUID) {
            output << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(item);
        }
        output << ',';
        output << std::dec << std::setfill(' ');
        output << prop.limits << ',';
        //output << prop.sparseProperties...
        return output;
    }
} // ::vulkan

int main() {
    // std::cout << "instance layers:" << std::endl;
    // for (auto const& layer : layers()) {
    //     std::cout << layer << std::endl;
    //     for (auto const& extension : extensions(layer.layerName)) {
    //         std::cout << '\t' << extension << std::endl;
    //     }
    // }
    // std::cout << "instance extensions:" << std::endl;
    // auto instance_extensions = extensions();
    // for (auto const& extension : extensions()) {
    //     std::cout << extension << std::endl;
    // }

    try {
        auto display = safe_ptr(wl_display_connect(nullptr));
        auto registry = safe_ptr(wl_display_get_registry(display.get()));

        static wl_compositor* compositor_raw = nullptr;
        static zxdg_shell_v6* shell_raw = nullptr;
        wl_registry_listener listener = {
            .global = [](auto, auto registry, auto name, auto interface, auto version) noexcept {
                if (std::string_view(interface) == wl_compositor_interface.name) {
                    compositor_raw = (wl_compositor*) wl_registry_bind(registry,
                                                                       name,
                                                                       &wl_compositor_interface,
                                                                       version);
                }
                else if (std::string_view(interface) == zxdg_shell_v6_interface.name) {
                    shell_raw = (zxdg_shell_v6*) wl_registry_bind(registry,
                                                                  name,
                                                                  &zxdg_shell_v6_interface,
                                                                  version);
                }
            },
            .global_remove = [](auto...) noexcept { },
        };
        wl_registry_add_listener(registry.get(), &listener, nullptr);
        wl_display_roundtrip(display.get());
        auto compositor = safe_ptr(compositor_raw);
        auto shell = safe_ptr(shell_raw);
        auto surface = safe_ptr(wl_compositor_create_surface(compositor.get()));

        auto create_instance = [] {
            VkApplicationInfo appInfo = {
                .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pApplicationName = "7171c4bd-43fa-4013-8570-f4649586b618",
                .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                .pEngineName = "No Engine",
                .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                .apiVersion = VK_MAKE_VERSION(1, 0, 0),
            };
            char const* layers[] = {
                "VK_LAYER_LUNARG_api_dump",
            };
            char const* extensions[] = {
                "VK_KHR_wayland_surface",
            };
            VkInstanceCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .pApplicationInfo = &appInfo,
                .enabledLayerCount = std::size(layers),
                .ppEnabledLayerNames = layers,
                .enabledExtensionCount = std::size(extensions),
                .ppEnabledExtensionNames = extensions,
            };
            VkInstance instance_raw = nullptr;
            auto ret = vkCreateInstance(&createInfo, nullptr, &instance_raw);
            if (ret != VK_SUCCESS) {
                std::cerr << "vkCreateInstance failed: " << ret << std::endl;
            }
            return instance_raw;
        };
        auto instance = safe_ptr(create_instance(), [](auto ptr) { vkDestroyInstance(ptr, nullptr); });
        for (auto pdev : physical_devices(instance.get())) {
            std::cout << properties(pdev) << std::endl;
            for (auto const& layer : vulkan::layers(pdev)) {
                std::cout << layer << std::endl;
            }
        }

        auto create_device = [&] {
            VkPhysicalDeviceFeatures supportedFeatures;
            vkGetPhysicalDeviceFeatures(physical_devices(instance.get()).front(), &supportedFeatures);
            VkPhysicalDeviceFeatures requiredFeatures = {
                .geometryShader = VK_TRUE,
                .tessellationShader = VK_TRUE,
                .multiDrawIndirect = supportedFeatures.multiDrawIndirect,
            };
            VkDeviceQueueCreateInfo deviceQueueCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = 0,
                .queueCount = 1,
                .pQueuePriorities = nullptr,
            };
            VkDeviceCreateInfo deviceCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueCreateInfoCount = 1,
                .pQueueCreateInfos = &deviceQueueCreateInfo,
                .enabledLayerCount = 0,
                .ppEnabledLayerNames = nullptr,
                .enabledExtensionCount = 0,
                .ppEnabledExtensionNames = nullptr,
                .pEnabledFeatures = &requiredFeatures,
            };
            VkDevice device_raw = nullptr;
            auto ret = vkCreateDevice(physical_devices(instance.get()).front(),
                                      &deviceCreateInfo,
                                      nullptr,
                                      &device_raw);
            if (ret != VK_SUCCESS) {
                std::cerr << "vkCreateDevice failed: " << ret << std::endl;
            }
            return device_raw;
        };
        auto device = safe_ptr(create_device(),
                               [](auto ptr) noexcept {
                                   vkDestroyDevice(ptr, nullptr);
                               });
        VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .display = display.get(),
            .surface = surface.get(),
        };

        auto create_surface = [&] {
            VkSurfaceKHR vk_surface = nullptr;
            if (VK_SUCCESS != vkCreateWaylandSurfaceKHR(instance.get(), &surfaceCreateInfo, nullptr, &vk_surface)) {
                std::cerr << "vkCreateWaylandSurfaceKHR failed..." << std::endl;
            }
            return vk_surface;
        };
        auto vk_surface = safe_ptr(create_surface(),
                                   [&](auto ptr) noexcept {
                                       vkDestroySurfaceKHR(instance.get(), ptr, nullptr);
                                   });

        //std::cout << capabilities(physical_devices(instance.get()).front(), vk_surface.get()) << std::endl;

        auto create_swapchain = [&] {
            VkSwapchainKHR swapchain = nullptr;
            VkSwapchainCreateInfoKHR info = {
                .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .pNext = nullptr,
                .flags = 0,
                .surface = vk_surface.get(),
                .minImageCount = 2,
                .imageFormat = VK_FORMAT_R8G8B8A8_UNORM,
                .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
                .imageExtent = { 1024, 768 },
                .imageArrayLayers = 1,
                .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .queueFamilyIndexCount = 0,
                .pQueueFamilyIndices = nullptr,
                .preTransform = VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR,
                .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                .presentMode = VK_PRESENT_MODE_FIFO_KHR,
                .clipped = VK_TRUE,
                //.oldSwapchain = swapchain,
            };
            if (auto ret = vkCreateSwapchainKHR(device.get(), &info, nullptr, &swapchain); ret != VK_SUCCESS) {
                std::cerr << "vkCreateSwapchainKHR failed..." << std::endl;
            }
            std::cout << "swapchain: " << swapchain << std::endl;
            return swapchain;
        };
        auto swapchain = safe_ptr(create_swapchain(),
                                  [&](auto ptr) noexcept {
                                      vkDestroySwapchainKHR(device.get(), ptr, nullptr);
                                  });
        // wait to clean up
        while (vkDeviceWaitIdle(device.get()) != VK_SUCCESS) continue;
        return 0;
    }
    catch (std::exception& ex) {
        std::cerr << "exception occurred: " << ex.what() << std::endl;
    }
    return -1;
}
