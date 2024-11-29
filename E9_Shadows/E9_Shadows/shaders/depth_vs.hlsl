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
    float time;
    float2 padding;
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

float3 GerstnerWave(float steep, float wavelen, float2 dir, float2 xzPos)
{
    float k = 2 * 3.14159 / wavelen;
    float c = sqrt(9.8 / k);
    float2 d = normalize(dir);
    float f = k * (dot(d, xzPos) - c * time);
    float a = steep / k;
    
    return float3(d.x * (a * cos(f)), a * sin(f), d.y * (a * cos(f)));
}

float3 GetTangent(float steep, float wavelen, float2 dir, float3 tan, float2 xzPos)
{
    float k = 2 * 3.14159 / wavelen;
    float c = sqrt(9.8 / k);
    float2 d = normalize(dir);
    float f = k * (dot(d, xzPos) - c * time);
    float a = steep / k;
    
    tan += float3(1 - d.x * d.x * (steep * sin(f)), d.x * (steep * cos(f)), -d.x * d.y * (steep * sin(f)));
    
    return tan;
}

float3 GetBiormal(float steep, float wavelen, float2 dir, float3 binorm, float2 xzPos)
{
    float k = 2 * 3.14159 / wavelen;
    float c = sqrt(9.8 / k);
    float2 d = normalize(dir);
    float f = k * (dot(d, xzPos) - c * time);
    float a = steep / k;
    
    binorm += float3(-d.x * d.y * (steep * sin(f)), d.y * (steep * cos(f)), 1 - d.y * d.y * (steep * sin(f)));
    
    return binorm;
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
    
    if (meshType == 2)
    {
        float2 direction1 = float2(1, 0);
        float2 direction2 = float2(1, 1);
    
        float3 binormal = float3(0, 0, 0);
        float3 tangent = float3(0, 0, 0);
        input.position += float4(GerstnerWave(0.2f, 10, direction1, input.position.xz), 1);
        binormal = GetBiormal(0.2f, 10, direction1, binormal, input.position.xz);
        tangent = GetTangent(0.2f, 10, direction1, tangent, input.position.xz);
    
        input.position += float4(GerstnerWave(0.2f, 10, direction1, input.position.xz), 1);
        binormal = GetBiormal(0.2f, 10, direction2, binormal, input.position.xz);
        tangent = GetTangent(0.2f, 10, direction2, tangent, input.position.xz);
    }

    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Store the position value in a second input value for depth value calculations.
    output.depthPosition = output.position;
	
    return output;
}