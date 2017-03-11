#pragma once

#include "foo_dock.h"


/**
 * FileScan
 * Helper class, it's used to check dropped files.
 */
class FileScan
{

public:

	static bool IsMusicFile(std::string& url);
	static bool IsDirectory(std::string& url);

};
