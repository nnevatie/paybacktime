#include "object.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <glm/gtx/transform.hpp>

#include "common/log.h"

namespace bpt = boost::property_tree;

namespace pt
{

namespace
{

bpt::ptree readJson(const fs::path& path)
{
    try
    {
        bpt::ptree tree;
        bpt::read_json(path.string(), tree);
        return tree;
    }
    catch (const std::exception& e)
    {
        HCLOG(Warn) << e.what();
    }
    return bpt::ptree();
}

template <typename T>
T getVec(const bpt::ptree& tree, const std::string& key)
{
    try
    {
        T vec;
        auto children = tree.get_child(key);
        for (auto it = children.begin(); it != children.end(); ++it)
            vec[std::distance(children.begin(), it)] =
                it->second.get_value<float>();

        return vec;
    }
    catch (const std::exception& e)
    {}
    return {};
}

}

struct Object::Data
{
    Data(const fs::path& path, TextureStore* textureStore) :
        model(path, textureStore)
    {
        bpt::ptree tree(readJson(path / "object.json"));
        // TODO: Parse json properties
        name   = path.filename().string();
        origin = getVec<glm::vec3>(tree, "origin");
    }

    Model       model;
    std::string name;
    glm::vec3   origin;
};

Object::Object()
{}

Object::Object(const fs::path& path, TextureStore* textureStore) :
    d(std::make_shared<Data>(path, textureStore))
{}

Object::operator==(const Object& other) const
{
    return d == other.d;
}

Object::operator!=(const Object& other) const
{
    return !operator==(other);
}

Object::operator bool() const
{
    return d.operator bool();
}

std::string Object::name() const
{
    return d->name;
}

glm::vec3 Object::origin() const
{
    return d->origin;
}

glm::mat4x4 Object::transform() const
{
    return glm::translate(d->origin);
}

Model Object::model() const
{
    return d->model;
}

} // namespace pt
