#ifndef __PLAYLIST__H
#define __PLAYLIST__H

#include "foo_dock.h"


class Playlist
{

public:

	static void AddSongsList(const pfc::list_base_const_t<metadb_handle_ptr>& list, bool play_first);
	static void SelectPlayList(std::wstring& filename);
	static void GetSongTitle(metadb_handle_ptr& song, pfc::string8& format, pfc::string_formatter& text);
	static std::wstring GetPlayingTitle();
	static std::string GetPlayingTitleA();
	static std::wstring GetPlayingTitle(pfc::string8& _format);
	static std::string  GetPlayingTitleA(pfc::string8& _format);


};


#endif
