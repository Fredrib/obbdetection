/*************************************************************
**			Jonathan Tompson Application Interface			**
**   -> Windowing and DirectX initialization, Summer 2009	**
*************************************************************/
// File:		app.cpp
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com
//
// Acknowledgments: I loved the object formatting of two-kings code so I used this
// as a starting point.  Many references don't suggest good methods of
// organizing game code... I can only assume this is a good approach.
//
// I also modified some F. Luna's code and used it here... But I imageine that's what
// the two-kings guys did as well.

#include	"main.h"

/************************************************************************/
/* Name:		app														*/
/* Description:	define member variables and calls init functions		*/
/************************************************************************/
app::app(void)
{
	//define member variables
	m_pD3DObj = NULL;
	m_pD3DDev = NULL;

	// Clear error codes
	m_strError.clear();
	
	// Failsafe settings --> They will normally be fetched from config file
	m_dwWidth = 800;						// Default Resolution 800 x 600
	m_dwHeight = 600;
	m_ColorFormat = D3DFMT_R5G6B5;
	m_DepthStencilFormat = D3DFMT_D16; 
	m_dwVertexProcessing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	m_MultiSampling = D3DMULTISAMPLE_NONE;
	m_dwAnisotropy = 0;
	m_FullScreen = 1;
	m_Shadows = 1;
	m_RebuildObb = 0;
	m_ObbRenderDepth = 4;
	m_ObbDebugMode = 0;

	m_bRunningWindow = true;
	m_bAppPaused = false;
	m_bRunningD3D = 0;

	// UI and debug settings
	m_drawConvexHull = 0;
	m_pausePhysics = 0;
	m_EnterObbDebugMode = 0;
	m_ObbDebugModeNext = 0;
	m_drawOBBTree = 0;
	m_drawModels = 1;

	// Some time keeping variables
	m_cntStart = 0;
	m_alphaTime = 0.0f;
	m_deltaTime = 0.0f;
	m_accumulator = 0.0f;
	m_currentTime = 0.0f;
	m_currentTimePhysics = 0.0f;

	m_DeviceCaps = new (D3DCAPS9);
	m_ObjectCaps = new (D3DCAPS9);
}//app
void app::InitApp(void)
{
	g_app = new app;
	g_renderer = new renderer;
	g_physics = new physics;
	g_camera = new camera;		
	g_keyboard = new keyboard;
	g_mouse = new mouse;
	g_sky =  new sky;
	g_hud = new hud;

	g_light = new light[NO_LIGHTS];
	g_rbobjects = new rbobject*[NO_RBOBJECTS]; // array of rigid body object pointers
	for(int i = 0; i < NO_RBOMESHES; i ++)
		g_rbobjects[i] = new rboMesh;

	//create window
	g_app->InitWindow();
}
/************************************************************************/
/* Name:		~app													*/
/* Description:	Default Destructor										*/
/************************************************************************/
app::~app(void)
{
	delete m_DeviceCaps; delete m_ObjectCaps; 
	DestroyAllVertexDeclarations();
	KillObjects();
	KillD3D();
	KillWindow();
}//~app
void app::KillApp(void)
{
	delete [] g_light;
	for(int i = 0; i < NO_RBOBJECTS; i++)
	{
		if(g_rbobjects[i])
		{
			delete g_rbobjects[i];
			g_rbobjects[i] = NULL;
		}
	}
	delete [] g_rbobjects;
	delete g_renderer;
	delete g_physics;
	delete g_camera;
	delete g_keyboard;
	delete g_mouse;
	delete g_sky;
	delete g_app;
	delete g_hud;
}
/************************************************************************/
/* Name:		InitWindow												*/
/* Description:	create	window class, window and child elements			*/
/************************************************************************/
void app::InitWindow(void)
{
	WNDCLASSEX WindowClass; // Pop-up settings window
	RECT DesktopSize;
	ZeroMemory(&WindowClass, sizeof(WNDCLASSEX)); // clear out the window class for use

	InitCommonControls(); // Enable windows Styles

	GetClientRect(GetDesktopWindow(),&DesktopSize); //Get desktop resolution

	//fill window class and register class
	WindowClass.cbSize = sizeof(WNDCLASSEX); // Allows future Windows OS to resize struct. (FAILS IF NOT SET)
	WindowClass.style = CS_HREDRAW | CS_VREDRAW; // redraw on horizontal and vertical resizing
	WindowClass.lpfnWndProc = WindowProcedure; // Message dispatch function to call (must be global or static.  Never cast function pointer.
	WindowClass.cbClsExtra = 0;
	WindowClass.cbWndExtra = 0;
	WindowClass.hInstance = GetModuleHandle(NULL); // handle to win32 application
	WindowClass.hIcon = NULL;
	WindowClass.hIconSm = WindowClass.hIcon;
	WindowClass.hCursor = NULL; // mouse image - LoadCursor(NULL, IDC_ARROW); is default
	WindowClass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	WindowClass.lpszMenuName = NULL;
	WindowClass.lpszClassName = L"ClassName";

	WindowClass.hIconSm = LoadIcon(NULL,IDI_APPLICATION);

	if(!RegisterClassEx(&WindowClass)) // register the window class
	{
		m_strError = L"RegisterClassEx() failed. Error: " + util::Win32ErrorToString(GetLastError());
		m_bRunningWindow = 0;
		return;
	}

	// Get the window size for the requested client area size
	DWORD dwStyle = (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_THICKFRAME;
	SIZE sizeWindow;
	RECT rc;
	rc.top = rc.left = 0; rc.right = SETUPWINDOW_X; rc.bottom = SETUPWINDOW_Y;
	AdjustWindowRect(&rc, dwStyle, 0);
	sizeWindow.cx = rc.right - rc.left;
	sizeWindow.cy = rc.bottom - rc.top;

	m_hWindow = CreateWindowExA(WS_EX_CONTROLPARENT, // Extension to dwStyle (4th parameter) for more window options
		(LPCSTR)"ClassName", // Class name
		(LPCSTR)TITLE, // Window Title - from main.h
		dwStyle, // window style --> Can take out minimize, etc
		(DesktopSize.right - SETUPWINDOW_X) / 2, (DesktopSize.bottom - SETUPWINDOW_Y) / 2, // X & Y Position of the window (upper left corner)
		sizeWindow.cx,sizeWindow.cy, // width and hight of window
		NULL,NULL, // Parent window and menus
		GetModuleHandle(NULL), // Application handle
		NULL); // Used for multiple windows
	if(!m_hWindow)
	{
		m_strError = L"CreateWindowEx() failed. Error: " + util::Win32ErrorToString(GetLastError());
		UnregisterClass(WindowClass.lpszClassName, GetModuleHandle(NULL));
		m_bRunningWindow = 0;
		return;
	}

	AddButtons(); // Add all the config buttons

	//load settings from file
	LoadSettings();
}//InitWindow

/************************************************************************/
/* Name:		InitD3D													*/
/* Description:	create Direct3D object and device						*/
/************************************************************************/
void app::InitD3D(void)
{
	RemoveButtons(); // Remove all the config buttons

	//create Direct3D interface object, D3D_SDK_VERSION V9c = 32 
	if((m_pD3DObj = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
	{
		m_strError = L"Direct3DCreate9() failed. Error: " + util::DXErrorToString(GetLastError());
		m_bRunningD3D = 0;
		return;
	}

	// check that we can run the desired settings
	if(!CheckObjCaps())
	{
		m_bRunningD3D = 0; // Stop everything!
		return;
	}

	// Change window style to remove border in window mode
	DWORD dwStyle;
	if(!m_FullScreen)
	{
		dwStyle = (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_THICKFRAME;
		SetWindowLongPtr(m_hWindow, GWL_STYLE, dwStyle);
		// Make sure the changes are flushed and change window size anyway
		RECT DesktopSize; SIZE sizeWindow; RECT rc;
		rc.top = rc.left = 0; rc.right = m_dwWidth; rc.bottom = m_dwHeight;
		AdjustWindowRect(&rc, dwStyle, 0); // in window mode the border can hide some pixels
		sizeWindow.cx = rc.right - rc.left;
		sizeWindow.cy = rc.bottom - rc.top;
		GetClientRect(GetDesktopWindow(),&DesktopSize); //Get desktop resolution
		if(!SetWindowPos(m_hWindow, NULL, (DesktopSize.right - m_dwWidth) / 2, (DesktopSize.bottom - m_dwHeight) / 2, sizeWindow.cx, 
			sizeWindow.cy, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW))
		{
			m_strError = L"Failed to resize window!!";
			m_bRunningD3D = 0; // Stop everything!
			return;
		}
	}
	else
	{
		// Change the window style to a more fullscreen friendly style.
		SetWindowLongPtr(m_hWindow, GWL_STYLE, WS_POPUP);
		if(!SetWindowPos(m_hWindow, NULL, 0, 0, m_dwWidth, m_dwHeight, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW))
		{
			m_strError = L"Failed to resize window!!";
			m_bRunningD3D = 0; // Stop everything!
			return;
		}
	}

	// TEMP CODE!!!
	if(m_DepthStencilFormat != D3DFMT_D24S8)
	{
		m_strError = L"Only Depth Stencil Format D3DFMT_D24S8 is supported for now!!";
		m_bRunningD3D = 0; // Stop everything!
		return;
	}

	// store device (hardware) information
	ZeroMemory(&m_PresentParameters,sizeof(m_PresentParameters));
	m_PresentParameters.Windowed					= !m_FullScreen; // Program not windowed
	m_PresentParameters.SwapEffect					= D3DSWAPEFFECT_DISCARD; // Type of swap chain (for back buffers), discard old frame.
	m_PresentParameters.EnableAutoDepthStencil		= true; // Turn on and initialize the z-buffer
	m_PresentParameters.AutoDepthStencilFormat		= m_DepthStencilFormat; // # bits in z-buffer
	m_PresentParameters.hDeviceWindow				= m_hWindow; // the window to be used by D3D
	m_PresentParameters.BackBufferWidth				= m_dwWidth; // BB width
	m_PresentParameters.BackBufferHeight			= m_dwHeight; // BB hight
	m_PresentParameters.BackBufferFormat			= m_ColorFormat; // color depth --> F. Luna uses D3DFMT_UNKNOWN
	m_PresentParameters.BackBufferCount				= 1;
	m_PresentParameters.MultiSampleType				= m_MultiSampling; // 0-4 multi-samples (anti-aliasing)
	m_PresentParameters.MultiSampleQuality			= 0; // NOT SURE IF YOU NEED THIS!!
	m_PresentParameters.FullScreen_RefreshRateInHz	= D3DPRESENT_RATE_DEFAULT;
	m_PresentParameters.PresentationInterval		= D3DPRESENT_INTERVAL_IMMEDIATE;
	m_PresentParameters.Flags						= 0;

	if(FAILED(m_pD3DObj->CreateDevice(D3DADAPTER_DEFAULT, // Which video card to use
											  D3DDEVTYPE_HAL, // DeviceType: _HAL = hardware abstraction layer (may use software if no hardware support).
											  m_hWindow,
											  m_dwVertexProcessing, // Where 3D calculations are done (software, etc)
											  &m_PresentParameters, // our desired parameters
											  &m_pD3DDev))) // graphics device interface
	{
		std::wstring error = L"CreateDevice() failed: " + util::DXErrorToString(GetLastError());
		MessageBox(m_hWindow,error.c_str(),L"InitD3D()",MB_OK);
		m_bRunningD3D = 0; m_pD3DObj->Release(); m_pD3DObj = NULL;
		return;
	}

	if(!CheckDevCaps())
	{
		m_bRunningD3D = 0; m_pD3DObj->Release(); m_pD3DObj = NULL;
		return;
	}

	if(m_FullScreen)
		ShowCursor(0); // deactivate the cursor in fullscreen

	g_app->m_bAppPaused = 0;
	
}//InitD3D

/************************************************************************/
/* Name:		CheckObjCaps & CheckDevCaps								*/
/* Description:	check for necessary device caps							*/
/* --> If we're full screen, we can choose whatever as long as card		*/
/*     supports what we want.  If windowed we're need to check for 		*/
/*     format conversion (F. Luna, pg 84)								*/
/************************************************************************/
bool app::CheckObjCaps(void)
{
	D3DFORMAT fmtBackbuffer;
	D3DDISPLAYMODE mode;
	HRESULT hResult;

	if(!m_FullScreen) 
	{
		// Grab the current desktop format
		hResult = m_pD3DObj->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);
		if(FAILED(hResult))
		{
			m_strError = L"GetAdapterDisplayMode() failed. Error: " + util::DXErrorToString(hResult);
			return 0;
		}
		// Check for conversion between desired backbuffer and desktop
		hResult = m_pD3DObj->CheckDeviceFormatConversion(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_ColorFormat, mode.Format);
		if(FAILED(hResult))
		{
			MessageBox(NULL, L"Conversion: backbuffer format-->desktop format not supported.  Setting to current desktop format.", L"Warning", MB_OK);
			m_ColorFormat = mode.Format; // Change to the desktop color format!!
		}
		fmtBackbuffer = m_ColorFormat;
	} else {
		// Make up the format we want
		fmtBackbuffer = m_ColorFormat;
		mode.Format = m_ColorFormat;
		mode.Height = m_dwHeight;
		mode.Width = m_dwWidth; 
		mode.RefreshRate = 0; // Adapter default
	}

	// Need to see if this format is ok as a backbuffer format in this adapter mode
	hResult = m_pD3DObj->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, mode.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, fmtBackbuffer);
	if(FAILED(hResult))
	{
		m_strError = L"Unable to initialize your chosen backbuffer format!";
		return 0;
	}

	hResult = m_pD3DObj->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, mode.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, m_DepthStencilFormat);
	if(FAILED(hResult))
	{
		m_strError = L"Unable to initialize your chosen Depth Stencil format!";
		return 0;
	}

	DWORD retQualityLevels;
	hResult = m_pD3DObj->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, mode.Format, !m_FullScreen, m_MultiSampling, &retQualityLevels);
	if(FAILED(hResult))
	{
		m_strError = L"Unable to initialize your chosen Multi Sampling Format!";
		return 0;
	}

	// Get capabilities for this device --> HANDY TO HAVE
	hResult = m_pD3DObj->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_ObjectCaps);
	if(FAILED(hResult))
	{
		m_strError = L"GetDeviceCaps() failed. Error: " + util::DXErrorToString(hResult);
		return 0;
	}

	// check for hardware vertex processing if we've chosen it.
	if(m_dwVertexProcessing == D3DCREATE_HARDWARE_VERTEXPROCESSING)
	{
		if(!(m_ObjectCaps->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT))
		{
			m_strError = L"Device does not support hardware vertex processing!";
			return 0;
		}
	}

	// If pure device and HW T&L supported --> Make a pure device
	if( m_ObjectCaps->DevCaps & D3DDEVCAPS_PUREDEVICE &&
		m_dwVertexProcessing & D3DCREATE_HARDWARE_VERTEXPROCESSING)
			m_dwVertexProcessing |= D3DCREATE_PUREDEVICE;

	return 1; // Everything is OK with the settings!
}//CheckDeviceCaps

