#include "ManipulationShader.h"

ManipulationShader::ManipulationShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"manipulation_vs.cso", L"manipulation_ps.cso");
}

ManipulationShader::~ManipulationShader()
{
	// Release the sampler state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

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

	// Release the light constant buffer.
	if (booleanBuffer)
	{
		booleanBuffer->Release();
		booleanBuffer = 0;
	}

	// Release the light constant buffer.
	if (lightBuffer)
	{
		lightBuffer->Release();
		lightBuffer = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void ManipulationShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_SAMPLER_DESC heightmapDesc;
	D3D11_BUFFER_DESC boolBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;

	// Load (+compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

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

	// Setup light buffer
	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	boolBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	boolBufferDesc.ByteWidth = sizeof(BooleanBufferType);
	boolBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	boolBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	boolBufferDesc.MiscFlags = 0;
	boolBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&boolBufferDesc, NULL, &booleanBuffer);

	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);
}


void ManipulationShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* heightMap, bool nrmlMap, Light* lights[2])
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	XMMATRIX tworld, tview, tproj;

	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	BooleanBufferType* boolPtr;
	deviceContext->Map(booleanBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	boolPtr = (BooleanBufferType*)mappedResource.pData;
	boolPtr->normalMap = nrmlMap;
	boolPtr->padding = XMFLOAT3(0.0f, 0.0f, 0.0f);
	deviceContext->Unmap(booleanBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &booleanBuffer);

	LightBufferType* lightPtr;
	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;

	for (int i = 0; i < 2; i++)
	{
		lightPtr->ambient[i] = lights[i]->getAmbientColour();
		lightPtr->diffuse[i] = lights[i]->getDiffuseColour();
		lightPtr->position[i] = XMFLOAT4(lights[i]->getDirection().x, lights[i]->getDirection().y, lights[i]->getDirection().z, 1.0f);
	}

	deviceContext->Unmap(lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(1, 1, &lightBuffer);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetShaderResources(1, 1, &heightMap);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
	deviceContext->VSSetShaderResources(0, 1, &heightMap);
	deviceContext->VSSetSamplers(0, 1, &heightmapSample);
}