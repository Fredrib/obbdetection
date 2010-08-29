// NOTE: keyboard and mouse classes basically used as is. Just wanted quick class to 
// speed up development.  All processing of user input is mine.
// Jonathan Tompson, Summer 2009

// Origional material from: http://www.dhpoware.com/demos/index.html


#include "main.h"

//-----------------------------------------------------------------------------
// keyboard.
//-----------------------------------------------------------------------------

void keyboard::InitKeyboard(void)
{
	if (!create())
    {
        destroy();
		throw std::runtime_error("InitKeyboard: Failed to initialize keyboard!!");
    }
}

keyboard::keyboard()
{
    m_pDirectInput = NULL;
    m_pDevice = NULL;
    m_pCurrKeyStates = m_keyStates[0];
    m_pPrevKeyStates = m_keyStates[1];
    m_lastChar = 0;
}

keyboard::~keyboard()
{
    destroy();
}

bool keyboard::create()
{
    HINSTANCE hInstance = GetModuleHandle(0);
    HWND hWnd = GetForegroundWindow();

    if (FAILED(DirectInput8Create(hInstance, DIRECTINPUT_VERSION,
            IID_IDirectInput8, reinterpret_cast<void**>(&m_pDirectInput), 0)))
        return false;

    if (FAILED(m_pDirectInput->CreateDevice(GUID_SysKeyboard, &m_pDevice, 0)))
        return false;

    if (FAILED(m_pDevice->SetDataFormat(&c_dfDIKeyboard)))
        return false;

	// SET TO BACKGROUND IF WE'RE DEBUGGING WITH VISUAL STUDIO --> GETS RID OF ANNOYING ACQUIRE FAILURE BUG
#ifdef _DEBUG
	if (FAILED(m_pDevice->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
        return false;
#else
    if (FAILED(m_pDevice->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
        return false;
#endif

    if (FAILED(m_pDevice->Acquire()))
        return false;

    memset(m_keyStates, 0, sizeof(m_keyStates));
    return true;
}

void keyboard::destroy()
{
    if (m_pDevice)
    {
        m_pDevice->Unacquire();
        m_pDevice->Release();
        m_pDevice = 0;
    }

    if (m_pDirectInput)
    {
        m_pDirectInput->Release();
        m_pDirectInput = 0;
    }
}

void keyboard::handleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CHAR:
        m_lastChar = static_cast<char>(wParam);
        break;

    default:
        break;
    }
}

void keyboard::Update()
{
    if (!m_pDirectInput || !m_pDevice)
    {
		g_app->SetError(L"No keyboard devices found in Update()!!");
		g_app->m_bRunningD3D = false;
		return;
    }

    HRESULT hr = 0;
    unsigned char *pTemp = m_pPrevKeyStates;

    m_pPrevKeyStates = m_pCurrKeyStates;
    m_pCurrKeyStates = pTemp;
    
    while (true)
    {
        if(FAILED(m_pDevice->GetDeviceState(256, m_pCurrKeyStates)))
        {
            if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
            {
                if (FAILED(m_pDevice->Acquire()))
                    return;
            }
        }
        else
        {
            break;
        }
    }
}

//-----------------------------------------------------------------------------
// mouse.
//-----------------------------------------------------------------------------

const float mouse::WEIGHT_MODIFIER = 0.2f;
const int mouse::HISTORY_BUFFER_SIZE = 10;

void mouse::InitMouse(void)
{
	if (!create())
    {
        destroy();
		throw std::runtime_error("InitMouse: Failed to initialize mouse!!");
    }
}

mouse::mouse()
{
	m_pDirectInput = NULL;
	m_pDevice = NULL;
    m_pDevice = 0;
    m_pCurrmouseState = &m_mouseStates[0];
    m_pPrevmouseState = &m_mouseStates[1];
    
    m_deltamouseX = 0.0f;
    m_deltamouseY = 0.0f;
    m_deltamouseWheel = 0.0f;
    
    m_weightModifier = WEIGHT_MODIFIER;
    m_enableFiltering = true;

    m_historyBufferSize = HISTORY_BUFFER_SIZE;
	// m_historyBuffer.resize(m_historyBufferSize); // NO LONGER USING VECTOR
}

mouse::~mouse()
{
    destroy();
}

bool mouse::create()
{
    HINSTANCE hInstance = GetModuleHandle(0);
    HWND hWnd = GetForegroundWindow();

    if (FAILED(DirectInput8Create(hInstance, DIRECTINPUT_VERSION,
            IID_IDirectInput8, reinterpret_cast<void**>(&m_pDirectInput), 0)))
        return false;

    if (FAILED(m_pDirectInput->CreateDevice(GUID_SysMouse, &m_pDevice, 0)))
        return false;

    if (FAILED(m_pDevice->SetDataFormat(&c_dfDIMouse)))
        return false;

    if (FAILED(m_pDevice->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))) // Jonno: changed DISCL_FOREGROUND | DISCL_EXCLUSIVE
        return false;

    if (FAILED(m_pDevice->Acquire()))
        return false;

    ZeroMemory(&m_mouseStates[0], 2*sizeof(DIMOUSESTATE)); // note: LOTS OF DEBUGGING HERE WITH OVERLAPPING MEMORY SPACES!
	for(int i = 0; i < m_historyBufferSize*2; i+=2)
	{
		m_historyBuffer[i] = 0;
		m_historyBuffer[i+1] = 0;
	}

    m_mouseIndex = 0;
	m_mouseMovement[0] = 0.0f; m_mouseMovement[1] = 0.0f;
    m_mouseMovement[2] = 0.0f; m_mouseMovement[3] = 0.0f;

    return true;
}

void mouse::destroy()
{
    if (m_pDevice)
    {
        m_pDevice->Unacquire();
        m_pDevice->Release();
        m_pDevice = 0;
    }

    if (m_pDirectInput)
    {
        m_pDirectInput->Release();
        m_pDirectInput = 0;
    }
}

void mouse::Update()
{
    if (!m_pDirectInput || !m_pDevice)
    {
		g_app->SetError(L"No mouse devices found in Update()!!");
		g_app->m_bRunningD3D = false;
		return;
    }

    HRESULT hr = 0;
    DIMOUSESTATE *pTemp = m_pPrevmouseState;

    m_pPrevmouseState = m_pCurrmouseState;
    m_pCurrmouseState = pTemp;

    while (true)
    {
        hr = m_pDevice->GetDeviceState(sizeof(DIMOUSESTATE), m_pCurrmouseState);

        if (FAILED(hr))
        {
            if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
            {
                if (FAILED(m_pDevice->Acquire()))
                    return;
            }
        }
        else
        {
            break;
        }
    }

    if (m_enableFiltering)
    {
        performmouseFiltering(
            static_cast<float>(m_pCurrmouseState->lX),
            static_cast<float>(m_pCurrmouseState->lY));

        performmouseSmoothing(m_deltamouseX, m_deltamouseY);
    }
    else
    {
        m_deltamouseX = static_cast<float>(m_pCurrmouseState->lX);
        m_deltamouseY = static_cast<float>(m_pCurrmouseState->lY);
    }

    if (m_pCurrmouseState->lZ > 0)
        m_deltamouseWheel = 1.0f;           // wheel rolled forwards
    else if (m_pCurrmouseState->lZ < 0)
        m_deltamouseWheel = -1.0f;          // wheel rolled backwards
    else
        m_deltamouseWheel = 0.0f;           // wheel hasn't moved
}

void mouse::performmouseSmoothing(float x, float y)
{
    // Average the mouse movement across a couple of frames to smooth
    // out mouse movement.

    m_mouseMovement[2*m_mouseIndex] = x;
    m_mouseMovement[2*m_mouseIndex+1] = y;

    m_deltamouseX = (m_mouseMovement[0] + m_mouseMovement[2]) * 0.5f;
    m_deltamouseY = (m_mouseMovement[1] + m_mouseMovement[3]) * 0.5f;

    m_mouseIndex ^= 1;
    m_mouseMovement[2*m_mouseIndex] = 0.0f;
    m_mouseMovement[2*m_mouseIndex+1] = 0.0f;
}

void mouse::performmouseFiltering(float x, float y)
{
    // Filter the relative mouse movement based on a weighted sum of the mouse
    // movement from previous frames to ensure that the mouse movement this
    // frame is smooth.
    //
    // For further details see:
    //  Nettle, Paul "Smooth mouse Filtering", flipCode's Ask Midnight column.
    //  http://www.flipcode.com/cgi-bin/fcarticles.cgi?show=64462

    for (int i = (2*m_historyBufferSize) - 1; i > 1; i = i-2)
    {
        m_historyBuffer[i] = m_historyBuffer[i - 2];
        m_historyBuffer[1-1] = m_historyBuffer[i - 3];
    }

    m_historyBuffer[0] = x;
    m_historyBuffer[1] = y;

    float averageX = 0.0f;
    float averageY = 0.0f;
    float averageTotal = 0.0f;
    float currentWeight = 1.0f;
	
	for (int i = 0; i < m_historyBufferSize*2; i = i + 2)
    {
        averageX += m_historyBuffer[i] * currentWeight;
        averageY += m_historyBuffer[i+1] * currentWeight;
        averageTotal += 1.0f * currentWeight;
        currentWeight *= m_weightModifier;
    }


    m_deltamouseX = averageX / averageTotal;
    m_deltamouseY = averageY / averageTotal;
}

void mouse::setWeightModifier(float weightModifier)
{
    m_weightModifier = weightModifier;
}

void mouse::smoothmouse(bool smooth)
{
    m_enableFiltering = smooth;
}