bool app::CheckDevCaps(void)
{
	// Get capabilities for this device --> HANDY TO HAVE
	HRESULT hResult = m_pD3DDev->GetDeviceCaps(m_DeviceCaps);
	if(FAILED(hResult))
	{
		m_strError = L"GetDeviceCaps() failed. Error: " + util::DXErrorToString(hResult);
		return 0;
	}

	// Check for vertex shader version 2.0 support.
	if( (m_DeviceCaps->VertexShaderVersion < D3DVS_VERSION(3, 0)) || (m_DeviceCaps->PixelShaderVersion < D3DPS_VERSION(3, 0)))
	{
		if( (m_DeviceCaps->VertexShaderVersion < D3DVS_VERSION(2, 0)) || (m_DeviceCaps->PixelShaderVersion < D3DPS_VERSION(2, 0)) )
		{
			m_strError = L"GetDeviceCaps() failed. No support for vertex shader 3.0 or 2.0.  Try on a better computer.";
			return 0;
		}
		else
		{
#ifndef _DEBUG
			MessageBox(NULL, L"Shader Model 3.0 not supported, switching to shader model 2.0 and disabling anti-aliasing of shadow map", L"Warning", MB_OK);
#endif
			g_renderer->m_shaderVersion = 2;
		}
	} else
		g_renderer->m_shaderVersion = 3;

	return 1; // Everything is OK with the settings!
}

