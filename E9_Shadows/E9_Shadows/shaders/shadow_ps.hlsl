Texture2D shaderTexture : register(t0);
Texture2D depthMapTexture1 : register(t1);
Texture2D depthMapTexture2 : register(t2);
Texture2D heightMap : register(t3);

SamplerState diffuseSampler : register(s0);
SamplerState shadowSampler : register(s1);
SamplerState sampler0 : register(s2);

cbuffer LightBuffer : register(b0)
{
    float4 ambient[2];
    float4 diffuse[2];
    float4 direction[2];
    float4 position[2];
};

cbuffer MeshBuffer : register(b1)
{
    float meshType;
    float3 padding;
}

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float4 lightViewPos : TEXCOORD1;
    float4 lightViewPos2 : TEXCOORD2;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate(diffuse * intensity);
    return colour;
}

// Is the gemoetry in our shadow map
bool hasDepthData(float2 uv)
{
    if (uv.x < 0.f || uv.x > 1.f || uv.y < 0.f || uv.y > 1.f)
    {
        return false;
    }
    return true;
}

bool isInShadow(Texture2D sMap, float2 uv, float4 lightViewPosition, float bias)
{
    // Sample the shadow map (get depth of geometry)
    float depthValue = sMap.Sample(shadowSampler, uv).r;
    // Calculate the depth from the light.
    float lightDepthValue = lightViewPosition.z / lightViewPosition.w;
    lightDepthValue -= bias;

    // Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
    if (lightDepthValue < depthValue)
    {
        return false;
    }
    return true;
}

float2 getProjectiveCoords(float4 lightViewPosition)
{
    // Calculate the projected texture coordinates.
    float2 projTex = lightViewPosition.xy / lightViewPosition.w;
    projTex *= float2(0.5, -0.5);
    projTex += float2(0.5f, 0.5f);
    return projTex;
}

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

float4 main(InputType input) : SV_TARGET
{
    float shadowMapBias = 0.005;
    float4 colour = float4(0.f, 0.f, 0.f, 1.f);
    float4 textureColour = shaderTexture.Sample(diffuseSampler, input.tex);

    if (meshType == 1)
    {
        input.normal = CalculateNormal(input.tex);
    }
    
    // Calculate the projected texture coordinates.
    float2 pTexCoord = getProjectiveCoords(input.lightViewPos);
    float2 pTexCoord2 = getProjectiveCoords(input.lightViewPos2);

    // Shadow test. Is or isn't in shadow
    if (hasDepthData(pTexCoord))
    {

        float finalShadow = 1.0f;

        //shadowMapBias = dot(input.normal, -direction[0]);
        // Has depth map data
        if (isInShadow(depthMapTexture1, pTexCoord, input.lightViewPos, shadowMapBias))
        {
            finalShadow *= 0.0f;
        }

       // shadowMapBias = dot(input.normal, -direction[1]);
        if (isInShadow(depthMapTexture2, pTexCoord2, input.lightViewPos2, shadowMapBias))
        {
            finalShadow *= 0.0f;
        }

        if (finalShadow > 0.0f)
        {
            float4 light1 = calculateLighting(-direction[0], input.normal, diffuse[0]);
            float4 light2 = calculateLighting(-direction[1], input.normal, diffuse[1]);

            colour = light1 + light2;
        }
    }

    colour = saturate(colour + ambient[0]);
    return saturate(colour) * textureColour;
}