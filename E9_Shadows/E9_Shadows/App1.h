// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "TextureShader.h"
#include "ShadowShader.h"
#include "DepthShader.h"
#include "ManipulationShader.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void depthPass();
	void finalPass();
	void gui();

private:
	TextureShader* textureShader;
	PlaneMesh* mesh;
	SphereMesh* sphere;
	ManipulationShader* manipulation_shader;
	

	//Light* light;
	AModel* model;
	ShadowShader* shadowShader;
	DepthShader* depthShader;

	ShadowMap* shadowMap[2];

	Light* lights[2];

	XMFLOAT4 ambient;
	XMFLOAT3 lightPos = XMFLOAT3(1, 1, 1);
	XMFLOAT3 light2Pos = XMFLOAT3(1, 1, 1);;
	XMFLOAT3 lightDirection = XMFLOAT3(1, 1, 1);
	XMFLOAT3 light2Direction = XMFLOAT3(1, 1, 1);;
	XMFLOAT3 lightDif = XMFLOAT3(1, 1, 1);
	XMFLOAT3 light2Dif = XMFLOAT3(1, 1, 1);;

	XMFLOAT3 modelPos = XMFLOAT3(0, 20, 10);
	float elapsedTime;
};

#endif