#pragma once

#include "json.h"
#include "file_system.h"

namespace pt
{

json readJson(const fs::path& path);

} // namespace pt
