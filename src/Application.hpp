//
// Created by adamyuan on 10/28/17.
//

#ifndef PROBLEMA_APPLICATION_HPP
#define PROBLEMA_APPLICATION_HPP

#include <MyGL/Matrices.hpp>
#include <MyGL/VertexObject.hpp>
#include <MyGL/Shader.hpp>
#include <MyGL/Camera.hpp>
#include <MyGL/Texture.hpp>
#include <MyGL/FrameRate.hpp>

#include <GLFW/glfw3.h>
#include <iostream>

#include "ObjFileLoader.hpp"
#include "DroneGroup.hpp"

#define MODEL_NUM 3
class Application
{
private:
	int Width = 1200, Height = 700;
	GLFWwindow *Window = nullptr;

	MyGL::Matrices Matrices;
	MyGL::VertexObject ScreenObject, SkyboxObject, GroundObject;
	MyGL::Shader LightShader, BlurShader, FinalShader, SkyboxShader, GroundShader;
	MyGL::Camera Camera;
	MyGL::Texture SkyboxTexture, GroundTexture;
	MyGL::FrameRateManager FPSManager;

	std::vector<glm::vec3> Points[MODEL_NUM + 1];

	unsigned int HdrFBO, ColorBuffers[2], RBO, PingpongFBO[2], PingpongColorbuffers[2],
			EnvironmentFBO, EnvironmentColorBuffer;

	DroneGroup Group;
	int NextModel = 0;

	float HalfLaunchAreaSize;

	void InitFramebuffers();
	void InitOpengl();
	void InitResources();

	void UpdateFramebuffers();
	void DeleteFramebuffers();

	void Render();
	void RenderLight();
	void RenderPath();
	void RenderEnvironment();
	void Control();

public:
	Application();
	~Application();
	void Run();

};


#endif //PROBLEMA_APPLICATION_HPP
