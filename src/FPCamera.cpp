#include "FPCamera.h"

FP_Camera::FP_Camera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, XMFLOAT3 right, 
	FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth) :
	Camera( position, at, up, right, windowWidth, windowHeight, nearDepth, farDepth)
{
}

void FP_Camera::UpdateViewMatrix()
{
	// Load the vectors as XMVectors so we can use the built in math
	XMVECTOR R = XMLoadFloat3(&m_RightVec);
	XMVECTOR U = XMLoadFloat3(&m_UpVec);
	XMVECTOR L = XMLoadFloat3(&m_LookVec);
	XMVECTOR P = XMLoadFloat3(&m_Position);

	// normalize the Look at vector
	L = XMVector3Normalize(L);

	// We get the cross product of look and right vector to
	// make sure the vector is orthogonal and normalize it.
	U = XMVector3Normalize(XMVector3Cross(L, R));

	// Both Up and Look at vectors are normal and orthogonal,
	// thus we just need to calcualte the cross product and
	// the resulting vector which is orthogonal to them will 
	// end up being the right vector
	R = XMVector3Cross(U, L);

	//Fill in the view matrix entries
	float x = -XMVectorGetX(XMVector3Dot(P, R));
	float y = -XMVectorGetX(XMVector3Dot(P, U));
	float z = -XMVectorGetX(XMVector3Dot(P, L));

	// We dont need any more math so store the vectors as floats 
	XMStoreFloat3(&m_RightVec, R);
	XMStoreFloat3(&m_LookVec, L);
	XMStoreFloat3(&m_UpVec, U);

	// Fill in the right vec for the view matrix
	m_View(0, 0) = m_RightVec.x;
	m_View(1, 0) = m_RightVec.y;
	m_View(2, 0) = m_RightVec.z;
	m_View(3, 0) = x;

	// fill in the Up vec for the view matrix
	m_View(0, 1) = m_UpVec.x;
	m_View(1, 1) = m_UpVec.y;
	m_View(2, 1) = m_UpVec.z;
	m_View(3, 1) = y;

	// fill in the Look at vec for the view matrix
	m_View(0, 2) = m_LookVec.x;
	m_View(1, 2) = m_LookVec.y;
	m_View(2, 2) = m_LookVec.z;
	m_View(3, 2) = z;

	// fill in the last row in the view matrix
	m_View(0, 3) = 0.0f;
	m_View(1, 3) = 0.0f;
	m_View(2, 3) = 0.0f;
	m_View(3, 3) = 1.0f; 
}

void FP_Camera::Strafe(float force)
{
	XMVECTOR s = XMVectorReplicate(force);
	XMVECTOR r = XMLoadFloat3(&m_RightVec);
	XMVECTOR p = XMLoadFloat3(&m_Position);

	XMStoreFloat3(&m_Position, XMVectorMultiplyAdd(s, r, p));
}

void FP_Camera::Walk(float force)
{
	XMVECTOR s = XMVectorReplicate(force);
	XMVECTOR l = XMLoadFloat3(&m_LookVec);
	XMVECTOR p = XMLoadFloat3(&m_Position);

	// multiplies s by l and adds it to p
	XMStoreFloat3(&m_Position, XMVectorMultiplyAdd(s, l, p));
}

void FP_Camera::RotateP(float angle)
{
	// Roate up and look vector about the right vector

	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&m_RightVec), angle);

	XMStoreFloat3(&m_UpVec, XMVector3TransformNormal(XMLoadFloat3(&m_UpVec), R));
	XMStoreFloat3(&m_LookVec, XMVector3TransformNormal(XMLoadFloat3(&m_LookVec), R));
}

void FP_Camera::RotateY(float angle)
{
	// Rotate the basis vectors about the world y-axis
	XMMATRIX R = XMMatrixRotationY(angle);

	XMStoreFloat3(&m_RightVec, XMVector3TransformNormal(XMLoadFloat3(&m_RightVec), R));
	XMStoreFloat3(&m_UpVec, XMVector3TransformNormal(XMLoadFloat3(&m_UpVec), R));
	XMStoreFloat3(&m_LookVec, XMVector3TransformNormal(XMLoadFloat3(&m_LookVec), R));
}