/************************************************************************/
/* Name:		LoadSettings and SaveSettings							*/
/* Description:	load settings and fill combo boxes						*/
/************************************************************************/
void app::LoadSettings(void)
{
	ifstream File("settings.cfg");
	int iColorFormat,iDepthStencilFormat,iMultiSampling;

	//load settings from file
	File >> m_dwWidth;
	File >> m_dwHeight;
	File >> iColorFormat;
	File >> iDepthStencilFormat;
	File >> m_dwVertexProcessing;
	File >> iMultiSampling;
	File >> m_dwAnisotropy;
	File >> m_FullScreen;
	File >> m_Shadows;
	File >> m_RebuildObb;
	File >> m_ObbRenderDepth;
	File >> m_ObbDebugMode;

	//fill D3DENUMERATED_TYPES
	switch(iColorFormat)
	{
	case D3DFMT_R5G6B5: m_ColorFormat = D3DFMT_R5G6B5; break;
	case D3DFMT_A1R5G5B5: m_ColorFormat = D3DFMT_A1R5G5B5; break;
	case D3DFMT_X8R8G8B8: m_ColorFormat = D3DFMT_X8R8G8B8; break;
	case D3DFMT_A8R8G8B8: m_ColorFormat = D3DFMT_A8R8G8B8; break;
	default: m_ColorFormat = D3DFMT_X8R8G8B8; break;
	}

	switch(iDepthStencilFormat)
	{
	case D3DFMT_D16: m_DepthStencilFormat = D3DFMT_D16; break;
	case D3DFMT_D24S8: m_DepthStencilFormat = D3DFMT_D24S8; break;
	case D3DFMT_D32: m_DepthStencilFormat = D3DFMT_D32; break;
	default: m_DepthStencilFormat = D3DFMT_D24S8; break;
	}

	switch(iMultiSampling)
	{
	case D3DMULTISAMPLE_NONE: m_MultiSampling = D3DMULTISAMPLE_NONE; break;
	case D3DMULTISAMPLE_2_SAMPLES: m_MultiSampling = D3DMULTISAMPLE_2_SAMPLES; break;
	case D3DMULTISAMPLE_4_SAMPLES: m_MultiSampling = D3DMULTISAMPLE_4_SAMPLES; break;
	default: m_MultiSampling = D3DMULTISAMPLE_NONE; break;
	}

	// select loaded settings in combo boxes
	switch(m_dwHeight)
	{
	case 480: SendMessageA(m_hCbResolution,CB_SETCURSEL,0,0); break;
	case 600: SendMessageA(m_hCbResolution,CB_SETCURSEL,1,0); break;
	case 768: SendMessageA(m_hCbResolution,CB_SETCURSEL,2,0); break;
	case 864: SendMessageA(m_hCbResolution,CB_SETCURSEL,3,0); break;
	case 960: SendMessageA(m_hCbResolution,CB_SETCURSEL,4,0); break;
	case 1024: SendMessageA(m_hCbResolution,CB_SETCURSEL,5,0); break;
	case 1200: SendMessageA(m_hCbResolution,CB_SETCURSEL,6,0); break;
	case 720: SendMessageA(m_hCbResolution,CB_SETCURSEL,7,0); break;
	default: SendMessageA(m_hCbResolution,CB_SETCURSEL,1,0); break;
	}

	switch(m_ColorFormat)
	{
	case D3DFMT_R5G6B5: SendMessageA(m_hCbBackBuffer,CB_SETCURSEL,0,0); break;
	case D3DFMT_A1R5G5B5: SendMessageA(m_hCbBackBuffer,CB_SETCURSEL,1,0); break;
	case D3DFMT_X8R8G8B8: SendMessageA(m_hCbBackBuffer,CB_SETCURSEL,2,0); break;
	case D3DFMT_A8R8G8B8: SendMessageA(m_hCbBackBuffer,CB_SETCURSEL,3,0); break;
	default: SendMessageA(m_hCbBackBuffer,CB_SETCURSEL,2,0); break;
	}

	switch(m_DepthStencilFormat)
	{
	case D3DFMT_D16: SendMessageA(m_hCbDepthStencil,CB_SETCURSEL,0,0); break;
	case D3DFMT_D24S8: SendMessageA(m_hCbDepthStencil,CB_SETCURSEL,1,0); break;
	case D3DFMT_D32: SendMessageA(m_hCbDepthStencil,CB_SETCURSEL,2,0); break;
	default: SendMessageA(m_hCbDepthStencil,CB_SETCURSEL,0,0); break;
	}

	switch(m_dwVertexProcessing)
	{
	case D3DCREATE_SOFTWARE_VERTEXPROCESSING: SendMessageA(m_hCbVertexProcessing,CB_SETCURSEL,0,0); break;
	case D3DCREATE_HARDWARE_VERTEXPROCESSING: SendMessageA(m_hCbVertexProcessing,CB_SETCURSEL,1,0); break;
	}

	switch(m_MultiSampling)
	{
	case D3DMULTISAMPLE_NONE: SendMessageA(m_hCbMultiSampling,CB_SETCURSEL,0,0); break;
	case D3DMULTISAMPLE_2_SAMPLES: SendMessageA(m_hCbMultiSampling,CB_SETCURSEL,1,0); break;
	case D3DMULTISAMPLE_4_SAMPLES: SendMessageA(m_hCbMultiSampling,CB_SETCURSEL,2,0); break;
	default: SendMessageA(m_hCbMultiSampling,CB_SETCURSEL,0,0); break;
	}

	switch(m_dwAnisotropy)
	{
	case 0: SendMessageA(m_hCbAnisotropy,CB_SETCURSEL,0,0); break;
	case 2: SendMessageA(m_hCbAnisotropy,CB_SETCURSEL,1,0); break;
	case 4: SendMessageA(m_hCbAnisotropy,CB_SETCURSEL,2,0); break;
	case 8: SendMessageA(m_hCbAnisotropy,CB_SETCURSEL,3,0); break;
	case 16: SendMessageA(m_hCbAnisotropy,CB_SETCURSEL,4,0); break;
	default: SendMessageA(m_hCbAnisotropy,CB_SETCURSEL,0,0); break;
	}

	switch(m_FullScreen)
	{
	case 0: SendMessageA(m_hCbFullscreen,CB_SETCURSEL,0,0); break;
	case 1: SendMessageA(m_hCbFullscreen,CB_SETCURSEL,1,0); break;
	}

	switch(m_Shadows)
	{
	case 0: SendMessageA(m_hCbShadows,CB_SETCURSEL,0,0); break;
	case 1: SendMessageA(m_hCbShadows,CB_SETCURSEL,1,0); break;
	}

	switch(m_RebuildObb)
	{
	case 0: SendMessageA(m_hCbRebuildObb,CB_SETCURSEL,0,0); break;
	case 1: SendMessageA(m_hCbRebuildObb,CB_SETCURSEL,1,0); break;
	}

	switch(m_ObbRenderDepth)
	{
	case 1: SendMessageA(m_hCbObbRenderDepth,CB_SETCURSEL,0,0); break;
	case 2: SendMessageA(m_hCbObbRenderDepth,CB_SETCURSEL,1,0); break;
	case 4: SendMessageA(m_hCbObbRenderDepth,CB_SETCURSEL,2,0); break;
	case 8: SendMessageA(m_hCbObbRenderDepth,CB_SETCURSEL,3,0); break;
	case 10: SendMessageA(m_hCbObbRenderDepth,CB_SETCURSEL,4,0); break;
	default: SendMessageA(m_hCbObbRenderDepth,CB_SETCURSEL,0,0); break;
	}
	switch(m_ObbDebugMode)
	{
	case 0: SendMessageA(m_hCbObbDebugMode,CB_SETCURSEL,0,0); break;
	case 1: SendMessageA(m_hCbObbDebugMode,CB_SETCURSEL,1,0); break;
	}

	File.close();
}//LoadSettings

