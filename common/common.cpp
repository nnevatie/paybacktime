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

std::string readFile(const fs::path& path, bool binary)
{
    if (!fs::exists(path))
        throw std::runtime_error("File does not exist: " + path.generic_string());

    std::ifstream ifs(path.generic_string().c_str(), std::ios::in |
                     (binary ? std::ios::binary : std::ios_base::openmode(0)));

    const int size = int(fs::file_size(path));
    std::string buffer(size, 0);
    ifs.read(&buffer[0], size);

    return buffer;
}

}  // namespace
