// File:		winproc.cpp
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com
// This was taken verbatim from two-kings site, and modified (active state, etc)

#include	"main.h"

/************************************************************************/
/* Name:		WindowProcedure											*/
/* Description:	handles windows messages								*/
/************************************************************************/

LRESULT CALLBACK WindowProcedure(HWND hWindow,UINT uMessage,WPARAM wparam,LPARAM lparam)
{
	switch(uMessage)
	{
	case WM_ACTIVATE: // WM_ACTIVE is sent when the window is activated or deactivated. F. Luna's code!
		if( LOWORD(wparam) == WA_INACTIVE )
			g_app->m_bAppPaused = 1;
		else
			g_app->m_bAppPaused = 0;
		return 0;

	case WM_COMMAND: //user command on window
		{
			switch(LOWORD(wparam)) // wparam = type of the command message, LOWORD, just gets lower bits
			{
				//start button pressed
			case ID_START:
				{
					//switch to D3D loop and init D3D with settings
					g_app->SaveSettings();
					g_app->m_bRunningWindow = 0; // Kill the config window main loop
					g_app->m_bRunningD3D = 1; // Activate D3D main loop.
					SetFocus(hWindow); // switch to D3D window
					break;
				}
				//cancel button pressed
			case ID_CANCEL:
				{
					DestroyWindow(hWindow);
					break;
				}
			}
			return 0;
		}
	case WM_CLOSE:
		{
		DestroyWindow(hWindow);
		return 0;
		}
	case WM_KEYDOWN:
		{
			switch(wparam)
			{
			case VK_ESCAPE:	
				{
					DestroyWindow(hWindow);
					break;
				}
			case VK_F1:
				g_app->toggleFullScreenMode();
			}
			return 0;
		}
	case WM_DESTROY:
		{
			g_app->m_bRunningWindow = 0; // stop the config window main loop
			g_app->m_bRunningD3D = 0; // stop D3D main loop
			break;
		}
	}

	return DefWindowProc(hWindow,uMessage,wparam,lparam); // respond to all other messages
}//WindowProcedure