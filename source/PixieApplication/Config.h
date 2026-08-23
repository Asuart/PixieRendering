#pragma once
#include <filesystem>

namespace PixieApp {

class Config {
  public:
	static void SetApplicationPath(const std::filesystem::path& path);

	static std::filesystem::path GetApplicationPath();
	static std::filesystem::path GetApplicationDirectory();
	static std::filesystem::path ExpandPathToResources(const std::filesystem::path& path);

  protected:
	static std::filesystem::path m_applicationPath;
};

} // namespace PixieApp
