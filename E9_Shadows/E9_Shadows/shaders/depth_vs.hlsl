Texture2D heightMap : register(t0);
SamplerState sampler0 : register(s0);

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
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
    float4 depthPosition : TEXCOORD0;
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

    // Store the position value in a second input value for depth value calculations.
    output.depthPosition = output.position;
	
    return output;
}