#include "dockwindow.h"
#include "playlist.h"
#include "file_scan.h"
#include "config.h"


/**
 * Global variables
 */

DockWindow DockWindow::g_instance;



/********************************************************************************* /
/*    class DockWindow implementation                                              /
/**********************************************************************************/


void DockWindow::ShowWindow()
{
	if (!g_instance.IsWindow())
		cfg_enabled = (g_instance.Create() != NULL);

	else if (g_instance.IsWindow())
		SetWindowPos(g_instance.getHandle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
}

void DockWindow::HideWindow()
{
	cfg_enabled = false;
	g_instance.Destroy();
}

HWND DockWindow::getHandle()
{
	return g_instance.m_hWnd;
}

void DockWindow::DestroyWindow()
{
	g_instance.Destroy();
}

HWND DockWindow::Create()
{
	HWND hwnd = super::Create(0,
			TEXT(""),
			WS_POPUP | WS_VISIBLE,
			WS_EX_CLIENTEDGE | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
			POS_X, POS_Y, DOCK_SIZE, DOCK_SIZE
		);

	DragAcceptFiles(hwnd, TRUE);
	SetLayeredWindowAttributes(hwnd, 0, 95, LWA_ALPHA);
	::ShowWindow(hwnd, SW_SHOWNORMAL);

	return hwnd;
}

static HHOOK g_hookMouse;
LRESULT CALLBACK GlobalMouseHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	try
	{
		MSLLHOOKSTRUCT* hook = NULL;
		HWND hwnd = DockWindow::getHandle();

		if (wParam == WM_MOUSEWHEEL)
		{
			hook = (MSLLHOOKSTRUCT*)lParam;
			
			if (WindowFromPoint(hook->pt) == hwnd)
			{
				PostMessage(hwnd, WM_MOUSEWHEEL, hook->mouseData, hook->flags);
				//hook->flags |= LLMHF_INJECTED;
			}			
		}	
	}
	catch (std::exception& ex)
	{
	}
	
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

BOOL DockWindow::ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult)
{
	switch (uMsg)
	{
		case WM_CREATE:
		{
			lResult = OnCreate(reinterpret_cast<LPCREATESTRUCT>(lParam));
			m_mouseTracking = false;
			return TRUE;
		}

		case WM_DESTROY:
		{
			OnDestroy();
			return TRUE;
		}

		case WM_CLOSE:
		{
			return TRUE;
		}

		case WM_LBUTTONDOWN:
		{
			OnLButtonDown(wParam, CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
			return TRUE;
		}

		case WM_LBUTTONDBLCLK:
		{
			OnLButtonDoubleClick(wParam, CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
			return TRUE;
		}

		case WM_MBUTTONDOWN:
		{
			OnMButtonDown();
			return TRUE;
		}

		case WM_MBUTTONDBLCLK:
		{
			OnChangeLook();
			return TRUE;
		}

		case WM_CONTEXTMENU:
		{
			OnContextMenu((HWND)wParam, CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
			return TRUE;
		}

		case WM_PAINT:
		{
			OnPaint((HDC)wParam);
			return TRUE;
		}

		case WM_DROPFILES:
		{
			OnDropFiles((HDROP)wParam);
			return TRUE;
		}

		case WM_MOUSEMOVE:
		{
			OnMouseMove(wParam, CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));

			if (!m_mouseTracking)
				TrackMouse();

			if (!g_hookMouse)
				g_hookMouse = SetWindowsHookEx(WH_MOUSE_LL, GlobalMouseHook, GetModuleHandle(NULL), 0);

			lResult = TRUE;
			return TRUE;
		}

		case WM_MOUSELEAVE:
		{
			UnhookWindowsHookEx(g_hookMouse);
			g_hookMouse = NULL;
			m_mouseTracking = false;

			OnMouseLeave();
			return TRUE;
		}

		case WM_MOUSEWHEEL:
		{
			POINT point;
			GetCursorPos(&point);
			if (WindowFromPoint(point) == m_hWnd)
			{
				OnMouseScroll(
						GET_WHEEL_DELTA_WPARAM(wParam),
						GET_KEYSTATE_WPARAM(wParam),
						CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))
					);
			}

			return TRUE;
		}

		case WM_NOTIFY:
		{
			lResult = 1;
			if (((LPNMHDR)lParam)->code == TTN_GETDISPINFO)
			{
				OnBalloonTip((LPNMHDR)lParam, (LPNMTTDISPINFO)lParam);
			}

			return TRUE;
		}

		case WM_WINDOWPOSCHANGING:
		{
			ShowWindow();
			return TRUE;
		}

	}

	// The framework will call DefWindowProc() for us.
	return FALSE;
}

