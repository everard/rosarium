// Copyright Nezametdinov E. Ildus 2025.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

// Imports.
import rose.graphics.vulkan.device;
import rose.graphics.vulkan.swapchain;
import std.compat;

namespace rose {

////////////////////////////////////////////////////////////////////////////////
// Error definition.
////////////////////////////////////////////////////////////////////////////////

struct error {
    long long line;
    graphics::vulkan::error underlying;
};

////////////////////////////////////////////////////////////////////////////////
// Window definition.
////////////////////////////////////////////////////////////////////////////////

namespace detail {

using window_deleter = decltype([](SDL_Window* window) {
    if(window != nullptr) {
        std::cout << "Deleting SDL window\n";
        SDL_DestroyWindow(window);
    }
});

} // namespace detail

using window = std::unique_ptr<SDL_Window, detail::window_deleter>;

////////////////////////////////////////////////////////////////////////////////
// Main context definition.
////////////////////////////////////////////////////////////////////////////////

struct main_context {
    // Window.
    SDL_Window* window;

    // Instance and selected physical device.
    graphics::vulkan::instance instance;
    graphics::vulkan::physical_device physical_device;

    // Window surface and its properties.
    graphics::vulkan::surface surface;
    graphics::vulkan::surface_properties surface_properties;

    // Device and its queues.
    graphics::vulkan::device device;
    graphics::vulkan::device_queue_list device_queues;

    // Semaphores.
    struct {
        graphics::vulkan::semaphore rendering, swapchain;
    } semaphores;

    // Command pool and command buffers.
    graphics::vulkan::command_pool command_pool;
    std::array<VkCommandBuffer, 16> command_buffers;

    // Swapchain and its parameters.
    graphics::vulkan::swapchain swapchain;
    graphics::vulkan::swapchain_parameters swapchain_parameters;

