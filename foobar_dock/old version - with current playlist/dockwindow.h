#ifndef __DOCK_WINDOW__H
#define __DOCK_WINDOW__H

#include "foo_dock.h"


class play_callback_balloon : public play_callback_impl_base
{

public:

	play_callback_balloon() :
		play_callback_impl_base(flag_on_playback_new_track | flag_on_playback_stop)
	{}

	void on_playback_new_track(metadb_handle_ptr p_track);
	void on_playback_stop(play_control::t_stop_reason p_reason);
};

class DockWindow : public CSimpleWindowImpl<DockWindow>
{

public:

	typedef CSimpleWindowImpl<DockWindow> super;

	static void ShowWindow();
	static void HideWindow();
	static HWND getHandle();
	static void DestroyWindow();

	BOOL ProcessWindowMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result);

	// handles
	LRESULT OnCreate(LPCREATESTRUCT pCreateStruct);
	void OnDestroy();
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnLButtonDoubleClick(UINT nFlags, CPoint point);
	void OnMButtonDown();
	void OnContextMenu(HWND hWnd, CPoint point);
	void OnChangeLook();
	void OnPaint(HDC hdc);
	void OnPrintClient(HDC hdc, UINT uFlags);
	void OnDropFiles(HDROP hDrop);
	void OnMouseMove(UINT nFlags, CPoint point);
	void OnMouseLeave();
	void OnMouseScroll(INT wheel_delta, UINT keys, CPoint point);
	void OnBalloonTip(LPNMHDR info, LPNMTTDISPINFO lpnmtdi);

	void TrackMouse();
	void CreateBalloonTip();
	static void RefreshShowedBalloonTip();

	void PaintContent(PAINTSTRUCT &ps);
	void Draw(HDC hdc, CRect rcPaint);

	HWND Create();
	inline void RedrawWindow() {::RedrawWindow(m_hWnd, 0, 0, RDW_INVALIDATE);}


private:

	HICON m_hIcon;
	wchar_t* m_wstrBalloonBuf;
	HWND m_hwndBalloon;
	bool m_mouseTracking;


private:

	// This is a singleton class.
	DockWindow() {}
	~DockWindow() {delete[] m_wstrBalloonBuf;}

	static DockWindow g_instance;


private:

	enum LookTypes {
		Normal,
		Cover,
		Visualizer
	};
	
	LookTypes m_lookType;

};


#endif