LRESULT DockWindow::OnCreate(LPCREATESTRUCT pCreateStruct)
{
	//Report("DockWindow::OnCreate");

	//if (DefWindowProc(m_hWnd, WM_CREATE, 0, (LPARAM)pCreateStruct) != 0)
	//	return -1;

	// If "Remember window positions" is enabled, this will
	// restore the last position of our window. Otherwise it
	// won't do anything.
	cfg_popup_window_placement.on_window_creation(m_hWnd);


	// load icon
	m_hIcon = static_api_ptr_t<ui_control>()->get_main_icon();
	m_lookType = LookTypes::Normal;


	// create balloon tip
	CreateBalloonTip();


	return 0;
}

void DockWindow::OnDestroy()
{
	// writing window position to remember
	cfg_popup_window_placement.on_window_destruction(m_hWnd);
}

void DockWindow::OnLButtonDown(UINT nFlags, CPoint point)
{
	// for moving window (it's a lot simplier than handling mouse events)
	PostMessage(m_hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
	PostMessage(m_hWnd, WM_LBUTTONDOWN, 0, 0);
}

void DockWindow::OnLButtonDoubleClick(UINT nFlags, CPoint point)
{
	static_api_ptr_t<ui_control> uc;

	if (uc->is_visible())
		uc->hide();
	else
		uc->activate();
}

void DockWindow::OnMButtonDown()
{
	standard_commands::main_pause();
}

void DockWindow::OnContextMenu(HWND hWnd, CPoint point)
{
	static_api_ptr_t<playback_control> pc;
	static_api_ptr_t<ui_control> uc;
	static_api_ptr_t<playlist_manager> pm;
	service_ptr_t<contextmenu_manager> cmm;


	/* creating menu */

	enum {
		ID_COPY = 1,
		ID_SAC,//"Stop After Current"
		ID_STOP,
		ID_PAUSE,
		ID_PLAY,
		ID_PREV,
		ID_NEXT,
		ID_RAND,

		ID_CURRENT_PLAYLIST_FIRST,
		ID_CURRENT_PLAYLIST_LAST = ID_CURRENT_PLAYLIST_FIRST + 1000,

		ID_PLAYLIST_FIRST,
		ID_PLAYLIST_LAST = ID_PLAYLIST_FIRST + 1000,
		ID_PLAYLIST_CREATE_FILES,
		ID_PLAYLIST_CREATE_FOLDER,

		ID_ACTIVE_PLAYLIST_SHOW,
		ID_ACTIVE_PLAYLIST_REMOVE_DEAD,
		ID_ACTIVE_PLAYLIST_REMOVE_DUPLICATES,
		ID_ACTIVE_PLAYLIST_DESTROY,
		ID_ACTIVE_PLAYLIST_CREATE_FILES,
		ID_ACTIVE_PLAYLIST_CREATE_FOLDER,

		ID_ORDER_DEFAULT,
		ID_ORDER_REPEAT_PLAYLIST,
		ID_ORDER_REPEAT_TRACK,
		ID_ORDER_RANDOM,
		ID_ORDER_SHUFFLE_TRACKS,
		ID_ORDER_SHUFFLE_ALBUMS,
		ID_ORDER_SHUFFLE_FOLDERS,
		ID_QUEUE_FLUSH,

		ID_CONTEXT_FIRST,
		ID_CONTEXT_LAST = ID_CONTEXT_FIRST + 1000,

		ID_PREFERENCES,
		ID_HIDE,
		ID_EXIT
	};
	HMENU hMenu = CreatePopupMenu();
	HMENU hSubMenuNowPlaying, hSubMenuPlaylistsList, hSubMenuActivePlaylist, hSubMenuCurrentPlaylist, hSubMenuOrder;

	int flag, playlist_num;
	pfc::string_formatter text;
	pfc::string8 format;
	std::wstring wstr;
	bool tmpShow = false;


	// "Now Playing"
	contextmenu_manager::g_create(cmm);

	metadb_handle_list items;
	metadb_handle_ptr song;
	if (pc->get_now_playing(song))
		items.add_item(song);

	cmm->init_context_now_playing(0);

	if (cmm->get_root())
	{
		tmpShow = true;

		//submenu
		hSubMenuNowPlaying = CreatePopupMenu();
		cmm->win32_build_menu(hSubMenuNowPlaying, ID_CONTEXT_FIRST, ID_CONTEXT_LAST);

		//title for submenu
		if (song != NULL)
		{
			advconfig_now_playing_format.get(format);//"Now Playing: %title%";
			wstr = Playlist::GetPlayingTitle(format);
		}

		//adding submenu to main menu
		AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenuNowPlaying, wstr.c_str());
	

		//"Copy Title" option
		flag = MF_STRING;
		if (pc->get_stop_after_current())
			flag |= MF_CHECKED;

		AppendMenu(hMenu, MF_STRING, ID_COPY, TEXT("Copy Title"));
	}


	//"Current Playlist"
	if (advconfig_current_playlist_meaning || (!advconfig_current_playlist_meaning && pc->is_playing()))
	{
		cmm->init_context_playlist(0);

	
		hSubMenuCurrentPlaylist = CreatePopupMenu();

		//our options
		AppendMenu(hSubMenuCurrentPlaylist, MF_STRING, ID_ACTIVE_PLAYLIST_SHOW, TEXT("Show"));
		AppendMenu(hSubMenuCurrentPlaylist, MF_SEPARATOR, 0, 0);
		AppendMenu(hSubMenuCurrentPlaylist, MF_STRING, ID_ACTIVE_PLAYLIST_REMOVE_DEAD, TEXT("Remove Dead Entries"));
		AppendMenu(hSubMenuCurrentPlaylist, MF_STRING, ID_ACTIVE_PLAYLIST_REMOVE_DUPLICATES, TEXT("Remove Duplicates"));
		AppendMenu(hSubMenuCurrentPlaylist, MF_STRING, ID_ACTIVE_PLAYLIST_DESTROY, TEXT("Destroy Playlist"));
		AppendMenu(hSubMenuCurrentPlaylist, MF_SEPARATOR, 0, 0);
		AppendMenu(hSubMenuCurrentPlaylist, MF_STRING, ID_ACTIVE_PLAYLIST_CREATE_FILES, TEXT("Add Files..."));
		AppendMenu(hSubMenuCurrentPlaylist, MF_STRING, ID_ACTIVE_PLAYLIST_CREATE_FOLDER, TEXT("Add Folder..."));

		if (cmm->get_root())
		{
			AppendMenu(hSubMenuCurrentPlaylist, MF_SEPARATOR, 0, 0);

			//and also foobar's options
			cmm->win32_build_menu(hSubMenuCurrentPlaylist, ID_CURRENT_PLAYLIST_FIRST, ID_CURRENT_PLAYLIST_LAST);
		}
		
		// Which playlist is really active or currently playing (depending on setting)
		playlist_num = advconfig_current_playlist_meaning ? pm->get_active_playlist() : pm->get_playing_playlist();
		pm->playlist_get_name(playlist_num, text);

		wstr = L"Current Playlist: ";
		wstr += UtfConverter::FromUtf8(std::string(text.get_ptr()));
	
		AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenuCurrentPlaylist, wstr.c_str());
	}
		
	if (tmpShow)
	{
		//"Stop After Current"
		AppendMenu(hMenu, flag, ID_SAC, TEXT("Stop After Current"));
	}

	uAppendMenu(hMenu, MF_SEPARATOR, 0, 0);


	//playing options
	flag = MF_STRING;
	if (!pc->is_playing() && !pc->is_paused())
		flag |= MF_CHECKED;
	AppendMenu(hMenu, flag, ID_STOP, TEXT("Stop"));

	flag = MF_STRING;
	if (pc->is_paused())
		flag |= MF_CHECKED;
	AppendMenu(hMenu, flag, ID_PAUSE, TEXT("Pause"));

	flag = MF_STRING;
	if (pc->is_playing() && !pc->is_paused())
		flag |= MF_CHECKED;
	AppendMenu(hMenu, flag, ID_PLAY, TEXT("Play"));

	AppendMenu(hMenu, MF_STRING, ID_PREV, TEXT("Previous"));
	AppendMenu(hMenu, MF_STRING, ID_NEXT, TEXT("Next"));
	AppendMenu(hMenu, MF_STRING, ID_RAND, TEXT("Random"));


	// "Playlist"
	int num_playlists = pm->get_playlist_count();

	if (num_playlists)
	{
		hSubMenuPlaylistsList = CreatePopupMenu();

		// Create list of playlists
		for (int i = 0; i < num_playlists; i++)
		{
			pm->playlist_get_name(i, text);
			wstr = UtfConverter::FromUtf8(std::string(text.get_ptr()));

			// Create special menu for playing or just active playlist
			if (i == pm->get_active_playlist())
			{
				// active playlist settings options
				hSubMenuActivePlaylist = CreatePopupMenu();
				AppendMenu(hSubMenuActivePlaylist, MF_STRING, ID_ACTIVE_PLAYLIST_SHOW, TEXT("Show"));
				AppendMenu(hSubMenuActivePlaylist, MF_SEPARATOR, 0, 0);
				AppendMenu(hSubMenuActivePlaylist, MF_STRING, ID_ACTIVE_PLAYLIST_REMOVE_DEAD, TEXT("Remove Dead Entries"));
				AppendMenu(hSubMenuActivePlaylist, MF_STRING, ID_ACTIVE_PLAYLIST_REMOVE_DUPLICATES, TEXT("Remove Duplicates"));
				AppendMenu(hSubMenuActivePlaylist, MF_STRING, ID_ACTIVE_PLAYLIST_DESTROY, TEXT("Destroy Playlist"));
				AppendMenu(hSubMenuActivePlaylist, MF_SEPARATOR, 0, 0);
				AppendMenu(hSubMenuActivePlaylist, MF_STRING, ID_ACTIVE_PLAYLIST_CREATE_FILES, TEXT("Add Files..."));
				AppendMenu(hSubMenuActivePlaylist, MF_STRING, ID_ACTIVE_PLAYLIST_CREATE_FOLDER, TEXT("Add Folder..."));

				AppendMenu(hSubMenuPlaylistsList, MF_POPUP, (UINT_PTR)hSubMenuActivePlaylist, wstr.c_str());
			}

			//other playlist don't have special menu
			else
				AppendMenu(hSubMenuPlaylistsList, MF_STRING, ID_PLAYLIST_FIRST + i, wstr.c_str());
		}

		//option to create playlist and add files
		uAppendMenu(hSubMenuPlaylistsList, MF_SEPARATOR, 0, 0);
		AppendMenu(hSubMenuPlaylistsList, MF_STRING, ID_PLAYLIST_CREATE_FILES, TEXT("Create and Add Files..."));
		AppendMenu(hSubMenuPlaylistsList, MF_STRING, ID_PLAYLIST_CREATE_FOLDER, TEXT("Create and Add Folder..."));

		uAppendMenu(hMenu, MF_SEPARATOR, 0, 0);
		AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenuPlaylistsList, TEXT("Playlist"));
	}


	// "Order"
	hSubMenuOrder = CreatePopupMenu();

	// TODO these names could be got using pm->playback_order_get_guid()
	static const wchar_t* order_options[] = {
		L"Default",
		L"Repeat (playlist)",
		L"Repeat (track)",
		L"Random",
		L"Shuffle (tracks)",
		L"Shuffle (albums)",
		L"Shuffle (folders)"
	};

	for (int indexOption = 0; indexOption < 7; ++indexOption)
	{
		flag = MF_STRING;

		if (pm->playback_order_get_active() == indexOption)
			flag |= MF_CHECKED;

		AppendMenu(hSubMenuOrder, flag, indexOption + ID_ORDER_DEFAULT, order_options[indexOption]);
	}

	AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenuOrder, TEXT("Order"));


	// "Flush Queue"
	if (pm->queue_is_active())
		AppendMenu(hMenu, MF_STRING, ID_QUEUE_FLUSH, TEXT("Flush Queue"));


	uAppendMenu(hMenu, MF_SEPARATOR, 0, 0);


	// "Preferences"
	if (advconfig_show_preferences_option.get())
		AppendMenu(hMenu, MF_STRING, ID_PREFERENCES, TEXT("Preferences"));


	// "Hide"
	AppendMenu(hMenu, MF_STRING, ID_HIDE, TEXT("Hide"));


	if (advconfig_show_preferences_option.get())
		AppendMenu(hMenu, MF_SEPARATOR, 0, 0);


	// "Exit" option
	AppendMenu(hMenu, MF_STRING, ID_EXIT, TEXT("Exit"));


	/* actually showing created menu */
	CPoint pt;
	GetCursorPos(pt);

	menu_helpers::win32_auto_mnemonics(hMenu);

	int cmd = TrackPopupMenu(hMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hWnd, 0);



	/* checking what option was choosen */

	//"Copy Title" option
	if (cmd == ID_COPY)
	{
		advconfig_copy_title_format.get(format);
		text = Playlist::GetPlayingTitleA(format).c_str();


		if (::OpenClipboard(NULL))
		{
			::EmptyClipboard();
			HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (text.length()+1) * sizeof(TCHAR));
			
			if (hglbCopy != NULL)
			{
				LPTSTR lptstrCopy = (LPTSTR) GlobalLock(hglbCopy);
				wmemset(lptstrCopy, 0, text.length());
				memcpy(lptstrCopy, text.get_ptr(), text.length());
				lptstrCopy[text.length()] = 0;
				GlobalUnlock(hglbCopy);

				::SetClipboardData(CF_TEXT, hglbCopy);
			}

			::CloseClipboard();
		}
	}


	//"Stop After Current"
	else if(cmd == ID_SAC)
		standard_commands::main_stop_after_current();


	//"Stop"
	else if (cmd == ID_STOP)
		standard_commands::main_stop();


	//"Pause"
	else if (cmd == ID_PAUSE)
		standard_commands::main_pause();


	//"Play"
	else if (cmd == ID_PLAY)
		standard_commands::main_play();


	//"Previous"
	else if (cmd == ID_PREV)
		standard_commands::main_previous();


	//"Next"
	else if (cmd == ID_NEXT)
		standard_commands::main_next();


	//"Random"
	else if (cmd == ID_RAND)
		standard_commands::main_random();


	//"Playlist"
	else if (cmd >= ID_PLAYLIST_FIRST && cmd <= ID_PLAYLIST_LAST)
	{
		pm->set_active_playlist(cmd - ID_PLAYLIST_FIRST);

		// Autoplay
		if (advconfig_switched_playlist_changes_song.get())
		{
			pm->playlist_execute_default_action(cmd - ID_PLAYLIST_FIRST, 0);
			standard_commands::main_highlight_playing();
		}
	}


	// "Create and Add Files..."
	else if (cmd == ID_PLAYLIST_CREATE_FILES)
	{
		// Create Undo Backup
		if (advconfig_playlist_undo_backup.get())
			pm->playlist_undo_backup(playlist_num);


		// Save it to not minimize when it was activated
		flag = uc->is_visible();


		// Autoname playlist if option is set
//		if (!advconfig_autoname_playlist.get())
		{
			standard_commands::main_create_playlist();
			standard_commands::main_rename_playlist();
		}
		//else
		//{
		//	pm->create_playlist_autoname(pm->get_playlist_count());
		//}

		// the main part
		standard_commands::main_add_files();
		
		playlist_num = pm->get_playlist_count() - 1;
		pm->set_active_playlist(playlist_num);
		pm->playlist_execute_default_action(playlist_num, 0);

		pm->activeplaylist_ensure_visible(pm->activeplaylist_get_item_count() - 1);

		if (!flag)
			standard_commands::main_hide();

		// Autosave playlist if option is set
		if (advconfig_autosave_playlist.get())
			standard_commands::main_saveconfig();
	}


	// "Create and Add Folder..."
	else if (cmd == ID_PLAYLIST_CREATE_FOLDER)
	{
		// Create Undo Backup
		if (advconfig_playlist_undo_backup.get())
			pm->playlist_undo_backup(playlist_num);


		// Save it to not minimize when it was activated
		flag = uc->is_visible();


		// Indicates whether to add a folder
		tmpShow = true;

		// Autoname playlist if option is set
//		if (!advconfig_autoname_playlist.get())
		{
			standard_commands::main_create_playlist();
			tmpShow = standard_commands::main_rename_playlist();
		}
		//else
		//{
		//	pm->create_playlist_autoname(pm->get_playlist_count());
		//}
		
		// Only if user didn't Cancel creating a playlist
		if (tmpShow)
		{
			standard_commands::main_add_directory();
			
			playlist_num = pm->get_playlist_count() - 1;
			pm->set_active_playlist(playlist_num);
			pm->playlist_execute_default_action(playlist_num, 0);

			pm->activeplaylist_ensure_visible(pm->activeplaylist_get_item_count() - 1);

			if (!flag)
				standard_commands::main_hide();

			// Autosave playlist if option is set
			if (advconfig_autosave_playlist.get())
				standard_commands::main_saveconfig();
		}
	}


	// "Show" in active playlist
	else if (cmd == ID_ACTIVE_PLAYLIST_SHOW)
	{
		playlist_num = pm->get_active_playlist();

		pm->set_active_playlist(playlist_num);
		standard_commands::main_activate();

		if (pc->is_playing() || pc->is_paused())
			standard_commands::main_saveconfig();
	}


	//"Remove Dead Entries"
	else if (cmd == ID_ACTIVE_PLAYLIST_REMOVE_DEAD)
	{
		standard_commands::main_remove_dead_entries();

		// Autosave playlist if option is set
		if (advconfig_autosave_playlist.get())
			standard_commands::main_saveconfig();
	}


	//"Remove Duplicates"
	else if (cmd == ID_ACTIVE_PLAYLIST_REMOVE_DUPLICATES)
	{
		standard_commands::main_remove_duplicates();

		// Autosave playlist if option is set
		if (advconfig_autosave_playlist.get())
			standard_commands::main_saveconfig();
	}


	//"Destroy Playlist"
	else if (cmd == ID_ACTIVE_PLAYLIST_DESTROY)
	{
		playlist_num = pm->get_active_playlist();
		pm->remove_playlist_switch(playlist_num);

		if (advconfig_start_changed_playlist.get())
		{
			pm->set_playing_playlist(pm->get_active_playlist());
			pc->start();
		}
	}


	// "Add Files..." in active playlist
	else if (cmd == ID_ACTIVE_PLAYLIST_CREATE_FILES)
	{
		// Create Undo Backup
		if (advconfig_playlist_undo_backup.get())
			pm->playlist_undo_backup(playlist_num);

		// save it to not minimize when it was activated
		flag = uc->is_visible();

		// the main part
		playlist_num = pm->get_active_playlist();
		pm->set_active_playlist(playlist_num);
		standard_commands::main_add_files();

		pm->activeplaylist_ensure_visible(pm->activeplaylist_get_item_count() - 1);

		if (!flag)
			standard_commands::main_hide();

		// Autosave playlist if option is set
		if (advconfig_autosave_playlist.get())
			standard_commands::main_saveconfig();
	}


	// "Add Folder..." in active playlist
	else if (cmd == ID_ACTIVE_PLAYLIST_CREATE_FOLDER)
	{
		// Create Undo Backup
		if (advconfig_playlist_undo_backup.get())
			pm->playlist_undo_backup(playlist_num);

		// save it to not minimize when it was activated
		flag = uc->is_visible();

		// the main part
		playlist_num = pm->get_active_playlist();
		pm->set_active_playlist(playlist_num);
		standard_commands::main_add_directory();

		pm->activeplaylist_ensure_visible(pm->activeplaylist_get_item_count() - 1);

		if (!flag)
			standard_commands::main_hide();

		// Autosave playlist if option is set
		if (advconfig_autosave_playlist.get())
			standard_commands::main_saveconfig();
	}


	// "Order"
	else if (cmd >= ID_ORDER_DEFAULT && cmd <= ID_ORDER_SHUFFLE_FOLDERS)
	{
		pm->playback_order_set_active(cmd - ID_ORDER_DEFAULT);
	}


	// "Flush Queue"
	else if (cmd == ID_QUEUE_FLUSH)
		pm->queue_flush();


	//player's options
	else if (cmd >= ID_CONTEXT_FIRST && cmd <= ID_CONTEXT_LAST)
	{
		cmm->execute_by_id(cmd - ID_CONTEXT_FIRST);
	}

	//playlist options
	else if (cmd >= ID_CURRENT_PLAYLIST_FIRST && cmd <= ID_CURRENT_PLAYLIST_LAST)
	{
		cmm->execute_by_id(cmd - ID_CURRENT_PLAYLIST_FIRST);
	}


	//"Preferences" option
	else if (cmd == ID_PREFERENCES)
	{
		standard_commands::main_preferences();
	}


	//"Hide" option
	else if (cmd == ID_HIDE)
	{
		HideWindow();
	}


	//"Exit" option
	else if (cmd == ID_EXIT)
	{
		standard_commands::main_exit();
	}


	/* finally done */
	DestroyMenu(hSubMenuCurrentPlaylist);
	DestroyMenu(hSubMenuActivePlaylist);
	DestroyMenu(hSubMenuOrder);
	DestroyMenu(hSubMenuPlaylistsList);
	DestroyMenu(hSubMenuNowPlaying);
	DestroyMenu(hMenu);
}

