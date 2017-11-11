//
// Created by adamyuan on 10/28/17.
//

#include "GL/glew.h"
#include "Application.hpp"
#include <algorithm>

#define GROUND_Y 0.0f
#define H_GROUND_SIZE 1000.0f
#define INIT_DENSITY 0.7f

//most of the framebuffer stuff is from https://learnopengl.com/

bool control = true, showPath = false, bloom = true, transform = false;
void focusCallback(GLFWwindow*, int focused)
{
	control = focused != 0;
}
int sWidth, sHeight;
bool resized = false;
void framebufferSizeCallback(GLFWwindow*, int width, int height)
{
	sWidth = width, sHeight = height;
	resized = true;
}
void keyCallback(GLFWwindow*, int key, int scancode, int action, int mods)
{
	if(action != GLFW_PRESS)
		return;
	if(key == GLFW_KEY_V)
		showPath = !showPath;
	if(key == GLFW_KEY_B)
		bloom = !bloom;
	if(key == GLFW_KEY_ENTER)
		transform = true;
	if(key == GLFW_KEY_ESCAPE)
		control = false;

}

Application::Application(int argc, char **(&argv))
{
	//process arguments
	std::string objNames[MODEL_NUM] = {"models/wheel.obj", "models/dragon.obj", "models/kleinbottle.obj"};
	float densitys[MODEL_NUM] = {1.5f, 1.5f, 1.5f};
	float scales[MODEL_NUM] = {0.06f, 1.6f, 0.025f};
	float deltaY[MODEL_NUM] = {45.0f, 25.0f, 20.0f};
	int pointNum = 1296;

	InitOpengl();
	InitResources();
	InitFramebuffers();

	auto cmp = [](const glm::vec3 &l, const glm::vec3 &r){return l.x - l.y + l.z < r.x - r.y + r.z;};
	//prepare data
	for(int i=0; i<MODEL_NUM; ++i)
	{
		ObjFiles[i].Load(objNames[i], densitys[i], scales[i], deltaY[i]);
		ObjFiles[i].PickPoints(pointNum, Points[i]);
		std::cout << Points[i].size() << " drones total for object " << objNames[i] << std::endl;
		if(Points[i].size() < pointNum)
			throw std::runtime_error("Too few points");
		sort(Points[i].begin(), Points[i].end(), cmp);
	}

	std::vector<glm::vec3> initialPositions;
	initialPositions.reserve(static_cast<unsigned long>(pointNum));
	auto sqrt_pointNum = static_cast<int>(std::sqrt(pointNum));
	float mx = -sqrt_pointNum * INIT_DENSITY / 2;
	for(int i=0; i<pointNum; ++i)
		initialPositions.emplace_back(
				mx + (i % sqrt_pointNum) * INIT_DENSITY + INIT_DENSITY / 2,
				GROUND_Y,
				mx + (i / sqrt_pointNum) * INIT_DENSITY + INIT_DENSITY / 2);

	HalfLaunchAreaSize = -mx;

	sort(initialPositions.begin(), initialPositions.end(), cmp);
	Group.Init(initialPositions);

	std::swap(Points[MODEL_NUM], initialPositions);

	Camera.Position = glm::vec3(-51.2138, GROUND_Y + 1.7, 80.3485);
}

