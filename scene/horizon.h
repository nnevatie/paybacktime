#pragma once

#include <memory>
#include <string>

#include "common/file_system.h"

namespace pt
{

struct Horizon
{
    Horizon();
    Horizon(const fs::path& path);

    operator bool() const;

    operator==(const Horizon& other) const;
    operator!=(const Horizon& other) const;

    std::string name() const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
