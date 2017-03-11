#pragma once

#include "foo_dock.h"


/************************************************/
/* Internal configuration variables            */
/**********************************************/

extern cfg_bool cfg_enabled;
extern cfg_window_placement cfg_popup_window_placement;



/************************************************/
/*   Advanced  Preferences                     */
/**********************************************/

extern advconfig_checkbox_factory advconfig_scroll_way;
extern advconfig_string_factory advconfig_now_playing_format;
extern advconfig_string_factory advconfig_copy_title_format;
extern advconfig_checkbox_factory advconfig_start_changed_playlist;
//extern advconfig_checkbox_factory advconfig_show_preferences_option;
extern advconfig_checkbox_factory advconfig_switched_playlist_changes_song;
extern advconfig_checkbox_factory advconfig_autosave_playlist;
extern advconfig_checkbox_factory advconfig_playlist_undo_backup;
extern advconfig_checkbox_factory advconfig_dropped_folder_new_playlist;
extern advconfig_checkbox_factory advconfig_show_balloon_tip;
extern advconfig_string_factory advconfig_balloon_tip_format;


extern cfg_bool cfg_show_preferences_option;