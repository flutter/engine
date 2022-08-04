#include "impeller/renderer/backend/vulkan/framebuffer_vk.h"
#include <array>

#include "fml/logging.h"
#include "impeller/renderer/backend/vulkan/swapchain_vk.h"

namespace impeller {

std::shared_ptr<FramebufferVK> FramebufferVK::Create(
    vk::Device device,
    const RenderTarget& render_target,
    vk::RenderPass render_pass) {
  vk::FramebufferCreateInfo create_info;
  create_info.setRenderPass(render_pass);

  render_target.GetColorAttachments()
}

// std::shared_ptr<FramebufferVK> FramebufferVK::Create(
//     vk::Device device,
//     std::shared_ptr<SwapchainVK> swapchain,
//     vk::RenderPass render_pass) {
//   FML_UNREACHABLE();
// const auto& swapchain_images = swapchain->GetTextures();
// std::vector<vk::UniqueFramebuffer> framebuffers;

// for (size_t i = 0; i < swapchain_images.size(); ++i) {
//   const auto& image = swapchain_images[i];

//   std::array<vk::ImageView, 1> attachments = {*image->image_view_};

//   vk::FramebufferCreateInfo create_info;
//   create_info.setRenderPass(render_pass);
//   create_info.setAttachments(attachments);
//   create_info.setWidth(swapchain->extent_.width);
//   create_info.setHeight(swapchain->extent_.height);
//   create_info.setLayers(1);

//   auto framebuffer_res = device.createFramebufferUnique(create_info);
//   if (framebuffer_res.result != vk::Result::eSuccess) {
//     FML_CHECK(false) << "Failed to create framebuffer: "
//                      << vk::to_string(framebuffer_res.result);
//     return nullptr;
//   }

//   framebuffers.push_back(std::move(framebuffer_res.value));
// }

// return std::make_shared<FramebufferVK>(std::move(framebuffers));
// }

}  // namespace impeller