void DockWindow::OnChangeLook()
{
	//TODO!
	//change FooDock's look between:
	// *normal
	// *cover
	// *visualizer
	// * maybe options with timer too ("time left")


}

void DockWindow::OnPaint(HDC hdc)
{
	CPaintDC dc(m_hWnd);
	PaintContent(dc.m_ps);
}

void DockWindow::OnPrintClient(HDC hdc, UINT uFlags)
{
	PAINTSTRUCT ps = { 0 };
	ps.hdc = hdc;
	GetClientRect(m_hWnd, &ps.rcPaint);
	ps.fErase = FALSE;
	PaintContent(ps);
}


/**
 * process_locations_notify_impl
 * This is service class which implements files and folders dragging behavior.
 *
 * See DockWindow::OnDragFiles() method (below).
 */
class process_locations_notify_impl : public process_locations_notify
{

public:

	process_locations_notify_impl(bool _play) : process_locations_notify()
	{
		m_play = _play;
	}

	void on_completion(const pfc::list_base_const_t<metadb_handle_ptr>& p_items)
	{
		static_api_ptr_t<playlist_manager> pm;
		int item_num;


		// Create Undo Backup
		if (advconfig_playlist_undo_backup.get() && pm->activeplaylist_get_item_count() > 0)
			pm->playlist_undo_backup(pm->get_active_playlist());

		// Add items
		item_num = pm->activeplaylist_get_item_count();
		pm->activeplaylist_clear_selection();
        pm->activeplaylist_add_items(p_items, bit_array_true());
		pm->activeplaylist_ensure_visible(item_num);

		// Autosave playlist if option is set
		if (advconfig_autosave_playlist.get())
			standard_commands::main_saveconfig();

		// Autoplay added songs
		if (m_play || advconfig_start_changed_playlist.get())
			pm->activeplaylist_execute_default_action(item_num);
	}