    // Array of swapchain images.
    std::vector<VkImage> swapchain_images;
};

////////////////////////////////////////////////////////////////////////////////
// Vulkan extension query functions.
////////////////////////////////////////////////////////////////////////////////

auto
obtain_instance_extensions(SDL_Window* window) -> std::vector<char const*> {
    auto n = unsigned{};
    if(SDL_Vulkan_GetInstanceExtensions(window, &n, nullptr) != SDL_TRUE) {
        return {};
    }

    auto extensions = std::vector<char const*>{n, nullptr};
    if(SDL_Vulkan_GetInstanceExtensions(window, &n, extensions.data()) !=
       SDL_TRUE) {
        return {};
    }

    return extensions;
}

auto
obtain_device_extensions() -> std::vector<char const*> {
    return {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}

////////////////////////////////////////////////////////////////////////////////
// Command buffers construction function.
////////////////////////////////////////////////////////////////////////////////

auto
construct_command_buffers(main_context& context) -> std::expected<void, error> {
    // Reset command pool.
    if(vkResetCommandPool(context.device, context.command_pool, 0) !=
       VK_SUCCESS) {
        return std::unexpected{error{.line = __LINE__}};
    }

    // Obtain queue family index.
    auto queue_family_index = context.device.queue_family_index;

    // Record command buffers for all swapchain images.
    auto command_buffers = std::span{context.command_buffers}.first(
        std::min(
            context.swapchain_images.size(), context.command_buffers.size()));

    for(auto i = 0zU; auto command_buffer : command_buffers) {
        // Initialize image sub-resource range.
        auto image_subresource_range = VkImageSubresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1};

        // Begin recording.
        if(true) {
            auto info = VkCommandBufferBeginInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

            if(vkBeginCommandBuffer(command_buffer, &info) != VK_SUCCESS) {
                return std::unexpected{error{.line = __LINE__}};
            }
        }

        // Add pipeline barrier.
        if(true) {
            VkImageMemoryBarrier barriers[] = {
                {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                 .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
                 .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                 .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                 .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                 .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                 .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                 .image = context.swapchain_images[i],
                 .subresourceRange = image_subresource_range}};

            vkCmdPipelineBarrier(
                command_buffer,
                // Stage masks, dependency flags.
                VK_PIPELINE_STAGE_TRANSFER_BIT, // Source stage.
                VK_PIPELINE_STAGE_TRANSFER_BIT, // Destination stage.
                0,                              // Dependency flags.
                // Global memory barriers.
                0, nullptr,
                // Buffer memory barriers.
                0, nullptr,
                // Image memory barriers.
                static_cast<uint32_t>(std::ranges::size(barriers)), barriers);
        }

        // Clear the image.
        if(auto color = VkClearColorValue{.float32 = {0.5f, 0.5f, 0.1f, 1.0f}};
           true) {
            vkCmdClearColorImage(
                command_buffer, context.swapchain_images[i],
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &color, 1,
                &image_subresource_range);
        }

        // Add pipeline barrier.
        if(true) {
            VkImageMemoryBarrier barriers[] = {
                {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                 .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                 .dstAccessMask = 0,
                 .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                 .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                 .srcQueueFamilyIndex = queue_family_index.graphics,
                 .dstQueueFamilyIndex = queue_family_index.presentation,
                 .image = context.swapchain_images[i],
                 .subresourceRange = image_subresource_range}};

            vkCmdPipelineBarrier(
                command_buffer,
                // Stage masks, dependency flags.
                VK_PIPELINE_STAGE_TRANSFER_BIT,       // Source stage.
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // Destination stage.
                0,                                    // Dependency flags.
                // Global memory barriers.
                0, nullptr,
                // Buffer memory barriers.
                0, nullptr,
                // Image memory barriers.
                static_cast<uint32_t>(std::ranges::size(barriers)), barriers);
        }

        // End recording.
        if(vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
            return std::unexpected{error{.line = __LINE__}};
        }

        i++;
    }

    return {};
}

////////////////////////////////////////////////////////////////////////////////
// Swapchain initialization function.
////////////////////////////////////////////////////////////////////////////////

auto
initialize_swapchain(main_context& context) -> std::expected<void, error> {
    // Clear array of swapchain images.
    context.swapchain_images.clear();

    // Update swapchain parameters.
    if(auto width = 0, height = 0; true) {
        if(SDL_GetWindowSize(context.window, &width, &height);
           (width <= 0) || (height <= 0)) {
            return std::unexpected{error{.line = __LINE__}};
        }

        context.swapchain_parameters.image_extent.width =
            static_cast<uint32_t>(width);

        context.swapchain_parameters.image_extent.height =
            static_cast<uint32_t>(height);
    }

    // Initialize the swapchain.
    if(context.swapchain.parent = context.device; true) {
        auto object = initialize(
            std::move(context.swapchain), context.swapchain_parameters);

        if(!object) {
            return std::unexpected{
                error{.line = __LINE__, .underlying = object.error()}};
        } else {
            context.swapchain = std::move(*object);
        }
    }

    // Obtain swapchain images.
    if(auto images = obtain_images(context.swapchain); !images) {
        return std::unexpected{
            error{.line = __LINE__, .underlying = images.error()}};
    } else {
        context.swapchain_images = std::move(*images);
    }

    // Construct command buffers.
    return construct_command_buffers(context);
}

////////////////////////////////////////////////////////////////////////////////
// Main context initialization function.
////////////////////////////////////////////////////////////////////////////////

auto
initialize_main_context(SDL_Window* window)
    -> std::expected<main_context, error> {
    // Initialize an empty result.
    auto context = main_context{.window = window};

    // Initialize Vulkan instance.
    if(true) {
        auto object = initialize(
            graphics::vulkan::instance_parameters{
                .api_version = VK_API_VERSION_1_3,
                .extensions = obtain_instance_extensions(window)});

        if(!object) {
            return std::unexpected{
                error{.line = __LINE__, .underlying = object.error()}};
        } else {
            context.instance = std::move(*object);
        }
    }

    // Select physical device.
    if(true) {
        auto object = select(
            context.instance,
            graphics::vulkan::physical_device_preference{
                .api_version = VK_API_VERSION_1_3,
                .type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU});

        if(!object) {
            return std::unexpected{
                error{.line = __LINE__, .underlying = object.error()}};
        } else {
            context.physical_device = *object;
        }
    }

    // Obtain physical device features.
    auto features =
        graphics::vulkan::physical_device_features{context.physical_device};

    if(true) {
        // Check for support of specific features.
        //
        // Note: This should vary depending on particular application
        // requirements.
        auto is_feature_complete =
            features.vulkan_1_2.descriptorIndexing &&
            features.vulkan_1_2.descriptorBindingVariableDescriptorCount &&
            features.vulkan_1_2.drawIndirectCount &&
            features.common.features.multiDrawIndirect &&
            features.common.features.drawIndirectFirstInstance;

        if(!is_feature_complete) {
            return std::unexpected{error{.line = __LINE__}};
        }
    }

    // Create window surface.
    if(true) {
        auto object = initialize(
            context.instance,
            graphics::vulkan::surface_parameters{.window = window});

        if(!object) {
            return std::unexpected{
                error{.line = __LINE__, .underlying = object.error()}};
        } else {
            context.surface = std::move(*object);
        }
    }

    // Obtain surface properties.
    if(true) {
        auto object =
            obtain_properties(context.surface, context.physical_device);

        if(!object) {
            return std::unexpected{
                error{.line = __LINE__, .underlying = object.error()}};
        } else {
            context.surface_properties = std::move(*object);
        }
    }

    // Check surface properties.
    if((context.surface_properties.capabilities.supportedUsageFlags &
        VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0) {
        return std::unexpected{error{.line = __LINE__}};
    }

    if(context.surface_properties.capabilities.minImageCount > 3) {
        return std::unexpected{error{.line = __LINE__}};
    }

    // Initialize swapchain parameters.
    if(true) {
        context.swapchain_parameters = graphics::vulkan::swapchain_parameters{
            .surface = context.surface,
            .surface_transform =
                context.surface_properties.capabilities.currentTransform,
            .image_usage_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                 VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            .image_count = 3,
            .image_array_layer_count = 1};

        // Select image format.
        context.swapchain_parameters.surface_format =
            context.surface_properties.formats[0];

        for(auto format : context.surface_properties.formats) {
            if((format.format == VK_FORMAT_R8G8B8A8_UNORM) ||
               (format.format == VK_FORMAT_R8G8B8A8_SNORM)) {
                context.swapchain_parameters.surface_format = format;
                break;
            }
        }

        // Select presentation mode.
        context.swapchain_parameters.presentation_mode =
            VK_PRESENT_MODE_FIFO_KHR;

        for(auto mode : context.surface_properties.presentation_modes) {
            if(mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                context.swapchain_parameters.presentation_mode = mode;
                break;
            }
        }
    }

    // Initialize Vulkan device.
    if(true) {
        auto object = initialize(
            context.physical_device,
            graphics::vulkan::device_parameters{
                .extensions = obtain_device_extensions(),
                .features = features,
                .surface = context.surface});

        if(!object) {
            return std::unexpected{
                error{.line = __LINE__, .underlying = object.error()}};
        } else {
            context.device = std::move(*object);
        }
    }

    // Obtain device's queues.
    context.device_queues = obtain_device_queue_list(context.device);

    // Initialize semaphores.
    if(true) {
        auto semaphores = std::array{
            &(context.semaphores.rendering), &(context.semaphores.swapchain)};

        for(auto target : semaphores) {
            auto object = initialize<graphics::vulkan::semaphore>(
                vkCreateSemaphore, context.device,
                {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO});

            if(!object) {
                return std::unexpected{
                    error{.line = __LINE__, .underlying = object.error()}};
            } else {
                *target = std::move(*object);
            }
        }
    }

    // Initialize command pool.
    if(true) {
        auto object = initialize<graphics::vulkan::command_pool>(
            vkCreateCommandPool, context.device,
            {.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
             .queueFamilyIndex = context.device.queue_family_index.graphics});

        if(!object) {
            return std::unexpected{
                error{.line = __LINE__, .underlying = object.error()}};
        } else {
            context.command_pool = std::move(*object);
        }
    }

    // Allocate command buffers.
    if(true) {
        auto command_buffers = std::span{context.command_buffers};

        auto info = VkCommandBufferAllocateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = context.command_pool,
            .commandBufferCount =
                static_cast<uint32_t>(std::ranges::size(command_buffers))};

        if(vkAllocateCommandBuffers(
               context.device, &info, command_buffers.data()) != VK_SUCCESS) {
            return std::unexpected{error{.line = __LINE__}};
        }
    }

    // Initialize swapchain.
    if(auto result = initialize_swapchain(context); !result) {
        return std::unexpected{result.error()};
    }

    return std::move(context);
}

////////////////////////////////////////////////////////////////////////////////
// Rendering function.
////////////////////////////////////////////////////////////////////////////////

void
render(main_context& context) {
    if(auto width = 0, height = 0; true) {
        if(SDL_GetWindowSize(context.window, &width, &height);
           (width <= 0) || (height <= 0)) {
            return;
        }
    }

    // Acquire the next swapchain image.
    auto image_index = uint32_t{};
    if(vkAcquireNextImageKHR(
           context.device, context.swapchain, UINT64_MAX,
           context.semaphores.swapchain, nullptr, &image_index) != VK_SUCCESS) {
        return;
    }

    // Render the next frame.
    if(true) {
        VkPipelineStageFlags pipeline_stage_flags[] = {
            VK_PIPELINE_STAGE_TRANSFER_BIT};

        auto info = VkSubmitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &(context.semaphores.swapchain.handle),
            .pWaitDstStageMask = pipeline_stage_flags,
            .commandBufferCount = 1,
            .pCommandBuffers = &(context.command_buffers[image_index]),
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &(context.semaphores.rendering.handle)};

        if(vkQueueSubmit(context.device_queues.graphics, 1, &info, nullptr) !=
           VK_SUCCESS) {
            return;
        }
    }