void app::SaveSettings(void)
{
	ofstream File("settings.cfg",ios::trunc);

	//get selected settings from combo boxes
	switch(SendMessageA(m_hCbResolution,CB_GETCURSEL,0,0))
	{
	case 0: m_dwWidth = 640; m_dwHeight = 480; break;
	case 1: m_dwWidth = 800; m_dwHeight = 600; break;
	case 2: m_dwWidth = 1024; m_dwHeight = 768; break;
	case 3: m_dwWidth = 1152; m_dwHeight = 864; break;
	case 4: m_dwWidth = 1280; m_dwHeight = 960; break;
	case 5: m_dwWidth = 1280; m_dwHeight = 1024; break;
	case 6: m_dwWidth = 1600; m_dwHeight = 1200; break;
	case 7: m_dwWidth = 1280; m_dwHeight = 720; break;
	}

	switch(SendMessageA(m_hCbBackBuffer,CB_GETCURSEL,0,0))
	{
	case 0: m_ColorFormat = D3DFMT_R5G6B5; break;
	case 1: m_ColorFormat = D3DFMT_A1R5G5B5; break;
	case 2: m_ColorFormat = D3DFMT_X8R8G8B8; break;
	case 3: m_ColorFormat = D3DFMT_A8R8G8B8; break;
	}

	switch(SendMessageA(m_hCbDepthStencil,CB_GETCURSEL,0,0))
	{
	case 0: m_DepthStencilFormat = D3DFMT_D16; break;
	case 1: m_DepthStencilFormat = D3DFMT_D24S8; break;
	case 2: m_DepthStencilFormat = D3DFMT_D32; break;
	}

	switch(SendMessageA(m_hCbVertexProcessing,CB_GETCURSEL,0,0))
	{
	case 0: m_dwVertexProcessing = D3DCREATE_SOFTWARE_VERTEXPROCESSING; break;
	case 1: m_dwVertexProcessing = D3DCREATE_HARDWARE_VERTEXPROCESSING; break;
	}

	switch(SendMessageA(m_hCbMultiSampling,CB_GETCURSEL,0,0))
	{
	case 0: m_MultiSampling = D3DMULTISAMPLE_NONE; break;
	case 1: m_MultiSampling = D3DMULTISAMPLE_2_SAMPLES; break;
	case 2: m_MultiSampling = D3DMULTISAMPLE_4_SAMPLES; break;
	}

	switch(SendMessageA(m_hCbAnisotropy,CB_GETCURSEL,0,0))
	{
	case 0: m_dwAnisotropy = 0; break;
	case 1: m_dwAnisotropy = 2; break;
	case 2: m_dwAnisotropy = 4; break;
	case 3: m_dwAnisotropy = 8; break;
	case 4: m_dwAnisotropy = 16; break;
	}

	switch(SendMessageA(m_hCbFullscreen,CB_GETCURSEL,0,0))
	{
	case 0: m_FullScreen = 0; break;
	case 1: m_FullScreen = 1; break;
	}

	switch(SendMessageA(m_hCbShadows,CB_GETCURSEL,0,0))
	{
	case 0: m_Shadows = 0; break;
	case 1: m_Shadows = 1; break;
	}

	switch(SendMessageA(m_hCbRebuildObb,CB_GETCURSEL,0,0))
	{
	case 0: m_RebuildObb = 0; break;
	case 1: m_RebuildObb = 1; break;
	}

	switch(SendMessageA(m_hCbObbRenderDepth,CB_GETCURSEL,0,0))
	{
	case 0: m_ObbRenderDepth = 1; break;
	case 1: m_ObbRenderDepth = 2; break;
	case 2: m_ObbRenderDepth = 4; break;
	case 3: m_ObbRenderDepth = 8; break;
	case 4: m_ObbRenderDepth = 10; break;
	}
	switch(SendMessageA(m_hCbObbDebugMode,CB_GETCURSEL,0,0))
	{
	case 0: m_ObbDebugMode = 0; break;
	case 1: m_ObbDebugMode = 1; break;
	}

	//save settings to file
	File << m_dwWidth << endl;
	File << m_dwHeight << endl;
	File << m_ColorFormat << endl;
	File << m_DepthStencilFormat << endl;
	File << m_dwVertexProcessing << endl;
	File << m_MultiSampling << endl;
	File << m_dwAnisotropy << endl;
	File << m_FullScreen << endl;
	File << m_Shadows << endl;
	File << m_RebuildObb << endl;
	File << m_ObbRenderDepth << endl;
	File << m_ObbDebugMode << endl;

	File.close();
}//SaveSettings

