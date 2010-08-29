#ifndef d3dformats_h
#define d3dformats_h

#include	"main.h"

//constants
#define D3DFVF_CUSTOMVERTEX	(D3DFVF_XYZ | D3DFVF_NORMAL )

const D3DXCOLOR WHITE(1.0f, 1.0f, 1.0f, 1.0f);
const D3DXCOLOR BLACK(0.0f, 0.0f, 0.0f, 1.0f);
const D3DXCOLOR RED(1.0f, 0.0f, 0.0f, 1.0f);
const D3DXCOLOR GREEN(0.0f, 1.0f, 0.0f, 1.0f);
const D3DXCOLOR BLUE(0.0f, 0.0f, 1.0f, 1.0f);
const D3DXCOLOR YELLOW(1.0f, 1.0f, 0.0f, 1.0f);
const D3DXCOLOR CYAN(0.0f, 1.0f, 1.0f, 1.0f);
const D3DXCOLOR MAGENTA(1.0f, 0.0f, 1.0f, 1.0f);

//structures
struct CUSTOMVERTEX
{
float		fX,
			fY,
			fZ;
D3DVECTOR	NORMAL;
};

struct Mtrl
{
	Mtrl()
		:ambient(WHITE), diffuse(WHITE), spec(WHITE), specPower(8.0f){}
	Mtrl(const D3DXCOLOR& a, const D3DXCOLOR& d, 
		 const D3DXCOLOR& s, float power)
		:ambient(a), diffuse(d), spec(s), specPower(power){}

	D3DXCOLOR ambient;
	D3DXCOLOR diffuse;
	D3DXCOLOR spec;
	float specPower;
};

#endif