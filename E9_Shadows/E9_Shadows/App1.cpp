// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Create Mesh object and shader object
	mesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	model = new AModel(renderer->getDevice(), "res/teapot.obj");
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");
	textureMgr->loadTexture(L"height", L"res/height.png");

	// initial shaders
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	shadowShader = new ShadowShader(renderer->getDevice(), hwnd);
	manipulation_shader = new ManipulationShader(renderer->getDevice(), hwnd);

	// Variables for defining shadow map
	int shadowmapWidth = 1024 * 3;
	int shadowmapHeight = 1024 * 3;
	int sceneWidth = 100;
	int sceneHeight = 100;

	sphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());

	// This is your shadow map
	shadowMap[0] = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);
	shadowMap[1] = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);

	// Configure directional light
	lights[0] = new Light();
	lights[0]->setAmbientColour(0.0f, 0.0f, 0.0f, 1.0f);
	lights[0]->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	lights[0]->setDirection(0.0f, -0.7f, 0.7f);
	lights[0]->setPosition(0.f, 0.f, -10.f);
	lights[0]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
	//lights[0]->generateProjectionMatrix(250, 50);

	lights[1] = new Light();
	lights[1]->setAmbientColour(0.0f, 0.0f, 0.0f, 1.0f);
	lights[1]->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	lights[1]->setDirection(0.0f, -0.7f, 0.7f);
	lights[1]->setLookAt(0, 0, 0);
	lights[1]->setPosition(0.f, 0.f, -10.f);
	lights[1]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);

}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.

}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{
	elapsedTime += timer->getTime();

	// Perform depth pass
	depthPass();
	// Render scene
	finalPass();

	return true;
}

void App1::depthPass()
{
	// Set the render target to be the render to texture.
	shadowMap[0]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	// get the world, view, and projection matrices from the camera and d3d objects.
	lights[0]->generateViewMatrix();
	XMMATRIX lightViewMatrix = lights[0]->getViewMatrix();
	XMMATRIX lightProjectionMatrix = lights[0]->getOrthoMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);

	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	//manipulation_shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix, textureMgr->getTexture(L"rock"), textureMgr->getTexture(L"height"), 0, lights);
	//manipulation_shader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	//mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix, textureMgr->getTexture(L"height"), 2, elapsedTime);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(modelPos.x, modelPos.y, modelPos.z);;
	XMMATRIX scaleMatrix = XMMatrixScaling(0.25f, 0.25f, 0.25f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);

	// Render model
	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix, textureMgr->getTexture(L"nul"), 0, elapsedTime);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();

	// Set the render target to be the render to texture.
	shadowMap[1]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	// get the world, view, and projection matrices from the camera and d3d objects.
	lights[1]->generateViewMatrix();
	lightViewMatrix = lights[1]->getViewMatrix();
	lightProjectionMatrix = lights[1]->getOrthoMatrix();
	worldMatrix = renderer->getWorldMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix, textureMgr->getTexture(L"height"), 2, elapsedTime);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(modelPos.x, modelPos.y, modelPos.z);;
	scaleMatrix = XMMatrixScaling(0.25f, 0.25f, 0.25f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);

	// Render model
	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix, textureMgr->getTexture(L"nul"), 0, elapsedTime);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	camera->update();

	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"height"), shadowMap[0]->getDepthMapSRV(), shadowMap[1]->getDepthMapSRV(), lights, textureMgr->getTexture(L"height"), 2, elapsedTime);
	shadowShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// Render model
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(modelPos.x, modelPos.y, modelPos.z);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.25f, 0.25f, 0.25f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	model->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), shadowMap[0]->getDepthMapSRV(), shadowMap[1]->getDepthMapSRV(), lights, textureMgr->getTexture(L"nul"), 0, elapsedTime);
	shadowShader->render(renderer->getDeviceContext(), model->getIndexCount());

	worldMatrix = XMMatrixTranslation(lights[0]->getPosition().x, lights[0]->getPosition().y, lights[0]->getPosition().z);
	sphere->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), shadowMap[0]->getDepthMapSRV(), shadowMap[1]->getDepthMapSRV(), lights, textureMgr->getTexture(L"nul"), 0, elapsedTime);
	shadowShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();

	worldMatrix = XMMatrixTranslation(lights[1]->getPosition().x, lights[1]->getPosition().y, lights[1]->getPosition().z);
	sphere->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), shadowMap[0]->getDepthMapSRV(), shadowMap[1]->getDepthMapSRV(), lights, textureMgr->getTexture(L"nul"), 0, elapsedTime);
	shadowShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	gui();
	renderer->endScene();
}



void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);

	ImGui::Text("Light 1");
	ImGui::SliderFloat3("Position 1", &lightPos.x, -100.f, 100.0f);
	lights[0]->setPosition(lightPos.x, lightPos.y, lightPos.z);
	ImGui::SliderFloat3("Direction 1", &lightDirection.x, -100.f, 100.0f);
	lights[0]->setDirection(lightDirection.x, lightDirection.y, lightDirection.z);
	ImGui::ColorEdit3("Color 1", &lightDif.x);
	lights[0]->setDiffuseColour(lightDif.x, lightDif.y, lightDif.z, 1.0f);

	ImGui::Text("Light 2");
	ImGui::SliderFloat3("Position 2", &light2Pos.x, -100.f, 100.0f);
	lights[1]->setPosition(light2Pos.x, light2Pos.y, light2Pos.z);
	ImGui::SliderFloat3("Direction 2", &light2Direction.x, -100.f, 100.0f);
	lights[1]->setDirection(light2Direction.x, light2Direction.y, light2Direction.z);
	ImGui::ColorEdit3("Color 2", &light2Dif.x);
	lights[1]->setDiffuseColour(light2Dif.x, light2Dif.y, light2Dif.z, 1.0f);

	ImGui::Text("Model Position");
	ImGui::SliderFloat3("Model Pos", &modelPos.x, -500.f, 500.0f);

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

