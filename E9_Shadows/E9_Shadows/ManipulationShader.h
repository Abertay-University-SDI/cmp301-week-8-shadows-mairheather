#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;

class ManipulationShader : public BaseShader
{
private:
	struct BooleanBufferType
	{
		float normalMap;
		XMFLOAT3 padding;
	};

	struct LightBufferType
	{
		XMFLOAT4 ambient[2];
		XMFLOAT4 diffuse[2];
		XMFLOAT4 position[2];
	};

public:
	ManipulationShader(ID3D11Device* device, HWND hwnd);
	~ManipulationShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* heightMap, bool nrmlMap, Light* lights[2]);

private:
	void initShader(const wchar_t* cs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* booleanBuffer;
	ID3D11Buffer* lightBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* heightmapSample;
};
