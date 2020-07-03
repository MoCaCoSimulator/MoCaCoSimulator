#include "Utils.h"

std::string Utils::FilenameFromPath(const std::string& path, bool withExtension, const std::string& delims)
{
    std::string basename = path.substr(path.find_last_of(delims) + 1);
    if (withExtension)
        return basename;
    else
    {
        typename std::string::size_type const p(basename.find_last_of('.'));
        return p > 0 && p != std::string::npos ? basename.substr(0, p) : basename;
    }
}

std::string Utils::FoldernameFromPath(const std::string& path, const std::string& delims)
{
    return FilenameFromPath(path, true, delims);
}