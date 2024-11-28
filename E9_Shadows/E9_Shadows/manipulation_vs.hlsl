Texture2D heightMap : register(t0);
SamplerState sampler0 : register(s0);

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
};

// Calculate height based on UV coordinates
float GetHeight(float2 uv)
{
    // Sample the heightmap texture and return the height value.
    float height = heightMap.SampleLevel(sampler0, uv, 0).r;
    return height * 10;
}


OutputType main(InputType input)
{
    OutputType output;

    // Displace the vertex position based on heightmap
    float heightValue = GetHeight(input.tex);
    input.position.y += heightValue;

    // Apply world, view, and projection matrix transformations to the position
    output.position = mul(mul(mul(input.position, worldMatrix), viewMatrix), projectionMatrix);

    // Pass texture coordinates to the pixel shader
    output.tex = input.tex;

    output.worldPosition = mul(input.position, worldMatrix).xyz;

    return output;
}