// A simple light class to clean up top level code
// Jonathan Tompson, Summer 2009

// Big changes from fixed function pipeline!! (used to be MUCH simpler)

#ifndef light_h
#define light_h

#include"main.h"

struct SpotLight
{
	D3DXCOLOR ambient;
	D3DXCOLOR diffuse;
	D3DXCOLOR spec;
	D3DXVECTOR3 posW;
	D3DXVECTOR3 dirW;  
	float  spotPower;
};

class light
{
public:
					light(void);
					~light(void);
	void			InitLightColor(D3DXCOLOR AMBIENTColor, D3DXCOLOR diffuseColor, D3DXCOLOR specColor);
	void			InitLightPos(D3DXVECTOR3 setPos, D3DXVECTOR3 setTarget, D3DXVECTOR3 setUp, float power, float setLightFOV);
	void			rotateLightY(float angle_rad);  // DEBUG FUNCTION: Only rotates position and up vectors (not target)

	D3DXMATRIX		m_LightVP;   // PUBLIC REDUCES OVERHEAD! WVP =  m_lightView * m_lightProj
	SpotLight		m_SpotLight;

private:
	D3DXVECTOR3		m_target,
					m_up;
	float			m_LightFOV;
	
	D3DXMATRIX		m_lightView,
					m_lightProj,
					m_rot, m_trans;
};

#endif