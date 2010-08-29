// File:		app.h
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com
// Basic skelleton code borrowed from www.two-kings.de: See notes in app.cpp

#ifndef app_h
#define app_h

#include"main.h"

class app
{
public:
								app(void); // Default Constructor
								~app(void); // Destructor Destructor
	static void					InitApp(void); // make all global objects
	static void					KillApp(void); // delete all global objects

	void						InitWindow(void);
	void						InitD3D(void);
	bool						CheckObjCaps(void);
	bool						CheckDevCaps(void);
	void						SaveSettings(void);
	void						LoadSettings(void);
	void						RemoveButtons(void);
	void						AddButtons(void);
	void						toggleFullScreenMode(void);
	float						getTime(void);
	bool						CheckDevice(void);
	void						SetProcessorAffinity(void);

	void						KillWindow(void);
	void						KillD3D(void);
	void						KillObjects(void);

	void						UICode(void); // Get and process user input (key strokes, etc)

	// Access functions
	inline HWND					GetWindowHandle(void)					{ return m_hWindow; }
	inline DWORD				GetWidth(void)							{ return m_dwWidth; }
	inline DWORD				GetHeight(void)							{ return m_dwHeight; }
	inline std::wstring			GetError(void) const					{ return m_strError; }
	inline int					GetObbRenderDepth(void)					{ return m_ObbRenderDepth; }
	inline bool					CheckRebuildObb(void)					{ return m_RebuildObb; }
	inline bool					CheckObbDebugMode(void)					{ return m_ObbDebugMode; }

	// Modifier functions
	inline void					SetError(const std::wstring& errorin)	{ m_strError = errorin; }

	friend class renderer;	// I ADMIT THIS IS LAZY OOP! --> But makes life easier.

	// Values to hold the time for WinMain function
	float						m_deltaTime, // Time since last polling
								m_alphaTime, // Interpolation time (to linearly fill in frames between physics steps)
								m_accumulator, // Time since last physics step (ODE solver will try and keep this 0)
								m_currentTime, // Current simulation time (from performance counter)
								m_currentTimePhysics; // Current physics simulation time

	// THESE TWO DEVICES USED TO BE PRIVATE WITH ACCESS MODIFIERS --> TOO MUCH OVERHEAD
	LPDIRECT3D9					m_pD3DObj; // the long pointer to our Direct3D interface
	LPDIRECT3DDEVICE9			m_pD3DDev; // the long pointer to the device class --> Hardware side

	// UI and Debug settings
	bool						m_drawConvexHull;
	bool						m_drawOBBTree;
	bool						m_drawModels;
	bool						m_pausePhysics;
	bool						m_EnterObbDebugMode;
	bool						m_ObbDebugModeNext;

	bool						m_bRunningWindow, // Program will run until 0
								m_bRunningD3D, // Direct3D loop
								m_bAppPaused; // still running app but out of scope

private:
	// Handle to window and other window objects
	HWND						m_hWindow,								
								m_hBtnStart,
								m_hBtnCancel,
								m_hLblResolution,			m_hCbResolution,
								m_hLblBackBuffer,			m_hCbBackBuffer,
								m_hLblDepthStencil,			m_hCbDepthStencil,
								m_hLblVertexProcessing,		m_hCbVertexProcessing,
								m_hLblMultiSampling,		m_hCbMultiSampling,
								m_hLblAnisotropy,			m_hCbAnisotropy,
								m_hLblFullscreen,			m_hCbFullscreen,
								m_hLblShadows,				m_hCbShadows,
								m_hLblObbRenderDepth,		m_hCbObbRenderDepth,
								m_hLblRebuildObb,			m_hCbRebuildObb,
								m_hLblObbDebugMode,			m_hCbObbDebugMode;

	D3DPRESENT_PARAMETERS		m_PresentParameters; // Hold information about the card (parameters)
	// --> PUT CAPS ONTO HEAP TO AVOID RUNTIME ERRORS IN VC2008 EXPRESS!!
	D3DCAPS9 *					m_DeviceCaps; // Hold capabilities of the card 
	D3DCAPS9 *					m_ObjectCaps; // Hold capabilities of the card

	// Desired settings
	DWORD						m_dwWidth,
								m_dwHeight,
								m_dwVertexProcessing,
								m_dwAnisotropy;
	D3DFORMAT					m_ColorFormat,
								m_DepthStencilFormat;
	D3DMULTISAMPLE_TYPE			m_MultiSampling;
	bool						m_FullScreen,
								m_Shadows,
								m_RebuildObb,
								m_ObbDebugMode;
	int							m_ObbRenderDepth;

	// Error handling
	std::wstring				m_strError; // error message to return

	// Counter variables
	__int64						m_cntFreq, // Frequency of the precision counter
								m_cntCurrent, // Current counter value
								m_cntStart; // Start time for precision counter
	float						m_secsPerCnt; // counter period
};

#endif