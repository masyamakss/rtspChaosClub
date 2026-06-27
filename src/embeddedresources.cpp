#include "embeddedresources.h"
#include <cmrc/cmrc.hpp>

CMRC_DECLARE(web_resources);

std::string EmbeddedResources::loadText(const std::string& path)
{
    const auto fileSystem = cmrc::web_resources::get_filesystem();

    const auto file = fileSystem.open(path);

    return std::string(file.begin(), file.end());
}