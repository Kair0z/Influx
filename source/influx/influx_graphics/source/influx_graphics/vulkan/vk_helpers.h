#pragma once
#include "vk_headers.h"

#include <vector>
#include <string>
#include <iostream>
#include <functional>

namespace influx::graphics::vk_helpers
{
    // debug callback!
    using debug_message_callback = std::function<void(
        VkDebugUtilsMessageSeverityFlagBitsEXT, // severity
        VkDebugUtilsMessageTypeFlagsEXT, // message type
        const VkDebugUtilsMessengerCallbackDataEXT&)>;

    inline std::list<debug_message_callback> g_debug_message_callbacks = {};

    inline VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT       messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT              messageTypes,
        VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
        void* /*pUserData*/ )
    {
#if INFLUX_DEBUG
        if (static_cast<uint32_t>(pCallbackData->messageIdNumber) == 0x822806fa)
        {
            // Validation Warning: vkCreateInstance(): to enable extension VK_EXT_debug_utils, but this extension is intended to support use by applications when
            // debugging and it is strongly recommended that it be otherwise avoided.
            return vk::False;
        }
        else if (static_cast<uint32_t>(pCallbackData->messageIdNumber) == 0xe8d1a9fe)
        {
            // Validation Performance Warning: Using debug builds of the validation layers *will* adversely affect performance.
            return vk::False;
        }
#endif

        // user callbacks
        for (const debug_message_callback& clb : g_debug_message_callbacks)
        {
            clb(messageSeverity, messageTypes, *pCallbackData);
        }

        return vk::False;
    }

    // instance_create_info_chain
#if INFLUX_DEBUG
    using instance_create_info_chain = vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT>;
#else
    using instance_create_info_chain = vk::StructureChain<vk::InstanceCreateInfo>;
