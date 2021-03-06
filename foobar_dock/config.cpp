#include "foo_dock.h"
#include "dockwindow.h"
#include "resource.h"


#define OPT_SHOW "Show FooDock"


/************************************************/
/* Internal configuration variables            */
/**********************************************/

cfg_bool cfg_enabled(GetFooDockGUID(1), true);
cfg_window_placement cfg_popup_window_placement(GetFooDockGUID(2));



/************************************************/
/*   Main  menu  setting                       */
/**********************************************/

class mainmenu_commands_impl : public mainmenu_commands
{
	virtual t_uint32 get_command_count()
	{
		return 1;
	}

	virtual GUID get_command(t_uint32 p_index)
	{
		if (p_index < 2)
			return GetFooDockGUID(40 + p_index);

		return pfc::guid_null;
	}

	virtual void get_name(t_uint32 p_index, pfc::string_base & p_out)
	{
		if (p_index == 0)
			p_out = OPT_SHOW;
	}

	virtual bool get_description(t_uint32 p_index, pfc::string_base & p_out)
	{
		if (p_index == 0)
			p_out = "Toggles FooDock window.";
		else
			return false;

		return true;
	}

	virtual GUID get_parent()
	{
		return mainmenu_groups::view;
	}

	virtual void execute(t_uint32 p_index, service_ptr_t<service_base> p_callback)
	{
		if (p_index == 0 && core_api::assert_main_thread())
		{
			if (cfg_enabled)
				DockWindow::HideWindow();
			else
				DockWindow::ShowWindow();
		}
	}

	virtual bool get_display(t_uint32 p_index, pfc::string_base & p_text, t_uint32 & p_flags)
	{
		p_flags = 0;

		if (is_checked(p_index))
			p_flags |= flag_checked;

		get_name(p_index, p_text);
		return true;
	}

	bool is_checked(t_uint32 p_index)
	{
		if (p_index == 0)
			return cfg_enabled;

		return false;
	}
};


static mainmenu_commands_factory_t<mainmenu_commands_impl> dock_menu;



/************************************************/
/*   Advanced  Preferences                     */
/**********************************************/

static GUID advconf_foodock_branch_guid = GetFooDockGUID(20);

advconfig_branch_factory advconf_foodock_branch (
	"FooDock",
	advconf_foodock_branch_guid,
	advconfig_branch::guid_branch_display,
	0.1
);

advconfig_checkbox_factory advconfig_scroll_way (
	"Scrolling up changes to the next song (otherwise: previous)",
	GetFooDockGUID(39),
	advconf_foodock_branch_guid,//advconfig_branch::guid_branch_display,
	0.2,
	false
);

advconfig_string_factory advconfig_now_playing_format (
	"Now Playing format",
	GetFooDockGUID(38),
	advconf_foodock_branch_guid,
	0.3,
	"Now Playing: %artist% - %title%"
);

advconfig_string_factory advconfig_copy_title_format (
	"Copy Title format",
	GetFooDockGUID(33),
	advconf_foodock_branch_guid,
	0.4,
	"[%album artist% - ]['['%album%[ CD%discnumber%][ #%tracknumber%]']' ]%title%[ '//' %track artist%]"
);

advconfig_checkbox_factory advconfig_show_balloon_tip (
	"Show Balloon Tip",
	GetFooDockGUID(31),
	advconf_foodock_branch_guid,
	0.42,
	true
);

advconfig_string_factory advconfig_balloon_tip_format (
	"Balloon Tip format",
	GetFooDockGUID(30),
	advconf_foodock_branch_guid,
	0.43,
	"[%album artist% - ]['['%album%[ CD%discnumber%][ #%tracknumber%]']' ]%title%[ '//' %track artist%]"
);

advconfig_checkbox_factory advconfig_start_changed_playlist (
	"Autoplay new songs after playlist change (when have added something)",
	GetFooDockGUID(37),
	advconf_foodock_branch_guid,
	0.5,
	true
);

/*advconfig_checkbox_factory advconfig_show_preferences_option (
	"Show Preferences option",
	GetFooDockGUID(36),
	advconf_foodock_branch_guid,
	0.6,
	true
);*/