void Application::Run()
{
	glViewport(0, 0, Width, Height);
	Matrices.UpdateMatrices(Width, Height);

	while(!glfwWindowShouldClose(Window))
	{
		if(transform && NextModel <= MODEL_NUM && Group.AllArrived()) {
			if(NextModel != 0)
				Group.SetDestinations(Points[NextModel++]);
			else {
				Group.ClearRecords();
				Group.SetDestinations(Points[NextModel++], 0.03f);
			}

			if(NextModel > MODEL_NUM)
				NextModel = 0;

			transform = false;
		}

		Group.NextTick(FPSManager);

		FPSManager.UpdateFrameRateInfo();

		Render();

		glfwSwapBuffers(Window);
		glfwPollEvents();
		if(resized)
		{
			Width = sWidth, Height = sHeight, resized = false;
			glViewport(0, 0, Width, Height);
			Matrices.UpdateMatrices(Width, Height);
			UpdateFramebuffers();
		}

		if(glfwGetMouseButton(Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			glfwSetCursorPos(Window, Width / 2, Height / 2);
			control = true;
		}

		if(control)
		{
			glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			Control();
		}
		else
			glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

Application::~Application()
{
	DeleteFramebuffers();
	glfwTerminate();
}

void Application::Render()
{
	glm::mat4 view = Camera.GetViewMatrix();
	// render to LightFBO
	glBindFramebuffer(GL_FRAMEBUFFER, LightFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	RenderLight();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, SkyboxFBO);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDepthMask(GL_FALSE);
	SkyboxShader.Use();
	SkyboxShader.PassMat4("view", glm::mat4(glm::mat3(view)));
	SkyboxShader.PassMat4("projection", Matrices.Projection3d);
	SkyboxShader.PassInt("skybox", 0);
	glActiveTexture(GL_TEXTURE0);
	SkyboxTexture.Bind();
	SkyboxObject.Render(GL_TRIANGLES);
	glDepthMask(GL_TRUE);

	GroundShader.Use();
	GroundShader.PassMat4("view", view);
	GroundShader.PassMat4("projection", Matrices.Projection3d);
	GroundShader.PassInt("image", 0);
	GroundShader.PassFloat("half_size", HalfLaunchAreaSize);
	glActiveTexture(GL_TEXTURE0);
	GroundTexture.Bind();
	GroundObject.Render(GL_TRIANGLES);

	RenderPath();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// blur bright fragments with two-pass Gaussian Blur
	bool horizontal = true, first_iteration = true;
	unsigned int amount = bloom ? 10 : 0;
	BlurShader.Use();
	for (unsigned int i = 0; i < amount; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, PingpongFBO[horizontal]);
		BlurShader.PassInt("horizontal", horizontal);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, first_iteration ? LightColorBuffer : PingpongColorbuffers[!horizontal]);
		ScreenObject.Render(GL_TRIANGLES);
		horizontal = !horizontal;
		if (first_iteration)
			first_iteration = false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	FinalShader.Use();
	FinalShader.PassInt("scene", 0);
	FinalShader.PassInt("bloomBlur", bloom ? 1 : 3);
	FinalShader.PassInt("skybox", 2);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, LightColorBuffer);
	if(bloom)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, PingpongColorbuffers[!horizontal]);
	}
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, SkyboxColorBuffer);
	ScreenObject.Render(GL_TRIANGLES);
}
void Application::RenderPath()
{
	if(!showPath)
		return;

	LightShader.Use();
	LightShader.PassVec4("color", glm::vec4(1.0, 1.0, 0.0, 1.0));
	for(int i=0; i<RECORD_NUM; ++i)
	{
		MyGL::VertexObject line;

		line.SetDataVec(Group.GetRecordPositions(i));
		line.SetAttributes(1, 0, 3);

		line.Render(GL_LINE_STRIP);
	}
}
void Application::RenderLight()
{
	MyGL::VertexObject lightObject;

	lightObject.SetDataVec(Group.GetDronePositions());
	lightObject.SetAttributes(1, 0, 3);

	glEnable(GL_PROGRAM_POINT_SIZE);
	LightShader.Use();

	LightShader.PassVec4("color", glm::vec4(1.0, 0.0, 0.0, 1.0));
	LightShader.PassMat4("projection", Matrices.Projection3d);
	LightShader.PassMat4("view", Camera.GetViewMatrix());

	lightObject.Render(GL_POINTS);

}

#define MOVE_DIST 0.4f
#define MOUSE_SENSITIVITY 0.17f
void Application::Control()
{
	float dist = FPSManager.GetMovementDistance(MOVE_DIST);
	if(glfwGetKey(Window, GLFW_KEY_W))
		Camera.MoveForward(dist, 0);
	if(glfwGetKey(Window, GLFW_KEY_S))
		Camera.MoveForward(dist, 180);
	if(glfwGetKey(Window, GLFW_KEY_A))
		Camera.MoveForward(dist, 90);
	if(glfwGetKey(Window, GLFW_KEY_D))
		Camera.MoveForward(dist, -90);

	double x, y;
	glfwGetCursorPos(Window, &x, &y);
	Camera.ProcessMouseMovement((float) (Width / 2 - x), (float) (Height / 2 - y),
								MOUSE_SENSITIVITY);
	glfwSetCursorPos(Window, Width / 2, Height / 2);
}

void Application::InitFramebuffers()
{
	glGenFramebuffers(1, &LightFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, LightFBO);
	// create a color attachment texture
	glGenTextures(1, &LightColorBuffer);
	glBindTexture(GL_TEXTURE_2D, LightColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, LightColorBuffer, 0);

	glGenFramebuffers(1, &SkyboxFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, SkyboxFBO);
	// create a color attachment texture
	glGenTextures(1, &SkyboxColorBuffer);
	glBindTexture(GL_TEXTURE_2D, SkyboxColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, SkyboxColorBuffer, 0);

	// ping-pong-framebuffer for blurring
	glGenFramebuffers(2, PingpongFBO);
	glGenTextures(2, PingpongColorbuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, PingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, PingpongColorbuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, Width, Height, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, PingpongColorbuffers[i], 0);
	}
}

