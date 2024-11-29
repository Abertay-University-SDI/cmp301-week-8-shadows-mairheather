// depth shader.cpp
#include "depthshader.h"

DepthShader::DepthShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"depth_vs.cso", L"depth_ps.cso");
	loadVertexShader(L"depth_vs.cso");
	loadPixelShader(L"depth_ps.cso");
}

DepthShader::~DepthShader()
{
	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void DepthShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;

	// Load (+ compile) shader files

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);
	
	D3D11_SAMPLER_DESC heightmapDesc;
	// Create a texture sampler state description.
	heightmapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	heightmapDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	heightmapDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	heightmapDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	heightmapDesc.MipLODBias = 0.0f;
	heightmapDesc.MaxAnisotropy = 1;
	heightmapDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	heightmapDesc.MinLOD = 0;
	heightmapDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&heightmapDesc, &heightmapSample);

	D3D11_BUFFER_DESC meshBufferDesc;
	meshBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	meshBufferDesc.ByteWidth = sizeof(MeshBufferType);
	meshBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	meshBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	meshBufferDesc.MiscFlags = 0;
	meshBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&meshBufferDesc, NULL, &meshBuffer);

}

void DepthShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView* heightMap, float t, float dt)
{
	loadVertexShader(L"depth_vs.cso");
	loadPixelShader(L"depth_ps.cso");
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	
	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);

	// Lock the constant buffer so it can be written to.
	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	MeshBufferType* meshPtr;
	deviceContext->Map(meshBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	meshPtr = (MeshBufferType*)mappedResource.pData;
	meshPtr->type = t;
	meshPtr->time = dt;
	meshPtr->padding = XMFLOAT2(0.0f, 0.0f);
	deviceContext->Unmap(meshBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &meshBuffer);

	deviceContext->VSSetShaderResources(0, 1, &heightMap);
	deviceContext->VSSetSamplers(0, 1, &heightmapSample);
}