	void on_aborted()
	{
	}


private:

	bool m_play;

};

void DockWindow::OnDropFiles(HDROP hDrop)
{
	static_api_ptr_t<playlist_manager> pm;

	int bufSize, len;
	wchar_t* buf = NULL;
	bool play = false;
	
	pfc::list_t<const char*> list;


	// Indicate whether to start playing
	play = (GetKeyState(VK_SHIFT) & SHIFTED);

	//number of dropped files
	int numFiles = DragQueryFile(hDrop, 0xFFFFFFFF, 0, 0);


	// Creating a list
	for (int i = 0; i < numFiles; i++)
	{
		// Get the filename
		bufSize = DragQueryFile(hDrop, i, NULL, 0);
		buf = new wchar_t[bufSize+1];
		buf[bufSize] = 0;

		DragQueryFile(hDrop, i, buf, bufSize+1);

		// Converting utf16 to utf8
		std::string tmpstr = UtfConverter::ToUtf8(std::wstring(buf));
		
		len = tmpstr.length();
		char* str = new char[len+1];
		memcpy(str, tmpstr.c_str(), len);
		str[len] = 0;
		


		// We do not delete 'str' variable cause it's pointer will
		// be used inside that list.

		list.add_item(str);


		// (option) Dropped folder creates new playlist
		if (numFiles == 1 && advconfig_dropped_folder_new_playlist.get())
		{
			if (GetFileAttributes(buf) & FILE_ATTRIBUTE_DIRECTORY)
			{
				pm->create_playlist(
					(strrchr(str, '\\') + 1),//we just do not want a full path
					len,
					pm->get_playlist_count()
				);

				pm->set_active_playlist(pm->get_playlist_count() - 1);
			}
		}

		// But 'buf' isn't needed anymore
		delete[] buf;
	}
	

	// Scan all given files
	try
	{
		static_api_ptr_t<playlist_incoming_item_filter_v2> pif;
		service_ptr_t<process_locations_notify_impl> notify = new service_impl_t<process_locations_notify_impl>(play);

		pif->process_locations_async(list, playlist_incoming_item_filter_v2::op_flag_background,
			NULL, NULL, NULL, notify);
	} catch (...) {}


	// Now songs will be automatically added, when parsed,
	// by the process_locations_notify_impl service.


	// Delete dragging WinAPI object
	DragFinish(hDrop);
}