void Application::InitOpengl()
{
	//init glfw and opengl

	glfwInit();

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	Window = glfwCreateWindow(Width, Height, "ProblemA", nullptr, nullptr);
	if(Window == nullptr)
		throw std::runtime_error("Error when creating Window");

	glfwMakeContextCurrent(Window);

	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK)
		throw std::runtime_error("Error when initialize GLEW");

	//hide the mouse

	glfwSetWindowFocusCallback(Window, focusCallback);
	glfwSetFramebufferSizeCallback(Window, framebufferSizeCallback);
	glfwSetKeyCallback(Window, keyCallback);
}

void Application::InitResources()
{
	//init vertexObject
	float quadVertices[24] = {
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			1.0f, -1.0f,  1.0f, 0.0f,

			-1.0f,  1.0f,  0.0f, 1.0f,
			1.0f, -1.0f,  1.0f, 0.0f,
			1.0f,  1.0f,  1.0f, 1.0f
	};
	ScreenObject.SetDataArr(quadVertices, 24);
	ScreenObject.SetAttributes(2, 0, 2, 1, 2);

	float groundVertices[] = {
			-H_GROUND_SIZE, GROUND_Y,  H_GROUND_SIZE,
			-H_GROUND_SIZE, GROUND_Y, -H_GROUND_SIZE,
			H_GROUND_SIZE, GROUND_Y, -H_GROUND_SIZE,

			-H_GROUND_SIZE, GROUND_Y,  H_GROUND_SIZE,
			H_GROUND_SIZE, GROUND_Y, -H_GROUND_SIZE,
			H_GROUND_SIZE, GROUND_Y,  H_GROUND_SIZE,
	};

	GroundObject.SetDataArr(groundVertices, 18);
	GroundObject.SetAttributes(1, 0, 3);

	GroundTexture.Load2d("resources/ground.png");
	GroundTexture.SetParameters(GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST, GL_REPEAT);
	GroundTexture.BuildMipmap();

	//init shader
	LightShader.LoadFromFile("resources/light.vert", GL_VERTEX_SHADER);
	LightShader.LoadFromFile("resources/light.frag", GL_FRAGMENT_SHADER);
	BlurShader.LoadFromFile("resources/blur.vert", GL_VERTEX_SHADER);
	BlurShader.LoadFromFile("resources/blur.frag", GL_FRAGMENT_SHADER);
	FinalShader.LoadFromFile("resources/final.vert", GL_VERTEX_SHADER);
	FinalShader.LoadFromFile("resources/final.frag", GL_FRAGMENT_SHADER);
	SkyboxShader.LoadFromFile("resources/skybox.vert", GL_VERTEX_SHADER);
	SkyboxShader.LoadFromFile("resources/skybox.frag", GL_FRAGMENT_SHADER);
	GroundShader.LoadFromFile("resources/ground.vert", GL_VERTEX_SHADER);
	GroundShader.LoadFromFile("resources/ground.frag", GL_FRAGMENT_SHADER);

	//init skybox
	float skyboxVertices[] = {
			// positions
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			1.0f,  1.0f, -1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			1.0f, -1.0f,  1.0f
	};
	SkyboxObject.SetDataArr(skyboxVertices, 108);
	SkyboxObject.SetAttributes(1, 0, 3);


	SkyboxTexture.LoadCubemap({
									  "resources/skybox/right.png",
									  "resources/skybox/left.png",
									  "resources/skybox/up.png",
									  "resources/skybox/down.png",
									  "resources/skybox/back.png",
									  "resources/skybox/front.png"
							  });
	SkyboxTexture.SetParameters(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
}

void Application::UpdateFramebuffers()
{
	glBindTexture(GL_TEXTURE_2D, LightColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	glBindTexture(GL_TEXTURE_2D, SkyboxColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	for (unsigned int PingpongColorbuffer : PingpongColorbuffers) {
		glBindTexture(GL_TEXTURE_2D, PingpongColorbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, Width, Height, 0, GL_RGB, GL_FLOAT, nullptr);
	}
}

void Application::DeleteFramebuffers()
{
	glDeleteFramebuffers(1, &LightFBO);
	glDeleteFramebuffers(1, &SkyboxFBO);
	glDeleteFramebuffers(2, PingpongFBO);

	glDeleteTextures(1, &LightColorBuffer);
	glDeleteTextures(1, &SkyboxColorBuffer);
	glDeleteTextures(2, PingpongColorbuffers);
}


