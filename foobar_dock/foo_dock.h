#ifndef __FOO_DOCK__H
#define __FOO_DOCK__H


/************************************************/
/*  Includes                                   */
/**********************************************/
//#include "targetver.h"
#include <windows.h>
#include <windowsx.h>
#include <string>
#include <tchar.h>
#include <strsafe.h>
#include <shellapi.h>
#include <memory>
#include <SDK/foobar2000.h>
#include <helpers/helpers.h>
#include <ATLHelpers/ATLHelpers.h>
#include "window_helper.h"
#include "UtfConverter.h"


/************************************************/
/*  Some  settings                             */
/**********************************************/
	#define MYWNDCLASS L"NamekFooDock"
	#define DOCK_SIZE 40
	#define SHIFTED 0x8000
	#define FOOBAR_CLASS L"{97E27FAA-C0B3-4b8e-A693-ED7881E99FC1}"
	#define POS_X 600
	#define POS_Y 120
	#define BALLOON_WINDOWNAME L"FooDock_Balloon"


/************************************************/
/*  Functions                                  */
/**********************************************/
	LRESULT CALLBACK MessageWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void ShowLastError(LPTSTR lpszFunction);
	GUID GetFooDockGUID(int seed);


#endif
