#pragma once
#include <d3d11_1.h>
#include <directxmath.h>
#include "structs.h"

class Camera
{
protected:
	// Attributes to store the camera position
	XMFLOAT3 m_Position;
	XMFLOAT3 m_LookVec;
	XMFLOAT3 m_UpVec;
	XMFLOAT3 m_RightVec;
	
	// Attributes to hold the window information and the near and far depth values 
	FLOAT m_WindowWidth;
	FLOAT m_WindowHeight;
	FLOAT m_NearDepth;
	FLOAT m_FarDepth;
	
	// Attributes to hold the view and projection matrices which will be passed to the shader
	XMFLOAT4X4 m_View;
	XMFLOAT4X4 m_Proj;
	
public:
	//Constructor and destructor for the camera
	Camera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, XMFLOAT3 right, FLOAT windowWidth, FLOAT windowHeight, 
		FLOAT nearDepth, FLOAT farDepth);
	~Camera();
	
	// Update function to make the current view and projection matrices
	void Update();
	
	// Retrieve and Set functions for the Camera Eye
	inline void SetPosition(const XMFLOAT3& position) { m_Position = position; }
	inline XMVECTOR GetPosition() const { return XMLoadFloat3(&m_Position); }

	// Retrieve and Set functions for the Camera At
	inline void SetAt(const XMFLOAT3& at) { m_LookVec = at; }
	inline XMVECTOR GetAt() const { return XMLoadFloat3(&m_LookVec); }

	// Retrieve and Set functions for the Camera Up
	inline void SetUp(const XMFLOAT3& up) { m_UpVec = up; }
	inline XMVECTOR GetUp() const { return XMLoadFloat3(&m_UpVec); }
	
	// Functions to retrieve View, Projection, and the ViewProjection matrices
	inline XMMATRIX GetViewMatrix() const { return XMLoadFloat4x4(&m_View); }
	inline XMMATRIX GetProjMatrix() const { return XMLoadFloat4x4(&m_Proj); }
	inline XMMATRIX GetViewProjMatrix() const; // update

	// A function to reshape the camera volume if the window is resized.
	void Reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);
};