/************************************************************************/
/* Name:		CheckDevice												*/
/* Description:	check for lost device --> ALT-TAB BUG!					*/
/************************************************************************/
bool app::CheckDevice(void)
{
	switch(m_pD3DDev->TestCooperativeLevel()) // a) lost, b) restorable, c) everying ok
	{
	case D3DERR_DEVICELOST: 
		{
			Sleep(20); // Pause to give other processes CPU
			return 0; // Stop the 3D rendering (we've lost focus)
		}
	case D3DERR_DRIVERINTERNALERROR:
		{
			MessageBox(m_hWindow,L"Internal Driver Error...Exiting", 0, 0);
			PostQuitMessage(0);
			return 0;
		}
	case D3DERR_DEVICENOTRESET: // Attempt to restart the rendering
		{
			g_renderer->onLostDevice();
			if(FAILED(m_pD3DDev->Reset(&m_PresentParameters))) 
			{
				MessageBox(m_hWindow,L"Reset() failed!",L"CheckDevice()",MB_OK);
				return 0;
			}
			g_renderer->onResetDevice();
			return 1;
		}
	default: return 1; // Otherwise, everything is ok
	}
}//CheckDevice

/************************************************************************/
/* Name:		KillWindow												*/
/* Description:	unregister window class									*/
/************************************************************************/
void app::KillWindow(void)
{
	UnregisterClass(L"ClassName",GetModuleHandle(NULL));
}//KillWindow

/************************************************************************/
/* Name:		KillD3D													*/
/* Description:	release memory for Direct3D device and Direct3D object	*/
/************************************************************************/
void app::KillD3D(void)
{
	ReleaseCOM(m_pD3DDev);
	ReleaseCOM(m_pD3DObj);

}//KillD3D

/************************************************************************/
/* Name:		KillObjects												*/
/* Description:	release memory for Rigid Body Objects					*/
/************************************************************************/
void app::KillObjects(void)
{
	// Do Nothing
}//KillObjects

