#pragma once

#include <memory>
#include <vector>

#include "common/file_system.h"

#include "horizon.h"

namespace pt
{

struct HorizonStore
{
    using Horizons = std::vector<Horizon>;

    HorizonStore(const fs::path& path);

    Horizons horizons() const;

    Horizon horizon(const std::string& name) const;

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace pt
