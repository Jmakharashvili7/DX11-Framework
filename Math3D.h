#pragma once
#include "application.h"

namespace Math3D {

	void ComputeNormal(const XMVECTOR& p0,
	                   const XMVECTOR& p1,
		               const XMVECTOR& p2,
		               XMVECTOR& out)
	{
		XMVECTOR u = p1 - p0;
		XMVECTOR v = p2 - p0;
		out = XMVector3Cross(u, v);
		out = XMVector3Normalize(out);
	}

	void NormalAvarage(SimpleVertex vertices[], const UINT indices[], UINT numTriangles)
	{
		// For each triangle in the mesh:
		for (UINT i = 0; i < numTriangles; ++i)
		{
			// indices of the ith triangle
			UINT i0 = indices[i * 3 + 0];
			UINT i1 = indices[i * 3 + 1];
			UINT i2 = indices[i * 3 + 2];

			// vertices of ith triangle
			XMVECTOR v0 = XMLoadFloat3(&vertices[i0].pos);
			XMVECTOR v1 = XMLoadFloat3(&vertices[i1].pos);
			XMVECTOR v2 = XMLoadFloat3(&vertices[i2].pos);

			// compute face normal
			XMVECTOR e0 = v1 - v0;
			XMVECTOR e1 = v2 - v0;
			XMVECTOR faceNormal = XMVector3Cross(e0, e1);

			// This triangle shares the following three vertices,
			// so add this face normal into the average of these
			// vertex normals.
			XMFLOAT3 normal0, normal1, normal2;

			XMStoreFloat3(&normal0, faceNormal);
			XMStoreFloat3(&normal1, faceNormal);
			XMStoreFloat3(&normal2, faceNormal);

			vertices[i0].normal.x += normal0.x;
			vertices[i0].normal.y += normal0.y;
			vertices[i0].normal.z += normal0.z;

			vertices[i1].normal.x += normal1.x;
			vertices[i1].normal.y += normal1.y;
			vertices[i1].normal.z += normal1.z;

			vertices[i2].normal.x += normal2.x;
			vertices[i2].normal.y += normal2.y;
			vertices[i2].normal.z += normal2.z;
		}

		// For each vertex v, we have summed the face normals of all
		// the triangles that share v, so now we just need to normalize.
		for (UINT i = 0; i < 8; ++i)
		{
			XMVECTOR temp = XMLoadFloat3(&vertices[i].normal);
			temp = XMVector3Normalize(temp);
			XMStoreFloat3(&vertices[i].normal, temp);
		}
	}
}