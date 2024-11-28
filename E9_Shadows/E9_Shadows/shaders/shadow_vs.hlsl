Texture2D heightMap : register(t0);
SamplerState sampler0 : register(s0);

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    matrix lightViewMatrix;
    matrix lightProjectionMatrix;
    matrix lightViewMatrix2;
    matrix lightProjectionMatrix2;
};

cbuffer MeshBuffer : register(b1)
{
    float meshType;
    float3 padding;
}

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
    float4 lightViewPos : TEXCOORD1;
    float4 lightViewPos2 : TEXCOORD2;
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

    if (meshType == 1)
    {
        // Displace the vertex position based on heightmap
        float heightValue = GetHeight(input.tex);
        input.position.y += heightValue;
    }

    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Calculate the position of the vertice as viewed by the light source.
    output.lightViewPos = mul(input.position, worldMatrix);
    output.lightViewPos = mul(output.lightViewPos, lightViewMatrix);
    output.lightViewPos = mul(output.lightViewPos, lightProjectionMatrix);

    output.lightViewPos2 = mul(input.position, worldMatrix);
    output.lightViewPos2 = mul(output.lightViewPos2, lightViewMatrix2);
    output.lightViewPos2 = mul(output.lightViewPos2, lightProjectionMatrix2);

    output.tex = input.tex;
    output.normal = mul(input.normal, (float3x3)worldMatrix);
    output.normal = normalize(output.normal);

    return output;
}