/************************************************************************/
/* Name:		AddButtons and RemoveButtons							*/
/* Description:	Get rid of the buttons and all boxes					*/
/************************************************************************/
void app::AddButtons(void)
{
	// Alignment values (can be changed if you want)
	int x_textAlign	= 20;			int x_comboboxAlign	= 160;
	int x_textWidth	= 200;			int x_comboboxWidth	= 160;
	int x_textHight	= 18;			int x_comboboxHight	= 100;
	int y_spacing	= 12;
	// Y-values which get calculated along the way
	int y_textcur	= 24;			int y_comboboxCur	= 20;
	
	int i = 0;
	m_hLblFullscreen = CreateWindow(L"static",L"Fullscreen:",WS_CHILD | WS_VISIBLE | SS_LEFT,x_textAlign,y_textcur,x_textWidth,x_textHight,m_hWindow,NULL,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);
	m_hCbFullscreen = CreateWindow(L"combobox", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL,x_comboboxAlign,y_comboboxCur,x_comboboxWidth,x_comboboxHight,m_hWindow,NULL,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);
	y_textcur += y_spacing + x_textHight; y_comboboxCur += y_spacing + x_textHight;
	m_hLblResolution = CreateWindow(L"static",L"Resolution:",WS_CHILD | WS_VISIBLE | SS_LEFT,x_textAlign,y_textcur,x_textWidth,x_textHight,m_hWindow,NULL,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);
	m_hCbResolution = CreateWindow(L"combobox", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL,x_comboboxAlign,y_comboboxCur,x_comboboxWidth,x_comboboxHight,m_hWindow,NULL,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);
	y_textcur += y_spacing + x_textHight; y_comboboxCur += y_spacing + x_textHight;
	m_hLblBackBuffer = CreateWindow(L"static",L"Backbuffer Format:",WS_CHILD | WS_VISIBLE | SS_LEFT,x_textAlign,y_textcur,x_textWidth,x_textHight,m_hWindow,NULL,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);
	m_hCbBackBuffer = CreateWindow(L"combobox", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL,x_comboboxAlign,y_comboboxCur,x_comboboxWidth,x_comboboxHight,m_hWindow,NULL,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);
	y_textcur += y_spacing + x_textHight; y_comboboxCur += y_spacing + x_textHight;
	m_hLblDepthStencil = CreateWindow(L"static",L"DepthStencil Format:",WS_CHILD | WS_VISIBLE | SS_LEFT,x_textAlign,y_textcur,x_textWidth,x_textHight,m_hWindow,NULL,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);
	m_hCbDepthStencil = CreateWindow(L"combobox", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL,x_comboboxAlign,y_comboboxCur,x_comboboxWidth,x_comboboxHight,m_hWindow,NULL,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);
	
	y_textcur += (int)(3.5*y_spacing) + x_textHight; y_comboboxCur += (int)(3.5*y_spacing) + x_textHight; // big space
	m_hLblVertexProcessing = CreateWindow(L"static",L"Vertex Processing:",WS_CHILD | WS_VISIBLE | SS_LEFT,x_textAlign,y_textcur,x_textWidth,x_textHight,m_hWindow,NULL,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);
	m_hCbVertexProcessing = CreateWindow(L"combobox", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL,x_comboboxAlign,y_comboboxCur,x_comboboxWidth,x_comboboxHight,m_hWindow,NULL,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);
	y_textcur += y_spacing + x_textHight; y_comboboxCur += y_spacing + x_textHight;
	m_hLblMultiSampling = CreateWindow(L"static",L"MultiSampling:",WS_CHILD | WS_VISIBLE | SS_LEFT,x_textAlign,y_textcur,x_textWidth,x_textHight,m_hWindow,NULL,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);
	m_hCbMultiSampling = CreateWindow(L"combobox", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL,x_comboboxAlign,y_comboboxCur,x_comboboxWidth,x_comboboxHight,m_hWindow,NULL,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);
	y_textcur += y_spacing + x_textHight; y_comboboxCur += y_spacing + x_textHight;
	m_hLblAnisotropy = CreateWindow(L"static",L"Anisotropy:",WS_CHILD | WS_VISIBLE | SS_LEFT,x_textAlign,y_textcur,x_textWidth,x_textHight,m_hWindow,NULL,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);
	m_hCbAnisotropy = CreateWindow(L"combobox", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL,x_comboboxAlign,y_comboboxCur,x_comboboxWidth,x_comboboxHight,m_hWindow,NULL,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);
	y_textcur += y_spacing + x_textHight; y_comboboxCur += y_spacing + x_textHight;
	m_hLblShadows = CreateWindow(L"static",L"Shadows:",WS_CHILD | WS_VISIBLE | SS_LEFT,x_textAlign,y_textcur,x_textWidth,x_textHight,m_hWindow,NULL,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);
	m_hCbShadows = CreateWindow(L"combobox", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL,x_comboboxAlign,y_comboboxCur,x_comboboxWidth,x_comboboxHight,m_hWindow,NULL,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);
	
	y_textcur += (int)(3.5*y_spacing) + x_textHight; y_comboboxCur += (int)(3.5*y_spacing) + x_textHight; // big space
	m_hLblRebuildObb = CreateWindow(L"static",L"Rebuild OBB Data:",WS_CHILD | WS_VISIBLE | SS_LEFT,x_textAlign,y_textcur,x_textWidth,x_textHight,m_hWindow,NULL,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);
	m_hCbRebuildObb = CreateWindow(L"combobox", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL,x_comboboxAlign,y_comboboxCur,x_comboboxWidth,x_comboboxHight,m_hWindow,NULL,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);
	y_textcur += y_spacing + x_textHight; y_comboboxCur += y_spacing + x_textHight;
	m_hLblObbRenderDepth = CreateWindow(L"static",L"OBB Render Depth:",WS_CHILD | WS_VISIBLE | SS_LEFT,x_textAlign,y_textcur,x_textWidth,x_textHight,m_hWindow,NULL,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);
	m_hCbObbRenderDepth = CreateWindow(L"combobox", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL,x_comboboxAlign,y_comboboxCur,x_comboboxWidth,x_comboboxHight,m_hWindow,NULL,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);
	y_textcur += y_spacing + x_textHight; y_comboboxCur += y_spacing + x_textHight;
	m_hLblObbDebugMode = CreateWindow(L"static",L"OBB Debug Mode:",WS_CHILD | WS_VISIBLE | SS_LEFT,x_textAlign,y_textcur,x_textWidth,x_textHight,m_hWindow,NULL,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);
	m_hCbObbDebugMode = CreateWindow(L"combobox", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL,x_comboboxAlign,y_comboboxCur,x_comboboxWidth,x_comboboxHight,m_hWindow,NULL,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);

	y_textcur += (int)((7/3)*y_spacing) + x_textHight; // big space
	m_hBtnStart = CreateWindow(L"button",L"Start",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,65,y_textcur,80,24,m_hWindow,(HMENU)ID_START,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);
	m_hBtnCancel = CreateWindow(L"button",L"Cancel",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,175,y_textcur,80,24,m_hWindow,(HMENU)ID_CANCEL,(HINSTANCE)GetWindowLong(m_hWindow,GWL_HINSTANCE),NULL);


	//fill combo boxes
	SendMessageA(m_hCbResolution,CB_ADDSTRING,0,(long)"640 x 480");
	SendMessageA(m_hCbResolution,CB_ADDSTRING,0,(long)"800 x 600");
	SendMessageA(m_hCbResolution,CB_ADDSTRING,0,(long)"1024 x 768");
	SendMessageA(m_hCbResolution,CB_ADDSTRING,0,(long)"1152 x 864");
	SendMessageA(m_hCbResolution,CB_ADDSTRING,0,(long)"1280 x 960");
	SendMessageA(m_hCbResolution,CB_ADDSTRING,0,(long)"1280 x 1024");
	SendMessageA(m_hCbResolution,CB_ADDSTRING,0,(long)"1600 x 1200");
	SendMessageA(m_hCbResolution,CB_ADDSTRING,0,(long)"1280 x 720 -> 720p");

	SendMessageA(m_hCbBackBuffer,CB_ADDSTRING,0,(long)"D3DFMT_R5G6B5");
	SendMessageA(m_hCbBackBuffer,CB_ADDSTRING,0,(long)"D3DFMT_A1R5G5B5");
	SendMessageA(m_hCbBackBuffer,CB_ADDSTRING,0,(long)"D3DFMT_X8R8G8B8");
	SendMessageA(m_hCbBackBuffer,CB_ADDSTRING,0,(long)"D3DFMT_A8R8G8B8");

	SendMessageA(m_hCbDepthStencil,CB_ADDSTRING,0,(long)"D3DFMT_D16");
	SendMessageA(m_hCbDepthStencil,CB_ADDSTRING,0,(long)"D3DFMT_D24S8");
	SendMessageA(m_hCbDepthStencil,CB_ADDSTRING,0,(long)"D3DFMT_D32");

	SendMessageA(m_hCbVertexProcessing,CB_ADDSTRING,0,(long)"Software");
	SendMessageA(m_hCbVertexProcessing,CB_ADDSTRING,0,(long)"Hardware");

	SendMessageA(m_hCbMultiSampling,CB_ADDSTRING,0,(long)"No FSAA");
	SendMessageA(m_hCbMultiSampling,CB_ADDSTRING,0,(long)"2x FSAA");
	SendMessageA(m_hCbMultiSampling,CB_ADDSTRING,0,(long)"4x FSAA");

	SendMessageA(m_hCbAnisotropy,CB_ADDSTRING,0,(long)"No AF");
	SendMessageA(m_hCbAnisotropy,CB_ADDSTRING,0,(long)"2x AF");
	SendMessageA(m_hCbAnisotropy,CB_ADDSTRING,0,(long)"4x AF");
	SendMessageA(m_hCbAnisotropy,CB_ADDSTRING,0,(long)"8x AF");
	SendMessageA(m_hCbAnisotropy,CB_ADDSTRING,0,(long)"16x AF");

	SendMessageA(m_hCbFullscreen,CB_ADDSTRING,0,(long)"Windowed");
	SendMessageA(m_hCbFullscreen,CB_ADDSTRING,0,(long)"Fullscreen");

	SendMessageA(m_hCbShadows,CB_ADDSTRING,0,(long)"OFF");
	SendMessageA(m_hCbShadows,CB_ADDSTRING,0,(long)"ON");


	SendMessageA(m_hCbObbRenderDepth,CB_ADDSTRING,0,(long)"1");
	SendMessageA(m_hCbObbRenderDepth,CB_ADDSTRING,0,(long)"2");
	SendMessageA(m_hCbObbRenderDepth,CB_ADDSTRING,0,(long)"4");
	SendMessageA(m_hCbObbRenderDepth,CB_ADDSTRING,0,(long)"8");
	SendMessageA(m_hCbObbRenderDepth,CB_ADDSTRING,0,(long)"10");

	SendMessageA(m_hCbRebuildObb,CB_ADDSTRING,0,(long)"OFF");
	SendMessageA(m_hCbRebuildObb,CB_ADDSTRING,0,(long)"ON");

	SendMessageA(m_hCbObbDebugMode,CB_ADDSTRING,0,(long)"OFF");
	SendMessageA(m_hCbObbDebugMode,CB_ADDSTRING,0,(long)"ON");
} // AddButtons