void DockWindow::OnMouseMove(UINT nFlags, CPoint point)
{
	// change transparency level (off it)
	SetLayeredWindowAttributes(m_hWnd, NULL, 255, LWA_ALPHA);
}

void DockWindow::OnMouseLeave()
{
	//Report("DockWindow::OnMouseLeave - transparency 85/255");

	// change transparency level
	SetLayeredWindowAttributes(m_hWnd, NULL, 85, LWA_ALPHA);
}

void DockWindow::OnMouseScroll(INT wheel_delta, UINT keys, CPoint point)
{
	static_api_ptr_t<playback_control> pc;
	static_api_ptr_t<playlist_manager> pm;

	// Indicate whether play the next song or previous
	bool scroll_up = advconfig_scroll_way.get() == (wheel_delta > 0);


	// If there is something not played, but paused, then unpause it
	if (pc->is_paused())
		pc->toggle_pause();

	// Otherwise old behaviour
	else
	{
		// If there is something played
		if (pc->is_playing())
		{
			// play previous song
			if (scroll_up)
				standard_commands::main_next();

			// play next song
			else
				standard_commands::main_previous();
		}

		// nothing is playing
		else
		{
			// if paused then unpause or play
			if (scroll_up)
				pc->start();

			// just start playing the previous song
			else
			{
				pc->start();
				standard_commands::main_previous();
			}
		}
	}
}

