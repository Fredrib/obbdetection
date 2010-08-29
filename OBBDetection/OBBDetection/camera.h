// file name:	camera.cpp
// author:		Costa Itskov
// e-mail:		virgoci@mail.ru
// web site:	http://www.two-kings.de

// Heavilty modified by Jonathan Tompson.  Summer 2009.

#ifndef camera_h
#define camera_h

#include"main.h"

class camera
{
public:
						camera(void);

	void				Update(void);
	void				Move(void);
	void				Rotate(void);
	inline D3DXVECTOR3	GetEyePt(void) { return m_vEyePt; }

	// Less overhead if public
	D3DMATRIX			m_ViewProj;
	D3DXVECTOR3			m_vEyePt;

private:
	D3DXVECTOR3			m_vLookAtPt,
						m_vUp;

	float				m_fSpeed;

	D3DXMATRIX			m_matView,
						m_matProjection;

	float				m_fieldofView,
						m_aspectRatio,
						m_nearViewPlane,
						m_farViewPlane;
};

#endif