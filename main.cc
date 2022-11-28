
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
        output << "\n'(" << std::endl;
#define SIG_VAL_PAIR(x) (output << " (" #x " " << limits.x << ")" << std::endl)
        SIG_VAL_PAIR(maxImageDimension1D);
        SIG_VAL_PAIR(maxImageDimension2D);
        SIG_VAL_PAIR(maxImageDimension3D);
        SIG_VAL_PAIR(maxImageDimensionCube);
        SIG_VAL_PAIR(maxImageArrayLayers);
        SIG_VAL_PAIR(maxTexelBufferElements);
        SIG_VAL_PAIR(maxUniformBufferRange);
        SIG_VAL_PAIR(maxStorageBufferRange);
        SIG_VAL_PAIR(maxPushConstantsSize);
        SIG_VAL_PAIR(maxMemoryAllocationCount);
        SIG_VAL_PAIR(maxSamplerAllocationCount);
        SIG_VAL_PAIR(bufferImageGranularity);
        SIG_VAL_PAIR(sparseAddressSpaceSize);
        SIG_VAL_PAIR(maxBoundDescriptorSets);
        SIG_VAL_PAIR(maxPerStageDescriptorSamplers);
        SIG_VAL_PAIR(maxPerStageDescriptorUniformBuffers);
        SIG_VAL_PAIR(maxPerStageDescriptorStorageBuffers);
        SIG_VAL_PAIR(maxPerStageDescriptorSampledImages);
        SIG_VAL_PAIR(maxPerStageDescriptorStorageImages);
        SIG_VAL_PAIR(maxPerStageDescriptorInputAttachments);
        SIG_VAL_PAIR(maxPerStageResources);
        SIG_VAL_PAIR(maxDescriptorSetSamplers);
        SIG_VAL_PAIR(maxDescriptorSetUniformBuffers);
        SIG_VAL_PAIR(maxDescriptorSetUniformBuffersDynamic);
        SIG_VAL_PAIR(maxDescriptorSetStorageBuffers);
        SIG_VAL_PAIR(maxDescriptorSetStorageBuffersDynamic);
        SIG_VAL_PAIR(maxDescriptorSetSampledImages);
        SIG_VAL_PAIR(maxDescriptorSetStorageImages);
        SIG_VAL_PAIR(maxDescriptorSetInputAttachments);
        SIG_VAL_PAIR(maxVertexInputAttributes);
        SIG_VAL_PAIR(maxVertexInputBindings);
        SIG_VAL_PAIR(maxVertexInputAttributeOffset);
        SIG_VAL_PAIR(maxVertexInputBindingStride);
        SIG_VAL_PAIR(maxVertexOutputComponents);
        SIG_VAL_PAIR(maxTessellationGenerationLevel);
        SIG_VAL_PAIR(maxTessellationPatchSize);
        SIG_VAL_PAIR(maxTessellationControlPerVertexInputComponents);
        SIG_VAL_PAIR(maxTessellationControlPerVertexOutputComponents);
        SIG_VAL_PAIR(maxTessellationControlPerPatchOutputComponents);
        SIG_VAL_PAIR(maxTessellationControlTotalOutputComponents);
        SIG_VAL_PAIR(maxTessellationEvaluationInputComponents);
        SIG_VAL_PAIR(maxTessellationEvaluationOutputComponents);
        SIG_VAL_PAIR(maxGeometryShaderInvocations);
        SIG_VAL_PAIR(maxGeometryInputComponents);
        SIG_VAL_PAIR(maxGeometryOutputComponents);
        SIG_VAL_PAIR(maxGeometryOutputVertices);
        SIG_VAL_PAIR(maxGeometryTotalOutputComponents);
        SIG_VAL_PAIR(maxFragmentInputComponents);
        SIG_VAL_PAIR(maxFragmentOutputAttachments);
        SIG_VAL_PAIR(maxFragmentDualSrcAttachments);
        SIG_VAL_PAIR(maxFragmentCombinedOutputResources);
        SIG_VAL_PAIR(maxComputeSharedMemorySize);
        SIG_VAL_PAIR(maxComputeWorkGroupCount);
        SIG_VAL_PAIR(maxComputeWorkGroupInvocations);
        SIG_VAL_PAIR(maxComputeWorkGroupSize);
        SIG_VAL_PAIR(subPixelPrecisionBits);
        SIG_VAL_PAIR(subTexelPrecisionBits);
        SIG_VAL_PAIR(mipmapPrecisionBits);
        SIG_VAL_PAIR(maxDrawIndexedIndexValue);
        SIG_VAL_PAIR(maxDrawIndirectCount);
        SIG_VAL_PAIR(maxSamplerLodBias);
        SIG_VAL_PAIR(maxSamplerAnisotropy);
        SIG_VAL_PAIR(maxViewports);
        SIG_VAL_PAIR(maxViewportDimensions);
        SIG_VAL_PAIR(viewportBoundsRange);
        SIG_VAL_PAIR(viewportSubPixelBits);
        SIG_VAL_PAIR(minMemoryMapAlignment);
        SIG_VAL_PAIR(minTexelBufferOffsetAlignment);
        SIG_VAL_PAIR(minUniformBufferOffsetAlignment);
        SIG_VAL_PAIR(minStorageBufferOffsetAlignment);
        SIG_VAL_PAIR(minTexelOffset);
        SIG_VAL_PAIR(maxTexelOffset);
        SIG_VAL_PAIR(minTexelGatherOffset);
        SIG_VAL_PAIR(maxTexelGatherOffset);
        SIG_VAL_PAIR(minInterpolationOffset);
        SIG_VAL_PAIR(maxInterpolationOffset);
        SIG_VAL_PAIR(subPixelInterpolationOffsetBits);
        SIG_VAL_PAIR(maxFramebufferWidth);
        SIG_VAL_PAIR(maxFramebufferHeight);
        SIG_VAL_PAIR(maxFramebufferLayers);
        SIG_VAL_PAIR(framebufferColorSampleCounts);
        SIG_VAL_PAIR(framebufferDepthSampleCounts);
        SIG_VAL_PAIR(framebufferStencilSampleCounts);
        SIG_VAL_PAIR(framebufferNoAttachmentsSampleCounts);
        SIG_VAL_PAIR(maxColorAttachments);
        SIG_VAL_PAIR(sampledImageColorSampleCounts);
        SIG_VAL_PAIR(sampledImageIntegerSampleCounts);
        SIG_VAL_PAIR(sampledImageDepthSampleCounts);
        SIG_VAL_PAIR(sampledImageStencilSampleCounts);
        SIG_VAL_PAIR(storageImageSampleCounts);
        SIG_VAL_PAIR(maxSampleMaskWords);
        SIG_VAL_PAIR(timestampComputeAndGraphics);
        SIG_VAL_PAIR(timestampPeriod);
        SIG_VAL_PAIR(maxClipDistances);
        SIG_VAL_PAIR(maxCullDistances);
        SIG_VAL_PAIR(maxCombinedClipAndCullDistances);
        SIG_VAL_PAIR(discreteQueuePriorities);
        SIG_VAL_PAIR(pointSizeRange);
        SIG_VAL_PAIR(lineWidthRange);
        SIG_VAL_PAIR(pointSizeGranularity);
        SIG_VAL_PAIR(lineWidthGranularity);
        SIG_VAL_PAIR(strictLines);
        SIG_VAL_PAIR(standardSampleLocations);
        SIG_VAL_PAIR(optimalBufferCopyOffsetAlignment);
        SIG_VAL_PAIR(optimalBufferCopyRowPitchAlignment);
        SIG_VAL_PAIR(nonCoherentAtomSize);
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
    std::cout << std::tuple(1,2,3) << std::endl;
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

    auto display = safe_ptr(wl_display_connect(nullptr));
    auto registry = safe_ptr(wl_display_get_registry(display.get()));
    wl_compositor* compositor_raw = nullptr;
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
    wl_registry_add_listener(registry.get(), &listener, &compositor_raw);
    wl_display_roundtrip(display.get());
    auto compositor = safe_ptr(compositor_raw);
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
            .enabledLayerCount = (std::size(layers), 0),
            .ppEnabledLayerNames = (layers, nullptr),
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
    VkDevice logicalDevice = nullptr;
    auto ret = vkCreateDevice(physical_devices(instance.get()).front(),
                              &deviceCreateInfo,
                              nullptr,
                              &logicalDevice);
    if (ret != VK_SUCCESS) {
        std::cerr << "vkCreateDevice failed: " << ret << std::endl;
        return -1;
    }
    VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .display = display.get(),
        .surface = surface.get(),
    };
    VkSurfaceKHR vkSurface = nullptr;
    if (VK_SUCCESS != vkCreateWaylandSurfaceKHR(instance.get(), &surfaceCreateInfo, nullptr, &vkSurface)) {
        std::cerr << "vkCreateWaylandSurfaceKHR failed..." << std::endl;
        return -1;
    }

    for (;;) {
        if (vkDeviceWaitIdle(logicalDevice) == VK_SUCCESS) {
            vkDestroySurfaceKHR(instance.get(), vkSurface, nullptr);
            vkDestroyDevice(logicalDevice, nullptr);
            break;
        }
    }
    return 0;
}
