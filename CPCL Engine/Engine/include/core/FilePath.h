#pragma once
#include <filesystem>


class FilePath final {
private:
	std::filesystem::path _path;

public:
	explicit FilePath(const std::string& name = "");
	FilePath(const std::string& path, const std::string& name);

	[[nodiscard]] std::string getString() const { return _path.string(); }
	[[nodiscard]] bool empty() const { return _path.empty(); }
	[[nodiscard]] std::string getFileName() const;
	[[nodiscard]] std::string getParentPath() const;
	/**
	 * @brief Checks if the file path contains the specified sub path (tag).
	 * @param tag The FilePath object to check for containment.
	 * @return True if this path contains the specified tag, false otherwise.
	 */
	[[nodiscard]] bool contains(const FilePath& tag) const;

	bool operator==(const FilePath& name) const { return _path == name._path; }
	bool operator!=(const FilePath& name) const { return _path != name._path; }
	bool operator<(const FilePath& name) const { return _path < name._path; }
};