#endif
    inline instance_create_info_chain make_instance_create_info_chain(
            vk::ApplicationInfo const& applicationInfo,
            std::vector<char const*> const& layers,
            std::vector<char const*> const& extensions)
    {
#if INFLUX_DEBUG
        // in debug mode, addionally use the debugUtilsMessengerCallback in instance creation!
        vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

        vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

        vk::InstanceCreateInfo instance_create_info{ {}, &applicationInfo, layers, extensions };
        vk::DebugUtilsMessengerCreateInfoEXT debug_messenger_create_info = { {}, severityFlags, messageTypeFlags, &debugUtilsMessengerCallback };
        return instance_create_info_chain{ instance_create_info, debug_messenger_create_info };
#else
        return instance_create_info_chain{ { {}, &applicationInfo, layers, extensions } };
#endif
    }

    // gather layers
    inline std::vector<char const*> gatherLayers(
        std::vector<std::string> const& layers,
        std::vector<vk::LayerProperties> const& layerProperties)
    {
        std::vector<char const*> enabledLayers;
        enabledLayers.reserve(layers.size());
        for (auto const& layer : layers)
        {
            assert(std::any_of(layerProperties.begin(), layerProperties.end(), [layer](vk::LayerProperties const& lp) { return layer == lp.layerName; }));
            enabledLayers.push_back(layer.data());
        }

#if INFLUX_DEBUG
        // Enable standard validation layer to find as much errors as possible!
        if (std::none_of(layers.begin(), layers.end(), [](std::string const& layer) { return layer == "VK_LAYER_KHRONOS_validation"; }) &&
            std::any_of(layerProperties.begin(),
                layerProperties.end(),
                [](vk::LayerProperties const& lp) { return (strcmp("VK_LAYER_KHRONOS_validation", lp.layerName) == 0); }))
        {
            enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
        }
#endif
        return enabledLayers;
    }

    // gather extensions
    inline std::vector<char const*> gatherExtensions(std::vector<std::string> const& extensions,
        std::vector<vk::ExtensionProperties> const& extensionProperties)
    {
        std::vector<char const*> enabledExtensions;
        enabledExtensions.reserve(extensions.size());
        for (auto const& ext : extensions)
        {
            assert(std::any_of(
                extensionProperties.begin(), extensionProperties.end(), [ext](vk::ExtensionProperties const& ep) { return ext == ep.extensionName; }));
            enabledExtensions.push_back(ext.data());
        }
#if INFLUX_DEBUG
        if (std::none_of(
            extensions.begin(), extensions.end(), [](std::string const& extension) { return extension == VK_EXT_DEBUG_UTILS_EXTENSION_NAME; }) &&
            std::any_of(extensionProperties.begin(),
                extensionProperties.end(),
                [](vk::ExtensionProperties const& ep) { return (strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, ep.extensionName) == 0); }))
        {
            enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
#endif
        return enabledExtensions;
    }

    // gather device extensions
    inline std::vector<std::string> getDeviceExtensions()
    {
        return { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    }

    inline vk::Instance createInstance(std::string const& appName,
        std::string const& engineName,
        std::vector<std::string> const& layers,
        std::vector<std::string> const& extensions,
        uint32_t                         apiVersion,
        debug_message_callback debug_callback)
    {
#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
        VULKAN_HPP_DEFAULT_DISPATCHER.init();
#endif

        vk::ApplicationInfo       applicationInfo(appName.c_str(), 1, engineName.c_str(), 1, apiVersion);
        std::vector<char const*> enabledLayers = gatherLayers(layers, vk::enumerateInstanceLayerProperties());
        std::vector<char const*> enabledExtensions = gatherExtensions(extensions, vk::enumerateInstanceExtensionProperties());

        // add global callback to list
        g_debug_message_callbacks.push_back(debug_callback);

        vk::Instance instance = vk::createInstance(
            make_instance_create_info_chain(applicationInfo, enabledLayers, enabledExtensions).get<vk::InstanceCreateInfo>());

#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
        // initialize function pointers for instance
        VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);
#endif

        return instance;
}

    inline uint32_t findGraphicsQueueFamilyIndex(std::vector<vk::QueueFamilyProperties> const& queueFamilyProperties)
    {
        // get the first index into queueFamiliyProperties which supports graphics
        std::vector<vk::QueueFamilyProperties>::const_iterator graphicsQueueFamilyProperty =
            std::find_if(queueFamilyProperties.begin(),
                queueFamilyProperties.end(),
                [](vk::QueueFamilyProperties const& qfp) { return qfp.queueFlags & vk::QueueFlagBits::eGraphics; });

        assert(graphicsQueueFamilyProperty != queueFamilyProperties.end());

        return static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));
    }

    inline std::pair<uint32_t, uint32_t> findGraphicsAndPresentQueueFamilyIndex(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR const& surface)
    {
        std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

        uint32_t graphicsQueueFamilyIndex = findGraphicsQueueFamilyIndex(queueFamilyProperties);
        if (physicalDevice.getSurfaceSupportKHR(graphicsQueueFamilyIndex, surface))
        {
            return std::make_pair(graphicsQueueFamilyIndex,
                graphicsQueueFamilyIndex);  // the first graphicsQueueFamilyIndex does also support presents
        }

        // the graphicsQueueFamilyIndex doesn't support present -> look for an other family index that supports both
        // graphics and present
        for (size_t i = 0; i < queueFamilyProperties.size(); i++)
        {
            if ((queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
                physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface))
            {
                return std::make_pair(static_cast<uint32_t>(i), static_cast<uint32_t>(i));
            }
        }

        // there's nothing like a single family index that supports both graphics and present -> look for an other family
        // index that supports present
        for (size_t i = 0; i < queueFamilyProperties.size(); i++)
        {
            if (physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface))
            {
                return std::make_pair(graphicsQueueFamilyIndex, static_cast<uint32_t>(i));
            }
        }

        throw std::runtime_error("Could not find queues for both graphics or present -> terminating");
    }

    inline std::vector<std::string> getInstanceLayers()
    {
        std::vector<std::string> layers{};
        return layers;
    }

    inline std::vector<std::string> getInstanceExtensions()
    {
        std::vector<std::string> extensions;
        extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined (_WIN32)
        extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

#if defined( VK_USE_PLATFORM_ANDROID_KHR )
        extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined( VK_USE_PLATFORM_IOS_MVK )
        extensions.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif defined( VK_USE_PLATFORM_MACOS_MVK )
        extensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif defined( VK_USE_PLATFORM_MIR_KHR )
        extensions.push_back(VK_KHR_MIR_SURFACE_EXTENSION_NAME);
#elif defined( VK_USE_PLATFORM_VI_NN )
        extensions.push_back(VK_NN_VI_SURFACE_EXTENSION_NAME);
#elif defined( VK_USE_PLATFORM_WAYLAND_KHR )
        extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined( VK_USE_PLATFORM_XCB_KHR )
        extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined( VK_USE_PLATFORM_XLIB_KHR )
        extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif defined( VK_USE_PLATFORM_XLIB_XRANDR_EXT )
        extensions.push_back(VK_EXT_ACQUIRE_XLIB_DISPLAY_EXTENSION_NAME);
#endif
        return extensions;
    }

    inline vk::Device createDevice(vk::PhysicalDevice const& physicalDevice,
        uint32_t                           queueFamilyIndex,
        std::vector<std::string> const& extensions = {},
        vk::PhysicalDeviceFeatures const* physicalDeviceFeatures = nullptr,
        void const* pNext = nullptr)
    {
        std::vector<char const*> enabledExtensions;
        enabledExtensions.reserve(extensions.size());
        for (auto const& ext : extensions)
        {
            enabledExtensions.push_back(ext.data());
        }

        float                     queuePriority = 0.0f;
        vk::DeviceQueueCreateInfo deviceQueueCreateInfo({}, queueFamilyIndex, 1, &queuePriority);
        vk::DeviceCreateInfo      deviceCreateInfo({}, deviceQueueCreateInfo, {}, enabledExtensions, physicalDeviceFeatures, pNext);

        vk::Device device = physicalDevice.createDevice(deviceCreateInfo);
#if ( VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1 )
        // initialize function pointers for instance
        VULKAN_HPP_DEFAULT_DISPATCHER.init(device);
#endif
        return device;
    }
}