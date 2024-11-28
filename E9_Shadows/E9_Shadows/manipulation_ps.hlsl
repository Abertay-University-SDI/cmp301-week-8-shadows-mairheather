Texture2D texture0 : register(t0);
Texture2D heightMap : register(t1);
SamplerState sampler0 : register(s0);

cbuffer BoolBuffer : register(b0)
{
    float normalMap;
    float3 padding;
};

cbuffer LightBuffer : register(b1)
{
    float4 ambient[2];
    float4 diffuse[2];
    float4 position[2];
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
};

float GetHeight(float2 uv)
{
    float height = heightMap.SampleLevel(sampler0, uv, 0).r;
    return height * 10;

}

float3 CalculateNormal(float2 uv)
{
    float uvOffset = 0.001f;

    float heightN = GetHeight(float2(uv.x, uv.y + uvOffset));
    float heightE = GetHeight(float2(uv.x + uvOffset, uv.y));
    float heightS = GetHeight(float2(uv.x, uv.y - uvOffset));
    float heightW = GetHeight(float2(uv.x - uvOffset, uv.y));

    float worldStep = 100 * uvOffset;
    float3 tan = normalize(float3(2.0f * worldStep, heightE - heightW, 0));
    float3 bitan = normalize(float3(0, heightN - heightS, 2.0f * worldStep));

    return normalize(cross(bitan, tan));
}

float4 calculateLighting(float3 lightDirection, float3 normal, float4 ldiffuse)
{
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate(ldiffuse * intensity);
    return colour;
}

float4 main(InputType input) : SV_TARGET
{
    float4 textureColour, finalLightColour, lightColour;
    float3 lightVector;

    if (normalMap == 1)
    {
        input.normal = CalculateNormal(input.tex);
        return float4(input.normal, 1.0);
    }

    else
    {
        textureColour = texture0.Sample(sampler0, input.tex);

        for (int i = 0; i < 2; i++)
        {
            input.normal = CalculateNormal(input.tex);
            //lightVector = normalize(float3(position[i].x, position[i].y, position[i].z) - input.worldPosition);
           lightColour = ambient[i] + calculateLighting(position[i], input.normal, diffuse[i]);

            finalLightColour += lightColour;
        }

        return finalLightColour * textureColour;
    }
}