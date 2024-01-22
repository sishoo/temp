#include "renderer.hpp"
#include "heph_utils.hpp"
#include "heph_defines.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <iostream>
#include <cstring>

Renderer::Renderer(const std::string name)
{   
    init_instance(name);
    init_pdevice();
    init_device();
}

void Renderer::init_instance(const std::string name)
{   
    VkApplicationInfo application_info = {};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = name.c_str();
    application_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
    application_info.pEngineName = "Hephaestus Engine";
    application_info.engineVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
    application_info.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);


    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &application_info;


    /* = Getting requested extensions = */
    uint32_t requested_extension_count = 0;
    std::vector<const char *> requested_extensions;

    uint32_t glfw_extension_count = 0;
    const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    // TODO GET THE USERS EXTENSION THING ASWELL

    requested_extensions.resize(glfw_extension_count);
    memcpy(requested_extensions.data(), glfw_extensions, glfw_extension_count * sizeof(char *));

    #ifdef HEPH_OSX
        requested_extension_count += 2;
        requested_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        requested_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    #endif


    /* = Getting available extensions = */
    uint32_t available_extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, nullptr);
    printf("THERE ARE %i extensions\n", available_extension_count);
    std::vector<VkExtensionProperties> available_extensions(available_extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, available_extensions.data());


    /* = Verifying extensions are available = */
    for (size_t i = 0; i < requested_extension_count; i++)
    {
        bool extension_available = false;
        for (VkExtensionProperties properties : available_extensions)
        {
            if (!strcmp(requested_extensions[i], properties.extensionName))
            {
                extension_available = true;
                break;
            }
        }
        if (!extension_available)
        {
            heph_throw_error_unrecoverable("A requested extension is not available.");
        }
    }
    create_info.enabledExtensionCount   =   requested_extension_count;
    create_info.ppEnabledExtensionNames = requested_extensions.data();


    /* = Getting required layers = */
    uint32_t enabled_layer_count = 0;
    std::vector<const char*> enabled_layers;
    #if HEPH_VALIDATE
        enabled_layer_count++;
        enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
    #endif
    #if HEPH_MONITOR
        enabled_layer_count++;
        enabled_layers.push_back("VK_LAYER_LUNARG_monitor");
    #endif
    create_info.enabledLayerCount   =   enabled_layer_count;
    create_info.ppEnabledLayerNames = enabled_layers.data();


    /* = Getting create flags = */
    uint32_t create_flags = 0;
    #ifdef HEPH_OSX
        create_flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    #endif
    create_info.flags = create_flags;


    VkResult result = vkCreateInstance(&create_info, nullptr, &instance);
    if (result != VK_SUCCESS)
    {
        heph_throw_error_unrecoverable("Cannot create vulkan instance");
    }
}

void Renderer::init_pdevice()
{
    uint32_t physical_device_count = 0;
    vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);

    if (physical_device_count == 0)
        heph_throw_error_unrecoverable("There are no physical devices that support Vulkan.");

    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data());

    /* = Pick the best physical device = */
    VkPhysicalDevice best_device = nullptr;

    /* = First pass to see if they support the queues and surface we need = */
    for (VkPhysicalDevice pdevice : physical_devices)
    {
        uint32_t queue_family_property_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &queue_family_property_count, nullptr);

        std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_property_count);
        vkGetPhysicalDeviceQueueFamilyProperties(pdevice, &queue_family_property_count, queue_family_properties.data());

        for (size_t i = 0; i < queue_family_property_count; i++)
        {
            VkQueueFamilyProperties info = queue_family_properties[i];

        }
    }

    /* = Second pass to choose the best devices based off physical speed = */

    if (physical_devices.size() == 0)
        heph_throw_error_unrecoverable("Your device supports Vulkan, but is unsuitable to run Hephaestus.");

    if (physical_devices.size() == 1) 
    {
        pdevice = physical_devices[0];
        return;
    }

    uint32_t best_score = 0;
    int best_index = 0;
    uint32_t device_scores[5] = { 2 , 4 , 5 , 3 , 1 };
    size_t i = 0;
    for (VkPhysicalDevice device : physical_devices)
    {
        VkPhysicalDeviceProperties properties = {};
        vkGetPhysicalDeviceProperties(device, &properties);
        uint32_t device_score = device_scores[(uint32_t) properties.deviceType];
        if (device_score > best_score)
        {
            best_score = device_score;
            best_index = i;
        }
        i++;
    }

    if (!best_device)
        heph_throw_error_unrecoverable("Your device supports Vulkan, but is unsuitable to run Hephaestus.");

    #if HEPH_VALIDATE
        std::cout << "Using physical device: " << (physical_devices[best_index].name) << '\n';
    #endif
}

void Renderer::init_device()
{   




    VkDeviceQueueCreateInfo queue_info = {};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;






    VkDeviceCreateInfo device_info = {};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;



    VkDevice device = nullptr;
    vkCreateDevice(pdevice, &device_info, nullptr, &device);
}

void Renderer::destroy()
{   
    /* = Make sure the device work is completed before destroying stuff = */
    vkDeviceWaitIdle(device);


    vkDestroyDevice(device, nullptr);   
    vkDestroyInstance(instance, nullptr);

}