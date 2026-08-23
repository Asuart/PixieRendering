#include "Config.h"

namespace PixieApp {

std::filesystem::path Config::m_applicationPath = "";

void Config::SetApplicationPath(const std::filesystem::path& path) {
	m_applicationPath = path;
}

std::filesystem::path Config::GetApplicationPath() {
	return m_applicationPath;
}

std::filesystem::path Config::GetApplicationDirectory() {
	return m_applicationPath.parent_path();
}

std::filesystem::path Config::ExpandPathToResources(const std::filesystem::path& path) {
	return GetApplicationDirectory().string() + std::string("/resources/") + path.string();
}

} // namespace PixieApp