void DockWindow::OnBalloonTip(LPNMHDR info, LPNMTTDISPINFO lpnmtdi)
{
	//Report("OnBalloonTip");

	if (!advconfig_show_balloon_tip)
		return;

	static_api_ptr_t<playback_control> pc;
	std::wstring wstr;
	pfc::string8 format;
	metadb_handle_ptr pSong;

	// create text depending on player's playing state
	if (pc->is_playing() || pc->is_paused())
	{
		advconfig_balloon_tip_format.get(format);
		wstr = Playlist::GetPlayingTitle(format);
	}
	else
		wstr = L"Nothing is played right now.";

	// now change text for real
	delete[] this->m_wstrBalloonBuf;
	this->m_wstrBalloonBuf = new wchar_t[wstr.length()+1];
	lpnmtdi->lpszText = this->m_wstrBalloonBuf;
	for (volatile int i = 0; i < wstr.length(); ++i) lpnmtdi->lpszText[i] = wstr[i];
	lpnmtdi->lpszText[wstr.length()] = 0;

	lpnmtdi->hinst = NULL;
	lpnmtdi->uFlags = TTF_DI_SETITEM | TTF_ABSOLUTE;

	//SendMessage(info->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 200);
}

void DockWindow::TrackMouse()
{
	//Report("TrackMouse");

	TRACKMOUSEEVENT tme;

	tme.hwndTrack = m_hWnd; 
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE; //the main part

	m_mouseTracking = true;
	TrackMouseEvent(&tme);
}

