#ifndef SHELL_COMMON_SHOREBIRD_H_
#define SHELL_COMMON_SHOREBIRD_H_

#include "flutter/common/settings.h"

namespace flutter {

void ConfigureShorebird(std::string cache_path,
                        flutter::Settings& settings,
                        const std::string& shorebird_yaml,
                        const std::string& version,
                        int64_t version_code);

}  // namespace flutter

#endif  // SHELL_COMMON_SHOREBIRD_H_
