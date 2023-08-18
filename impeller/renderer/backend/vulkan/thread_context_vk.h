#pragma once

#include <memory>

namespace impeller {
class ThreadContextVK {
 public:
  ThreadContextVK(std::weak_ptr<ContextVK> context) : context_(context) {}

 private:
  std::weak_ptr<ContextVK> context_;
  std::shared_ptr<CommandPoolVK> command_pool_;
};
}  // namespace impeller