/***********************************************************
**						OBBDetection!					  **
** -> A OBB collision detection engine					  **
***********************************************************/
// File:		main.cpp
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com
//
// Acknowledgments: To get started I used some code taken from www.two-kings.de and 
// some from www.directxtutorial.com just to familiarize myself with Win32 applications.
// Also Frank Luna was a huge help.
// 

#include	"main.h"

// GLOBALS
app	*			g_app = NULL;				// Application Interface: windows are setup on this constructor call
renderer *		g_renderer = NULL;			// Renderer Class
physics *		g_physics = NULL;			// Runge Kutta 4th Order Numerical ODE Solver
camera *		g_camera = NULL;			// A camera class with vector state implimented rotations (change to quaternions later)
keyboard *		g_keyboard = NULL;			// A keyboard class for DirectInput manipulation
mouse *			g_mouse = NULL;				// A mouse class for DirectInput manipulation
sky *			g_sky = NULL;				// A sky class (for background)
hud *			g_hud = NULL;				// A class to handle hud display

// OBJECT DATA
// Note: ARRAY NOTATION HERE DIDN'T WORK! g_rboMesh[1] would overlap address spaces with g_rboMesh[0]!
rbobject **				g_rbobjects;				// An array of Rigid Body Objects
light *					g_light;					// An array of lights

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPreviousInstance,LPSTR lpcmdline,int nCmdShow)
{
#ifdef _DEBUG
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);
	//	_CrtDumpMemoryLeaks(); (gives false dumps)
#endif

	app::InitApp();

	MSG Message;
	Message.message = WM_NULL;

	//window loop
	while(g_app->m_bRunningWindow) //GetWindowStatus() 1 if window is running (0 if stopped)
	{
		if(GetMessage(&Message,NULL,0,0)) // Get messages from queue --> hangs until a massage arrives (Blocking)
		{
			if(!IsDialogMessage(g_app->GetWindowHandle(),&Message)) 
				// Messages used for translation (tabs, etc) handled seperately
			{
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			}
			if(g_app->m_bAppPaused)
			{
				Sleep(20); // Free up some CPU cycles to other apps
				continue;
			}
		}
	}

	if(g_app->m_bRunningD3D)
		g_app->InitD3D();

	//switch to D3D loop or quit --> Necessary if initialization failed or something went wrong
	if(!g_app->m_bRunningD3D)
	{
		std::wstring m_strError = g_app->GetError();
		if(!m_strError.empty())
		{
			MessageBox(NULL, m_strError.c_str(), L"Error", MB_OK | MB_ICONERROR);
			app::KillApp();
			return -1;
		}
	} else
	{
		try
		{
			// INITIALIZE IMPORTANT OBJECTS AND DEVICES
			InitAllVertexDeclarations();
			g_hud->InitHud();
			g_renderer->InitRenderer();
			g_app->SetProcessorAffinity();
			g_keyboard->InitKeyboard();
			g_mouse->InitMouse();

			// VALUES TO HOLD THE GAME TIME
			g_app->m_currentTime = g_app->getTime();

			float newTime = 0.0; // temporary value
			int framesSkip = 0; // number of times we've taken a physics step without rendering it

			//D3D loop
			while(g_app->m_bRunningD3D)
			{
				// Check to see if any messages are waiting in the queue and drop out if not
				if(PeekMessage(&Message,NULL,0,0,PM_REMOVE)) // PM_REMOVE --> Remove off message queue when read
				{
					// Translate the message and dispatch it to WindowProc()
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				} 
				else 
				{
					if(g_app->m_bAppPaused)
					{
						Sleep(20); // Free up some CPU cycles to other apps
						continue;
					}
					if(g_app->CheckDevice()) // See if the device got lost --> ALT-TAB bug
					{
						g_keyboard->Update();
						g_mouse->Update();
						g_app->UICode(); // Gets and applies user input, and other misc. stuff

						if(g_app->m_bRunningD3D) // Might render frame after killing it! Check again to be sure.
							newTime = g_app->getTime(); // temporary value to calculate deltaT
						g_app->m_deltaTime = newTime - g_app->m_currentTime;
						g_app->m_currentTime = newTime;

						g_app->m_accumulator += g_app->m_deltaTime;

						// While: Physics is behind rending time --> Update it and stop if we hit a max (ensures we update every so often)
						// Note: Physics time will have a latency of PHYSICS_STEP.
						framesSkip = 0;
						while ((g_app->m_accumulator >= PHYSICS_STEP) && (framesSkip <= MAX_RENDER_FRAME_SKIP)) 
						{
							g_app->m_accumulator -= PHYSICS_STEP; // we're about to take a physics step so remove it from accumulator
							g_physics->calcStep(); // Calculate delta V and all the other velocities (probably wont use this implimentation)
							g_app->m_currentTimePhysics += PHYSICS_STEP;
							framesSkip ++;
							// Now g_physics->y and g_physics->yfinal are calculated (and ready for interpolation)
						}

						g_app->m_alphaTime = g_app->m_accumulator / PHYSICS_STEP; // Interpolation amount
						g_physics->InterpolateFrame(g_app->m_alphaTime);

						g_hud->update(g_app->m_deltaTime);

						g_renderer->RenderFrame();

					} // end if
				} // end else
			} // end while
		} // end tr
		catch (const std::exception &e)
		{
			g_app->m_bRunningD3D = 0;
			MessageBox(NULL, toWideString(e.what(),-1).c_str(), L"Error", MB_OK | MB_ICONERROR);
			app::KillApp();
			return -1;
		}
	}

	app::KillApp();
	return 0;

}//WinMain