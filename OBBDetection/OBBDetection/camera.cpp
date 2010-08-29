// file name:	camera.h
// author:		Costa Itskov
// e-mail:		virgoci@mail.ru
// web site:	http://www.two-kings.de

// Heavilty modified by Jonathan Tompson.  Summer 2009.

#include "main.h"
#include "util.h"

/************************************************************************/
/* Name:		camera													*/
/* Description:	define member variables									*/
/************************************************************************/

camera::camera(void)
{
	m_vEyePt = D3DXVECTOR3(0.0f,20.0f,-100.0f);
	m_vLookAtPt = D3DXVECTOR3(0.0f,0.0f,0.0f);
	m_vUp = D3DXVECTOR3(0.0f, 1.0f, 0.0f); // This is only correct if m_vEyePt starts with (x,0,z)
	GetCorrectUp(&m_vLookAtPt, &m_vEyePt, &m_vUp); // To correct for arbitrary eye direction
	m_fSpeed = CAMERA_SPEED; // formally 0.25f
	m_fieldofView = D3DXToRadian(45);
	m_aspectRatio = (float)g_app->GetWidth() / (float)g_app->GetHeight();
	m_nearViewPlane = 1.0f;
	m_farViewPlane = 1000.0f;
}//camera

/************************************************************************/
/* Name:		Update													*/
/* Description:	call move() and rotate() and set view matrix			*/
/************************************************************************/
void camera::Update(void)
{
	Rotate();
	Move();

	D3DXMatrixLookAtLH(&m_matView,&m_vEyePt,&m_vLookAtPt,&m_vUp);

	D3DXMatrixPerspectiveFovLH(&m_matProjection, m_fieldofView, m_aspectRatio, m_nearViewPlane, m_farViewPlane);

	m_ViewProj = m_matView * m_matProjection;
}//Update

/************************************************************************/
/* Name:		Move													*/
/* Description:	change m_vEyePt and m_vLookAtPt							*/
/************************************************************************/

void camera::Move(void)
{
	D3DXVECTOR3 vDirection;

	D3DXVec3Normalize(&vDirection,&(m_vLookAtPt - m_vEyePt));
	float mov_scale = g_keyboard->keyDown(KEY_LSHIFT) ? m_fSpeed * CAMERA_SPEED_FAST_MULTIPLIER : m_fSpeed * 1.0f;
		

	if(g_keyboard->keyDown(KEY_W))
	{
		m_vEyePt += vDirection * mov_scale;
		m_vLookAtPt += vDirection * mov_scale;
	}
	if(g_keyboard->keyDown(KEY_S))
	{
		m_vEyePt -= vDirection * mov_scale;
		m_vLookAtPt -= vDirection * mov_scale;
	}
	if(g_keyboard->keyDown(KEY_A))
	{
		D3DXVec3Cross(&vDirection,&vDirection,&m_vUp);
		D3DXVec3Normalize(&vDirection,&vDirection);
		m_vEyePt += vDirection * mov_scale;
		m_vLookAtPt += vDirection * mov_scale;
	}
	if(g_keyboard->keyDown(KEY_D))
	{
		D3DXVec3Cross(&vDirection,&vDirection,&m_vUp);
		D3DXVec3Normalize(&vDirection,&vDirection);
		m_vEyePt -= vDirection * mov_scale;
		m_vLookAtPt -= vDirection * mov_scale;
	}
	if(g_keyboard->keyDown(KEY_Q))
	{
		D3DXVec3Normalize(&vDirection,&m_vUp);
		m_vEyePt -= vDirection * mov_scale;
		m_vLookAtPt -= vDirection * mov_scale;
	}
	if(g_keyboard->keyDown(KEY_E))
	{
		D3DXVec3Normalize(&vDirection,&m_vUp);
		m_vEyePt += vDirection * mov_scale;
		m_vLookAtPt += vDirection * mov_scale;
	}
}//Move

/************************************************************************/
/* Name:		Rotate													*/
/* Description:	rotate m_vLookAtPt and m_vUpVec around the axis			*/
/************************************************************************/

void camera::Rotate()
{
	D3DXVECTOR3 vDirection,vRotAxis;
	D3DXMATRIX matRotAxis,matRotZ;

	D3DXVec3Normalize(&vDirection,&(m_vLookAtPt - m_vEyePt));
	D3DXVec3Cross(&vRotAxis,&vDirection,&m_vUp);
	D3DXVec3Normalize(&vRotAxis,&vRotAxis);

	D3DXMatrixRotationAxis(&matRotAxis,&vRotAxis,g_mouse->yPosRelative() / -360);
	D3DXMatrixRotationY(&matRotZ,g_mouse->xPosRelative() / 360);

	D3DXVec3TransformCoord(&vDirection,&vDirection,&(matRotAxis * matRotZ));
	D3DXVec3TransformCoord(&m_vUp,&m_vUp,&(matRotAxis * matRotZ));
	m_vLookAtPt = vDirection + m_vEyePt;
}//Rotate