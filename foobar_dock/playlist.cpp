#include "foo_dock.h"
#include "playlist.h"


void Playlist::AddSongsList(const pfc::list_base_const_t<metadb_handle_ptr>&list, bool play_first)
{
	static_api_ptr_t<playlist_manager> pm;
	static_api_ptr_t<playback_control> pc;

	int currently_played_index = pm->activeplaylist_get_item_count();
	int num = list.get_count();
	//pfc::list_t<metadb_handle_ptr> list;
	metadb_handle_ptr pSong;

	// A given playlist is already checked now.
	// So we just add it to the player.
	
	
	// Converting locations to metadb handlers
//	list = _list;
	/*for (int i = 0; i < num; i++)
	{
		const char* pStr = _list.get_item(i);
		static_api_ptr_t<metadb>()->handle_create(pSong, make_playable_location::make_playable_location(pStr,0));
		list.add_item(pSong);
	}*/

	//selection on all added files
	bit_array_var_table* mask = new bit_array_var_table(new bool[num], num, true);	
	
	// Adding files to the real playlist
	pm->activeplaylist_add_items(list, *mask);

	// Play first file
	if (play_first)
	{
		pm->activeplaylist_clear_selection();
		pm->activeplaylist_set_selection_single(currently_played_index, true);
		pc->start(playback_control::t_track_command::track_command_next);
	}
}


void Playlist::SelectPlayList(std::wstring& filename)
{
	throw new std::exception("not yet implemented");
}


void Playlist::GetSongTitle(metadb_handle_ptr& song, pfc::string8& format, pfc::string_formatter& text)
{
	static_api_ptr_t<playback_control> pc;
	service_ptr_t<titleformat_object> script;
			
	static_api_ptr_t<titleformat_compiler>()->compile_safe(script, format);
	pc->playback_format_title_ex(song, NULL, text, script, NULL, play_control::display_level_titles);
}

// returns UTF-8
std::wstring Playlist::GetPlayingTitle()
{
	pfc::string8 format = "%title%";

	return GetPlayingTitle(format);
}

std::string Playlist::GetPlayingTitleA()
{
	pfc::string8 format = "%title%";

	return GetPlayingTitleA(format);
}

// returns UTF-8
std::wstring Playlist::GetPlayingTitle(pfc::string8& _format)
{
	return UtfConverter::FromUtf8(GetPlayingTitleA(_format));
}

std::string Playlist::GetPlayingTitleA(pfc::string8& _format)
{
	static_api_ptr_t<playback_control> pc;
	metadb_handle_ptr pSong;
	pfc::string8 format = _format;
	pfc::string_formatter out_text;

	pc->get_now_playing(pSong);
	GetSongTitle(pSong, format, out_text);

	return std::string(out_text.get_ptr());
}
