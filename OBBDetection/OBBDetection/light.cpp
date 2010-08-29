// A simple light class
// Jonathan Tompson, Summer 2009

#include"main.h"

light::light(void)
{
	// Zero all internal values
	m_target.x = 0.0f; m_target.y = 0.0f; m_target.z = 0.0f; 
	m_up.x = 0.0f; m_up.y = 0.0f; m_up.z = 0.0f; 
	m_LightFOV = 0.0f;
	D3DXMatrixIdentity(&m_LightVP);
	D3DXMatrixIdentity(&m_lightView);
	D3DXMatrixIdentity(&m_lightProj);
	ZeroMemory(& m_SpotLight, sizeof(SpotLight));
}

light::~light(void)
{
	// do nothing
}

void light::InitLightColor(D3DXCOLOR AMBIENTColor, D3DXCOLOR diffuseColor, D3DXCOLOR specColor)
{
	m_SpotLight.ambient = AMBIENTColor;
	m_SpotLight.diffuse = diffuseColor;
	m_SpotLight.spec = specColor;
}

void light::InitLightPos(D3DXVECTOR3 setPos, D3DXVECTOR3 setTarget, D3DXVECTOR3 setUp, float power, float setLightFOV)
{
	m_SpotLight.posW = setPos; 
	m_SpotLight.spotPower = power;
	m_target = setTarget; m_up = setUp;
	m_SpotLight.dirW = m_target - m_SpotLight.posW; D3DXVec3Normalize(&m_SpotLight.dirW, &m_SpotLight.dirW); // calculate unit direction vector
	GetCorrectUp(&m_target, &m_SpotLight.posW, &m_up); // Normalize the up vector to be tangent to direction (with correct rotation)
	D3DXMatrixLookAtLH(&m_lightView, &m_SpotLight.posW, &m_target, &m_up); // Calculate the light view matrix
	m_LightFOV = setLightFOV;
	D3DXMatrixPerspectiveFovLH(&m_lightProj, m_LightFOV, 1.0f, 1.0f, 200.0f); // Calculate the light projection matrix
	m_LightVP =  m_lightView * m_lightProj;
}

void light::rotateLightY(float angle_rad)
{
	// Re-build look-at matrix
	D3DXMATRIX Ry;
	D3DXMatrixRotationY(&Ry, angle_rad);
	D3DXVec3TransformCoord(&m_SpotLight.posW , &m_SpotLight.posW , &Ry);
	m_SpotLight.dirW = m_target - m_SpotLight.posW; D3DXVec3Normalize(&m_SpotLight.dirW, &m_SpotLight.dirW); // calculate unit direction vector
	m_up.x = 0.0f; m_up.y = 1.0f; m_up.z = 0.0f; 
	D3DXVec3TransformCoord(&m_up, &m_up, &Ry);
	GetCorrectUp(&m_target, &m_SpotLight.posW, &m_up); // Normalize the up vector to be tangent to direction (with correct rotation)
	D3DXMatrixLookAtLH(&m_lightView, &m_SpotLight.posW, &m_target, &m_up);

	// Rebuild View*Projection matrix
	m_LightVP =  m_lightView * m_lightProj;
} 