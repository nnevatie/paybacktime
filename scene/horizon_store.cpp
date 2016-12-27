#include "horizon_store.h"

#include "common/log.h"

namespace pt
{

struct HorizonStore::Data
{
    Data()
    {}

    Horizons horizons;
};

HorizonStore::HorizonStore(const fs::path& path) :
    d(std::make_shared<Data>())
{
    for (const auto& entry : fs::directory_iterator(path))
        if (fs::is_regular_file(entry))
            d->horizons.push_back(Horizon(entry.path()));

    // Move none-horizon as the first item
    if (const auto none = horizon("none"))
    {
        d->horizons.erase(std::remove(
            d->horizons.begin(), d->horizons.end(), none), d->horizons.end());
        d->horizons.insert(d->horizons.begin(), none);
    }
    PTLOG(Info) << "Horizons: " << d->horizons.size();
    for (const auto& horz : d->horizons)
        PTLOG(Info) << horz.name();
}

HorizonStore::Horizons HorizonStore::horizons() const
{
    return d->horizons;
}

Horizon HorizonStore::horizon(const std::string& name) const
{
    for (const auto& horz : d->horizons)
        if (horz.name() == name)
            return horz;

    return Horizon();
}

} // namespace pt
