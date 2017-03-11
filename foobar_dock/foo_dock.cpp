#include "foo_dock.h"
#include "playlist.h"
#include "dockwindow.h"

#ifdef _DEBUG
	#include "vld.h" //for detecting memory leaks
#endif


DECLARE_COMPONENT_VERSION(
	"FooDock",
	"0.2.6.0",
	"FooDock's functions:\n"
	" * add files to the current playlist by dropping files (try to drop folders!)\n"
	" * dropping with SHIFT starts playing recently added files\n"
	" * right click has some useful options like tray icon and more (try while playing)\n"
	" * double click shows/hides foobar window\n"
	" * scrolling over dock changes song\n"
	" * balloon tip showing currently played song\n"
	" * middle click (scroll click) works as pause\n"
	" * Flush Queue option appears when there is something in queue\n"
	"\n"
	"author:\n"
	" Kamil \"nameczanin\" Dabrowski aka Namek\n"
	" http://namek.eu/code/foo_dock\n"
);



/********************************************************************************* /
/*    Main implementation                                                         /
/********************************************************************************/


class foo_dock : public initquit
{
	virtual void on_init()
	{
		DockWindow::ShowWindow();
	}

	virtual void on_quit()
	{
		DockWindow::DestroyWindow();
	}
};

initquit_factory_t<foo_dock> foo_initquit;