advconfig_checkbox_factory advconfig_switched_playlist_changes_song (
	"Switching playlist changes song (otherwise just select playlist)",
	GetFooDockGUID(35),
	advconf_foodock_branch_guid,
	0.7,
	true
);

advconfig_checkbox_factory advconfig_autosave_playlist (
	"Autosave modified playlist (saves configuration)",
	GetFooDockGUID(34),
	advconf_foodock_branch_guid,
	0.8,
	true
);

advconfig_checkbox_factory advconfig_playlist_undo_backup (
	"Create playlist Undo backup after playlist being modified",
	GetFooDockGUID(40),
	advconf_foodock_branch_guid,
	0.9,
	true
);

advconfig_checkbox_factory advconfig_dropped_folder_new_playlist (
	"Dropped folder creates new playlist (otherwise add to existing)",
	GetFooDockGUID(32),
	advconf_foodock_branch_guid,
	1.0,
	true
);


// konfiguracje zamiast advanced properties

// Defaults
bool default_cfg_show_preferences_option = true;


cfg_bool cfg_show_preferences_option(GetFooDockGUID(36), default_cfg_show_preferences_option);


/************************************************/
/*   Preferences -> Display                    */
/**********************************************/

class CFooDockPreferences : public CDialogImpl<CFooDockPreferences>, public preferences_page_instance
{

public:

	CFooDockPreferences(preferences_page_callback::ptr callback) : m_callback(callback) {}

	enum {IDD = IDD_FOODOCK_PREFERENCES};


	t_uint32 get_state()
	{
		t_uint32 state = preferences_state::resettable;
		if (HasChanged()) state |= preferences_state::changed;

		return state;
	}

	void apply()
	{
		cfg_show_preferences_option = SendDlgItemMessage(IDC_CHECKBOX_SHOW_PREFERENCES_OPT, BM_GETCHECK, 0, 0);
		int a = 5;
	}

	// Sets default settings
	void reset()
	{
		cfg_show_preferences_option = default_cfg_show_preferences_option;
	}


	BEGIN_MSG_MAP(CMyPreferences)
		MSG_WM_INITDIALOG(OnInitDialog)
		//COMMAND_HANDLER_EX(IDC_BOGO1, EN_CHANGE, OnEditChange)
		COMMAND_HANDLER_EX(IDC_CHECKBOX_SHOW_PREFERENCES_OPT, EN_CHANGE, OnEditChange)
	END_MSG_MAP()

private:

	const preferences_page_callback::ptr m_callback;

	BOOL OnInitDialog(CWindow, LPARAM)
	{
		UpdateDialog();
		
		return TRUE;
	}

	void UpdateDialog()
	{
		// Look style
		//SendDlgItemMessage(IDC_COMBOBOX_LOOKSTYLE, CB_INSERTSTRING, 0, (LPARAM)L"Alien cat");
		//SendDlgItemMessage(IDC_COMBOBOX_LOOKSTYLE, CB_INSERTSTRING, 1, (LPARAM)L"Album art");
		//SendDlgItemMessage(IDC_COMBOBOX_LOOKSTYLE, CB_SELECTSTRING, 0, (LPARAM)L"Alien cat");
		
		// Show Preferences option
		SendDlgItemMessage(IDC_CHECKBOX_SHOW_PREFERENCES_OPT, BM_SETCHECK, cfg_show_preferences_option, 0);
	}

	void CFooDockPreferences::OnEditChange(UINT, int, CWindow)
	{
		if (HasChanged())
			m_callback->on_state_changed();
	}

	BOOL HasChanged()
	{
		bool changed = false;
		
		if (cfg_show_preferences_option != SendDlgItemMessage(IDC_CHECKBOX_SHOW_PREFERENCES_OPT, BM_GETCHECK, 0, 0))
			changed = true;

		return changed;
	}

};

class preferences_page_foodock_impl : public preferences_page_impl<CFooDockPreferences>
{
public:
	const char* get_name() { return "FooDock"; }
	
	GUID get_guid() {
		return GetFooDockGUID(150);
	}

	GUID get_parent_guid() { return guid_display; }
};

static preferences_page_factory_t<preferences_page_foodock_impl> g_preferences_page_foodock_impl_factory;
