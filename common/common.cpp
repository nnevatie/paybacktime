#include "common.h"

namespace pt
{

SourceLocation::SourceLocation(const char* file, const char* func, int line) :
    file(file), func(func), line(line)
{
}

std::string str(const std::ostream& ostr)
{
    return static_cast<const std::ostringstream&>(ostr).str();
}

std::string readFile(const filesystem::path& path, bool binary)
{
    if (!filesystem::exists(path))
        throw std::runtime_error("File does not exist: " + path.string());

    std::ifstream ifs(path.string().c_str(), std::ios::in |
                     (binary ? std::ios::binary : std::ios_base::openmode(0)));

    const int size = int(filesystem::file_size(path));
    std::string buffer(size, 0);
    ifs.read(&buffer[0], size);

    return buffer;
}

}  // namespace