    // Present rendered frame.
    if(true) {
        auto info = VkPresentInfoKHR{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &(context.semaphores.rendering.handle),
            .swapchainCount = 1,
            .pSwapchains = &(context.swapchain.handle),
            .pImageIndices = &image_index};

        vkQueuePresentKHR(context.device_queues.presentation, &info);
    }
}

} // namespace rose

////////////////////////////////////////////////////////////////////////////////
// Program entry point.
////////////////////////////////////////////////////////////////////////////////

int
main() {
    // Initialize SDL subsystems.
    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        return 0;
    }

    // Make sure SDL state is cleaned-up upon exit.
    struct guard {
        ~guard() {
            std::cout << "Deleting SDL state\n";
            SDL_Quit();
        }
    } _;

    // Create a new window.
    auto window = rose::window{SDL_CreateWindow(
        "Main", 0, 0, 1280, 720, SDL_WINDOW_BORDERLESS | SDL_WINDOW_VULKAN)};

    if(!window) {
        return 0;
    }

    // Initialize main context.
    auto context = rose::initialize_main_context(window.get());
    if(!context) {
        std::cout << "Main context initialization failed.\n";
        return 0;
    }

    // Run event loop.
    for(bool should_run = true; should_run;) {
        // Get current frame time.
        auto t0 = std::chrono::high_resolution_clock::now();

        // Process events.
        for(SDL_Event event; SDL_PollEvent(&event) != 0;) {
            switch(event.type) {
                case SDL_QUIT:
                    should_run = false;
                    break;

                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym) {
                        case SDLK_q:
                            should_run = false;
                            break;

                        default:
                            break;
                    }

                    break;

                case SDL_KEYUP:
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if(event.button.button == SDL_BUTTON_LEFT) {
                        // Process mouse button event.
                    }

                    break;

                case SDL_MOUSEBUTTONUP:
                    if(event.button.button == SDL_BUTTON_LEFT) {
                        // Process mouse button event.
                    }

                    break;

                case SDL_MOUSEMOTION:
                    // Process mouse motion event.
                    break;

                default:
                    break;
            }
        }

        // Render the next frame.
        render(*context);

        // Sleep between frames to reduce CPU load.
        auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - t0);

        if(delta.count() < 16) {
            auto dt = 16 - delta.count();
            std::this_thread::sleep_for(std::chrono::milliseconds{dt});
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds{2});
        }
    }

    if(context->device.handle != nullptr) {
        vkDeviceWaitIdle(context->device);
    }

    return 0;
}
