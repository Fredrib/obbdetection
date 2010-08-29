// NOTE: Below code was basically used as is, with some changes to implimentation 
// of "FIRST_PERSON" mode.  Just wanted quick class to speed up development.
// Jonathan Tompson, Summer 2009

// Origional material from: http://www.dhpoware.com/demos/index.html


#ifndef camera_h
#define camera_h

#include "main.h"

//-----------------------------------------------------------------------------
// A general purpose 6DoF (six degrees of freedom) quaternion based camera.
//
// This camera class supports 2 different behaviors:
// first person mode and flight mode.
//
// First person mode only allows 5DOF (x axis movement, y axis movement, z axis
// movement, yaw, and pitch) and movement is always parallel to the world x-z
// (ground) plane.
//
// Flight mode supports 6DoF. This is the camera class' default behavior.
//
// This camera class allows the camera to be moved in 2 ways: using fixed
// step world units, and using a supplied velocity and acceleration. The former
// simply moves the camera by the specified amount. To move the camera in this
// way call one of the move() methods. The other way to move the camera
// calculates the camera's displacement based on the supplied velocity,
// acceleration, and elapsed time. To move the camera in this way call the
// updatePosition() method.
//-----------------------------------------------------------------------------

class camera
{
public:
    enum cameraBehavior
    {
        camera_BEHAVIOR_FIRST_PERSON,
        camera_BEHAVIOR_FLIGHT
    };

    camera();
    ~camera();

    void lookAt(const D3DXVECTOR3 &target);
    void lookAt(const D3DXVECTOR3 &eye, const D3DXVECTOR3 &target, const D3DXVECTOR3 &up);
    void move(float dx, float dy, float dz);
    void move(const D3DXVECTOR3 &direction, const D3DXVECTOR3 &amount);
    void perspective(float fovx, float aspect, float znear, float zfar);
    void rotate(float headingDegrees, float pitchDegrees, float rollDegrees);
    void rotateSmoothly(float headingDegrees, float pitchDegrees, float rollDegrees);
    void updatePosition(const D3DXVECTOR3 &direction, float elapsedTimeSec);

    // Getter methods.

    const D3DXVECTOR3 &getAcceleration() const;
    cameraBehavior getBehavior() const;
    const D3DXVECTOR3 &getCurrentVelocity() const;
    const D3DXQUATERNION &getOrientation() const;
    const D3DXVECTOR3 &getPosition() const;
    float getRotationSpeed() const;
    const D3DXMATRIX &getProjectionMatrix() const;
    const D3DXVECTOR3 &getVelocity() const;
    const D3DXVECTOR3 &getViewDirection() const;
    const D3DXMATRIX &getViewMatrix() const;
    const D3DXVECTOR3 &getXAxis() const;
    const D3DXVECTOR3 &getYAxis() const;
    const D3DXVECTOR3 &getZAxis() const;
    
    // Setter methods.

    void setAcceleration(const D3DXVECTOR3 &acceleration);
    void setAcceleration(float x, float y, float z);
    void setBehavior(cameraBehavior behavior);
    void setCurrentVelocity(const D3DXVECTOR3 &currentVelocity);
    void setCurrentVelocity(float x, float y, float z);
    void setOrientation(const D3DXQUATERNION &orientation);
    void setPosition(const D3DXVECTOR3 &eye);
    void setPosition(float x, float y, float z);
    void setRotationSpeed(float rotationSpeed);
    void setVelocity(const D3DXVECTOR3 &velocity);
    void setVelocity(float x, float y, float z);
        
private:
    void rotateFirstPerson(float headingDegrees, float pitchDegrees);
    void rotateFlight(float headingDegrees, float pitchDegrees, float rollDegrees);
    void updateVelocity(const D3DXVECTOR3 &direction, float elapsedTimeSec);
    void updateViewMatrix();
    
    static const float DEFAULT_ROTATION_SPEED;
    static const float DEFAULT_FOVX;   
    static const float DEFAULT_ZFAR;
    static const float DEFAULT_ZNEAR;
    static const D3DXVECTOR3 WORLD_XAXIS;
    static const D3DXVECTOR3 WORLD_YAXIS;
    static const D3DXVECTOR3 WORLD_ZAXIS;

    cameraBehavior m_behavior;
    float m_accumPitchDegrees;
    float m_rotationSpeed;
    float m_fovx;
    float m_aspectRatio;
    float m_znear;
    float m_zfar;
    D3DXVECTOR3 m_eye;
    D3DXVECTOR3 m_xAxis;
    D3DXVECTOR3 m_yAxis;
    D3DXVECTOR3 m_zAxis;
    D3DXVECTOR3 m_viewDir;
    D3DXVECTOR3 m_acceleration;
    D3DXVECTOR3 m_currentVelocity;
    D3DXVECTOR3 m_velocity;
    D3DXQUATERNION m_orientation;
    D3DXMATRIX m_viewMatrix;
    D3DXMATRIX m_projMatrix;
};

//-----------------------------------------------------------------------------

inline const D3DXVECTOR3 &camera::getAcceleration() const
{ return m_acceleration; }

inline camera::cameraBehavior camera::getBehavior() const
{ return m_behavior; }

inline const D3DXVECTOR3 &camera::getCurrentVelocity() const
{ return m_currentVelocity; }

inline const D3DXQUATERNION &camera::getOrientation() const
{ return m_orientation; }

inline const D3DXVECTOR3 &camera::getPosition() const
{ return m_eye; }

inline float camera::getRotationSpeed() const
{ return m_rotationSpeed; }

inline const D3DXMATRIX &camera::getProjectionMatrix() const
{ return m_projMatrix; }

inline const D3DXVECTOR3 &camera::getVelocity() const
{ return m_velocity; }

inline const D3DXVECTOR3 &camera::getViewDirection() const
{ return m_viewDir; }

inline const D3DXMATRIX &camera::getViewMatrix() const
{ return m_viewMatrix; }

inline const D3DXVECTOR3 &camera::getXAxis() const
{ return m_xAxis; }

inline const D3DXVECTOR3 &camera::getYAxis() const
{ return m_yAxis; }

inline const D3DXVECTOR3 &camera::getZAxis() const
{ return m_zAxis; }

#endif