#include "file_scan.h"

bool FileScan::IsMusicFile(std::string& url)
{
	return true;
}

bool FileScan::IsDirectory(std::string& url)
{
	abort_callback_impl abort_call;
	pfc::string8 str;

	//return value
	bool is_dir = false;


	filesystem::g_get_canonical_path(url.c_str(), str);

	if (filesystem::g_is_valid_directory(str.get_ptr(), abort_call))
	{
		if (!filesystem::g_is_empty_directory(str.get_ptr(), abort_call))
			is_dir = true;
	}

	return is_dir;
}
