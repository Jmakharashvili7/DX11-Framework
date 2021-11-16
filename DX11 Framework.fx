//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
    float4 DiffuseMtrl;
    float4 DiffuseLight;
    float3 LightVecW;
    float4 SpecularMtrl;
    float4 SpecularLight;
    float SpecularPower;
    float3 EyePosW; // camera position in world space
}

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : NORMAL;
    float3 PosW : POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
// Vertex Shader - Transforms the position of the object to projection space and transforms
// the normals to world space
//------------------------------------------------------------------------------------

VS_OUTPUT VS(float4 Pos : POSITION, float3 NormalL : NORMAL)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;

    // output.Pos is currently the position in world space
    output.Pos = mul(Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);

    // Convert from local space to world space
    // W component of vector is 0 as vectors cannot be translated
    float3 normalW = mul(float4(NormalL, 0.0f), World).xyz;
    output.Norm = normalize(normalW);

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader - Calculate the color of the object per pixel using local illumiation model
//--------------------------------------------------------------------------------------
float4 PS( VS_OUTPUT input ) : SV_Target
{  
    input.Norm = normalize(input.Norm);

    // Compute the vector from the vertex to the eye position. 
    float3 toEye = normalize(EyePosW - input.Pos.xyz);
    
    // Compute Colour
    float diffuseAmount = max(dot(LightVecW, input.Norm), 0.0f);

    // Compute the reflection vector
    float3 r = reflect(-LightVecW, input.Norm);

    // Determine how much specular light makes it into the eye
    float specularAmount = pow(max(dot(r, toEye), 0.0f), 100.0f);


    // Compute diffuse and ambient lighting
    float ambient = 0.05;
    float3 diffuse = diffuseAmount * (DiffuseMtrl * DiffuseLight).rgb;

    // Calculate specular lighting
    float3 specular = specularAmount * (SpecularMtrl * SpecularLight).rgb;

    // Sum all the terms together
    float4 Color;
    Color.rgb = ambient + diffuse + specular;
    Color.a = DiffuseMtrl.a;

    // return the color
    return Color;
}
