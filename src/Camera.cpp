#include "Camera.h"

Camera::Camera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, XMFLOAT3 right, FLOAT windowWidth, FLOAT windowHeight, 
	FLOAT nearDepth, FLOAT farDepth) 
	: m_Position(position), m_LookVec(at), m_UpVec(up), m_RightVec(right), m_WindowHeight(windowHeight)
	, m_WindowWidth(windowWidth), m_NearDepth(nearDepth), m_FarDepth(farDepth)
{
}

Camera::~Camera()
{

}

void Camera::Update()
{
	// Convert from float3 to vector
	XMVECTOR eye = XMLoadFloat3(&m_Position);
	XMVECTOR at  = XMLoadFloat3(&m_LookVec);
	XMVECTOR up  = XMLoadFloat3(&m_UpVec);

	// Initialize the view matrix
	XMStoreFloat4x4(&m_View, XMMatrixLookToLH(eye, at, up));

    // Initialize the projection matrix
	XMStoreFloat4x4(&m_Proj, XMMatrixPerspectiveFovLH(XM_PIDIV2, m_WindowWidth / m_WindowHeight, m_NearDepth, m_FarDepth));
}

inline XMMATRIX Camera::GetViewProjMatrix() const
{
	// convert the float4x4 to matrix
	XMMATRIX viewMatrix, projMatrix, combinedMatrix;
	viewMatrix = XMLoadFloat4x4(&m_View);
	projMatrix = XMLoadFloat4x4(&m_Proj);

	// store the combined matrix
	combinedMatrix = viewMatrix * projMatrix;

	return combinedMatrix;
}

void Camera::Reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
{
	m_WindowWidth = windowHeight;
	m_WindowHeight = windowWidth;
	m_NearDepth = nearDepth;
	m_FarDepth = farDepth;
}
