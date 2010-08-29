// File:		main.h
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com

#ifndef main_h
#define main_h

// Enable full D3D debugging for debug builds
#ifdef _DEBUG

#define _CRTDBG_MAP_ALLOC  // Give report on memory leaks

#ifndef D3D_DEBUG_INFO
#define D3D_DEBUG_INFO
#endif

#endif

//#ifndef WIN32_LEAN_AND_MEAN 
//#define WIN32_LEAN_AND_MEAN
//#endif

#ifndef DCGAL_CH_NO_POSTCONDITIONS
#define DCGAL_CH_NO_POSTCONDITIONS
#endif

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif

#define _DEFINE_DEPRECATED_HASH_CLASSES 0 // Get rid of warning that hash tables aren't part of "namespace std"

//includes
#include	<windows.h>				// Win32API
#include	<commctrl.h>			// Windows XP Styles
#include	<d3d9.h>				// main Direct3D header
#pragma warning( disable : 4996 )	// disable deprecated warning 
#include	<d3dx9.h>				// Direct3D Extensions header
#include	<fstream>				// IO Stream to files
#include	<string>
#include	<tchar.h>
#include	<math.h>
#include	<dinput.h>
#include	<utility>
#include	<vector>				// Resizable array (O(2) amortized time)
#include	<list>					// Doubly-linked-list
#include	<sstream>
#include	<stdexcept>
#include	<stdlib.h>
#include	<crtdbg.h>
#include	<assert.h>
#include	<float.h>
#include	<sys/stat.h>

using namespace std;
#define ReleaseCOM(x) { if(x) { x->Release(); x = NULL; } } // R. Luna's code

// My functions
#include	"vertex.h"
#include	"app.h"
#include	"renderer.h"
#include	"util.h"
#include	"d3dformats.h"
#include	"rbobjects.h"
#include	"physics.h"
#include	"camera.h"
#include	"input.h"
#include	"light.h"
#include	"collision.h"
#include	"obbox.h"
#include	"obbtree.h"
#include	"hashset_int.h"
#include	"hud.h"
#include	"obbox_renderitems.h"
#include	"debug_object.h"
#include	"tritri_isectline.h"

// The next three classes are from F. Luna's book (used for shaddow mapping and background) --> Heavily modified!
#include	"sky.h"
#include	"vertex.h"
#include	"DrawableTex2D.h"

// Not sure who this is by, but is a function for finding eigenvalues of a symmetric 3x3 matrix and is useful
#include	"eig3.h"

// Convex hull libraries
#include	"CGAL.h"				// CGAL
// #include	"qhull_interface.h"		// qhull
#include "hull.h"					// StanHull --> Has some other stuff I use and many functions I wrote.

#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__) // Enable Debug version
#define new DEBUG_NEW
#endif

//constants - Objects and resolution
#define TITLE							"OBBDetection"
#define SETUPWINDOW_X					350
#define SETUPWINDOW_Y					500
#define NO_RBOMESHES					2
#define NO_RBOINDEXES					0		// NO LONGER IN USE...
#define NO_RBOBJECTS					(NO_RBOMESHES + NO_RBOINDEXES)
#define EPSILON							0.00000001 // A small value (with some margin)

//constants - UI
#define CAMERA_SPEED					0.1f
#define CAMERA_SPEED_FAST_MULTIPLIER	5.0f

//constants -Renderer
#define SMAP_SIZE						512
#define NO_LIGHTS						1		// MUST BE 1 FOR NOW!!!!

//constants - Physics
#define STATESIZE							13		// size of a rbobject (x, quat, P and L) in array form
const D3DXVECTOR3 GRAVITY					= D3DXVECTOR3(0.0f, -9.8f, 0.0f);
#define PHYSICS_STEP						0.05f // DEFAULT = 0.05F (20Hz)
#define MAX_RENDER_FRAME_SKIP				10 // If physics gets behind, still render some frames: DEFAULT = 4 
#define	PAUSE_PHYSICS						1
#define ADD_VECTOR_PRETURB					0
#define PAUSE_ON_OBBLEAF_COLLISION			1 // For debugging collision system
#define RENDER_OBBLEAF_COLLISION_TRIANGLES	1 // Render the actual triangles that are potenitally colliding
#define	RENDER_OBBLEAF_COLLISION_BOXES		0 // Render the obbox cells that are colliding
#define	COLLISION_VECTOR_SIZE				1024 // Starting size of vector storing collisions (big value avoids resizing at runtime)

//constants - OBB Generation
#define FIND_CONVEXHULL		1 // 0 = Don't bother using CGAL to find the convex hulls --> Just use model
#define ONE_LEVEL_OBBTREE	0 // 1 = Use to generate only root level of OBBTree --> For debuging Convex Hull
#define DRAW_OBB_AS_LINES	1 // 1 = draw obb as a bunch of line primates. 0 = draw as triangle primaties (with diagonals -> looks ugly)
#define CGAL_ONLY			0 // If FIND_CONVEXHULL = 1: 1 = Just use CGAL, 0 = Use StanHull + CGAL (faster, but less robust)

//Button ID's
#define ID_START			1
#define ID_CANCEL			2

//globals
extern app *		g_app;		
extern renderer	*	g_renderer;	
extern physics *	g_physics;		
extern camera *		g_camera;
extern keyboard	*	g_keyboard;	
extern mouse *		g_mouse;
extern sky *		g_sky;
extern hud *		g_hud;

//object data
extern light *		g_light;	
extern rbobject **	g_rbobjects;	

//functions
LRESULT CALLBACK			WindowProcedure(HWND,UINT,WPARAM,LPARAM); // Windows message handler

#endif