void DockWindow::CreateBalloonTip()
{
	//Report("CreateBalloonTip");

	this->m_hwndBalloon = CreateWindowEx(
		NULL,
		TOOLTIPS_CLASS, BALLOON_WINDOWNAME,
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_BALLOON,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		m_hWnd,
		NULL,
		core_api::get_my_instance(),
		NULL);

	HWND hwndTT = this->m_hwndBalloon;

	if (!hwndTT)
		return;

	m_wstrBalloonBuf = NULL;

	SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	TOOLINFO ti;
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS |  TTF_IDISHWND;
	ti.hwnd = m_hWnd;
	ti.hinst = NULL;
	ti.uId = (UINT_PTR)m_hWnd;
	ti.lpszText = LPSTR_TEXTCALLBACK;

	RECT rect;
	GetClientRect(m_hWnd, &rect);

	ti.rect.left = rect.left;    
	ti.rect.top = rect.top;
	ti.rect.right = rect.right;
	ti.rect.bottom = rect.bottom;


	SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)&ti);


	new play_callback_balloon();
}

void DockWindow::RefreshShowedBalloonTip()
{
	HWND hWnd = g_instance.m_hwndBalloon;

	if (advconfig_show_balloon_tip.get())
	{
		static_api_ptr_t<playback_control> pc;
		std::wstring wstr;
		pfc::string8 format;
		metadb_handle_ptr pSong;

		// create text depending on player's playing state
		if (pc->is_playing() || pc->is_paused())
		{
			advconfig_balloon_tip_format.get(format);
			wstr = Playlist::GetPlayingTitle(format);
		}
		else
			wstr = L"Nothing is played right now.";

		// now change text for real
		delete[] g_instance.m_wstrBalloonBuf;
		g_instance.m_wstrBalloonBuf = new wchar_t[wstr.length()+1];
		for (volatile int i = 0; i < wstr.length(); ++i) g_instance.m_wstrBalloonBuf[i] = wstr[i];
		g_instance.m_wstrBalloonBuf[wstr.length()] = 0;


		TOOLINFO ti;
		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = TTF_SUBCLASS |  TTF_IDISHWND;
		ti.hwnd = g_instance.m_hWnd;
		ti.hinst = NULL;
		ti.uId = (UINT_PTR)g_instance.m_hWnd;
		ti.lpszText = g_instance.m_wstrBalloonBuf;

		RECT rect;
		GetClientRect(g_instance.m_hWnd, &rect);

		ti.rect.left = rect.left;    
		ti.rect.top = rect.top;
		ti.rect.right = rect.right;
		ti.rect.bottom = rect.bottom;

		::SendMessage(hWnd, TTM_UPDATETIPTEXT, NULL, (LPARAM)(LPTOOLINFO)&ti);
		::UpdateWindow(hWnd);
	}
}

