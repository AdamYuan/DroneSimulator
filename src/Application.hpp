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
	GLFWwindow *Window = nullptr;

	MyGL::Matrices Matrices;
	MyGL::VertexObject ScreenObject, SkyboxObject, GroundObject;
	MyGL::Shader LightShader, BlurShader, FinalShader, SkyboxShader, GroundShader;
	MyGL::Camera Camera;
	MyGL::Texture SkyboxTexture, GroundTexture;
	MyGL::FrameRateManager FPSManager;

	ObjFileLoader ObjFiles[3];

	std::vector<glm::vec3> Points[MODEL_NUM + 1];

	unsigned int LightFBO, LightColorBuffer, RBO, PingpongFBO[2], PingpongColorbuffers[2],
			SkyboxFBO, SkyboxColorBuffer;

	DroneGroup Group;
	bool Transform = false;
	int NextModel = 0;

	float HalfLaunchAreaSize;

	void InitFramebuffers();
	void InitOpengl();
	void InitResources();

	void Render();
	void RenderLight();
	void Control();

public:
	Application(int argc, char **(&argv));
	~Application();
	void Run();
};


#endif //PROBLEMA_APPLICATION_HPP