void app::RemoveButtons(void)
{
	DestroyWindow(m_hBtnStart);
	DestroyWindow(m_hBtnCancel);
	DestroyWindow(m_hLblResolution);
	DestroyWindow(m_hCbResolution);
	DestroyWindow(m_hLblBackBuffer);
	DestroyWindow(m_hCbBackBuffer);
	DestroyWindow(m_hLblDepthStencil);
	DestroyWindow(m_hCbDepthStencil);
	DestroyWindow(m_hLblVertexProcessing);
	DestroyWindow(m_hCbVertexProcessing);
	DestroyWindow(m_hLblMultiSampling);
	DestroyWindow(m_hCbMultiSampling);
	DestroyWindow(m_hLblAnisotropy);
	DestroyWindow(m_hCbAnisotropy);
	DestroyWindow(m_hLblFullscreen);
	DestroyWindow(m_hCbFullscreen);
	DestroyWindow(m_hLblShadows);
	DestroyWindow(m_hCbShadows);
	DestroyWindow(m_hLblRebuildObb);
	DestroyWindow(m_hCbRebuildObb);
	DestroyWindow(m_hLblObbRenderDepth);
	DestroyWindow(m_hCbObbRenderDepth);
	DestroyWindow(m_hLblObbDebugMode);
	DestroyWindow(m_hCbObbDebugMode);
} // Remove Buttons
/************************************************************************/
/* Name:		toggleFullScreenMode									*/
/* Description:	Toggle between fullscreen mode when user presses F1		*/
/************************************************************************/
void app::toggleFullScreenMode(void)
{
	DWORD dwStyle;
	int xSet, ySet, dxSet, dySet;

	if( !m_FullScreen ) // Toggle from windowed to fullscreen mode.
	{
		if( !m_PresentParameters.Windowed )  // Are we already in fullscreen mode?
			return; // do nothing
		m_PresentParameters.Windowed = 0; m_FullScreen = 1;
		dwStyle = WS_POPUP;
		xSet = 0;
		ySet = 0;
		dxSet = m_dwWidth;
		dySet = m_dwHeight;	
	}
	else // Toggle from fullscreen to window mode.
	{
		if( m_PresentParameters.Windowed )  // Are we already in windowed mode?
			return; // do nothing
		m_PresentParameters.Windowed = 1; m_FullScreen = 0;
		dwStyle = (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_THICKFRAME;
		RECT DesktopSize; SIZE sizeWindow; RECT rc;
		rc.top = rc.left = 0; rc.right = m_dwWidth; rc.bottom = m_dwHeight;
		AdjustWindowRect(&rc, dwStyle, 0); // in window mode the border can hide some pixels
		sizeWindow.cx = rc.right - rc.left;
		sizeWindow.cy = rc.bottom - rc.top;
		GetClientRect(GetDesktopWindow(),&DesktopSize); //Get desktop resolution
		xSet = (DesktopSize.right - m_dwWidth) / 2;
		ySet = (DesktopSize.bottom - m_dwHeight) / 2;
		dxSet = sizeWindow.cx;
		dySet = sizeWindow.cy;	
	}

	// Attempt to make the changes
	SetWindowLongPtr(m_hWindow, GWL_STYLE, dwStyle);
	if(!SetWindowPos(m_hWindow, NULL, xSet, ySet, dxSet, dySet, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW))
	{
		m_strError = L"Failed to resize window!!";
		m_bRunningD3D = 0; // Stop everything!
		return;
	}	

	// Reset the device with the changes.
	g_renderer->onLostDevice();
	if(FAILED(m_pD3DDev->Reset(&m_PresentParameters))) 
	{
		MessageBox(m_hWindow,L"Reset() failed!",L"CheckDevice()",MB_OK);
		return;
	}
	g_renderer->onResetDevice();
}
/************************************************************************/
/* Name:		startTime & getTime										*/
/* Description:	Functions to keep track of time in WinMain				*/
/************************************************************************/
float app::getTime(void)
{
    if (m_cntStart==0)
    {
        if(!QueryPerformanceFrequency((LARGE_INTEGER*)&m_cntFreq))
		{
			throw std::runtime_error("getTime: Failed to get performance counter!");
			return 0.0f;
		}
		m_secsPerCnt = 1.0f / (float)m_cntFreq;
		QueryPerformanceCounter((LARGE_INTEGER*)&m_cntStart);
        return 0.0f;
    }

    QueryPerformanceCounter((LARGE_INTEGER*)&m_cntCurrent);
    return  ((float)(m_cntCurrent - m_cntStart) * (float)m_secsPerCnt);
}

/************************************************************************/
/* Name:		SetProcessorAffinity									*/
/* Description:	 Assign the current thread to one processor. Makes sure	*/
/*				 timing functions are correct.							*/
/************************************************************************/
void app::SetProcessorAffinity(void)
{
    DWORD_PTR dwProcessAffinityMask = 0;
    DWORD_PTR dwSystemAffinityMask = 0;
    HANDLE hCurrentProcess = GetCurrentProcess();

    if (!GetProcessAffinityMask(hCurrentProcess, &dwProcessAffinityMask, &dwSystemAffinityMask))
        return;

    if (dwProcessAffinityMask)
    {
        // Find the lowest processor that our process is allowed to run against.
        DWORD_PTR dwAffinityMask = (dwProcessAffinityMask & ((~dwProcessAffinityMask) + 1));

        // Set this as the processor that our thread must always run against.
        // This must be a subset of the process affinity mask.
        HANDLE hCurrentThread = GetCurrentThread();

        if (hCurrentThread != INVALID_HANDLE_VALUE)
        {
            SetThreadAffinityMask(hCurrentThread, dwAffinityMask);
            CloseHandle(hCurrentThread);
        }
    }
    CloseHandle(hCurrentProcess);
}
/************************************************************************/
/* Name:		UICode													*/
/* Description:	 Gets and applies user input, and other misc. stuff		*/
/************************************************************************/
void app::UICode(void)
{
	if(g_keyboard->keyPressed(KEY_C))
	{
		m_drawConvexHull = !m_drawConvexHull;
	}
	if(g_keyboard->keyPressed(KEY_O))
	{
		m_drawOBBTree = !m_drawOBBTree;
	}
	if(g_keyboard->keyPressed(KEY_M))
	{
		m_drawModels = !m_drawModels;
	}
	if(g_keyboard->keyPressed(KEY_N))
	{
		m_ObbDebugModeNext = 1;
	}
	if(g_keyboard->keyPressed(KEY_P))
	{
		m_pausePhysics = !m_pausePhysics;
	}
	if(g_keyboard->keyPressed(KEY_L))
	{
		for(int i = 0; i < NO_RBOBJECTS; i++) { g_rbobjects[i]->OBBTree->MoveUp(); }
	}
	if(g_keyboard->keyPressed(KEY_COMMA))  // ,
	{
		for(int i = 0; i < NO_RBOBJECTS; i++) { g_rbobjects[i]->OBBTree->MoveLeft(); }
	}
	if(g_keyboard->keyPressed(KEY_PERIOD)) // .
	{
		for(int i = 0; i < NO_RBOBJECTS; i++) { g_rbobjects[i]->OBBTree->MoveRight(); }
	}
}