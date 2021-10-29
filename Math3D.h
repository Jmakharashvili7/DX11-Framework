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

	void NormalAvarage(SimpleVertex vertices[], UINT indices[], UINT numTriangles)
	{
		// For each triangle in the mesh:
		for (UINT i = 0; i < numTriangles; ++i)
		{
			// indices of the ith triangle
			UINT i0 = indices[i * 3 + 0];
			UINT i1 = indices[i * 3 + 1];
			UINT i2 = indices[i * 3 + 2];
			// vertices of ith triangle
			XMVECTOR v0 = XMLoadFloat3(&vertices[i0]);
			XMVECTOR v1 = XMLoadFloat3(&vertices[i1]);
			XMVECTOR v2 = XMLoadFloat3(&vertices[i2]);
			// compute face normal
			XMVECTOR e0 = v1 - v0;
			XMVECTOR e1 = v2 - v0;
			XMVECTOR faceNormal = XMVector3Cross(e0, e1);
			// This triangle shares the following three vertices,
			// so add this face normal into the average of these
			// vertex normals.
			vertices[i0].normal += faceNormal;
			vertices[i1].normal += faceNormal;
			vertices[i2].normal += faceNormal;
		}

		// For each vertex v, we have summed the face normals of all
		// the triangles that share v, so now we just need to normalize.
		for (UINT i = 0; i < mNumVertices; ++i)
			mVertices[i].normal = Normalize(&mVertices[i].normal));
	}

}