void DockWindow::PaintContent(PAINTSTRUCT &ps)
{
	if (GetSystemMetrics(SM_REMOTESESSION))
	{
		// Do not use double buffering, if we are running on a Remote Desktop Connection.
		// The system would have to transfer a bitmap everytime our window is painted.
		Draw(ps.hdc, ps.rcPaint);
	}
	else if (!IsRectEmpty(&ps.rcPaint))
	{
		// Use double buffering for local drawing.
		CMemoryDC dc(ps.hdc, ps.rcPaint);
		Draw(dc, ps.rcPaint);
	}
}

void DockWindow::Draw(HDC hdc, CRect rcPaint)
{
	// We will paint the background in the default window color.
	/*HBRUSH hBrush = GetSysColorBrush(COLOR_WINDOW);
	FillRect(hdc, rcPaint, hBrush);*/


	DrawIconEx(hdc, 2, 2, m_hIcon, 32, 32, 0, /*hBrush*/NULL, DI_NORMAL);
}



/********************************************************************************* /
/*    class play_callback_balloon implementation                                   /
/**********************************************************************************/

void play_callback_balloon::on_playback_new_track(metadb_handle_ptr p_track)
{
	DockWindow::RefreshShowedBalloonTip();
}

void play_callback_balloon::on_playback_stop(play_control::t_stop_reason p_reason)
{
	if (p_reason != play_control::stop_reason_starting_another)
		DockWindow::RefreshShowedBalloonTip();
}
