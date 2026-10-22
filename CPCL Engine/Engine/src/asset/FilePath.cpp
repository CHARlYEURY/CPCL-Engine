#include "FilePath.h"

FilePath::FilePath(const std::string& name) : _path(name) {}

FilePath::FilePath(const std::string& path, const std::string& name) :
	_path(std::filesystem::path(path) / name) {}


std::string FilePath::getFileName() const { return _path.filename().string(); }

std::string FilePath::getParentPath() const { return _path.parent_path().string(); }

bool FilePath::contains(const FilePath& tag) const {
	return _path.string().find(tag._path.string()) != std::string::npos;
}