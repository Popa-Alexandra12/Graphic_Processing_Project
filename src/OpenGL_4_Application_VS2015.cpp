//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC

#include <iostream>
#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLEW/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include "Shader.hpp"
#include "SkyBox.hpp"
#include "Camera.hpp"
#define TINYOBJLOADER_IMPLEMENTATION

#include "Model3D.hpp"
#include "Mesh.hpp"

#define SHADOW_WIDTH 2048
#define SHADOW_HEIGHT 2048

int glWindowWidth = 1920;
int glWindowHeight = 1080;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;



glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

int fog;
GLuint fogLoc;

glm::mat3 lightDirMatrix;
GLuint lightDirMatrixLoc;

GLfloat lightAngle;

gps::Camera myCamera(glm::vec3(0.0f, 0.0f, 2.5f), glm::vec3(0.0f, 0.0f, -10.0f));
float cameraSpeed = 0.05f;

glm::vec2 position, prevPosition;
bool pressedKeys[1024];
float angle = 0.0f;
float angleY = 0.0f;

GLuint shadowMapFBO;

gps::Model3D tree;
gps::Model3D ground;
gps::Model3D bench;
gps::Model3D myModel4;
gps::Model3D bin;
gps::Model3D lamp;
gps::Model3D cloud;
gps::Model3D foisor;
gps::Model3D hedge;
gps::Model3D neighborhood;
gps::Model3D house;
gps::Model3D fountain;
gps::Model3D leagan;
gps::Model3D leagan1;
gps::Model3D birds;
gps::Shader myCustomShader;

gps::Shader lightShader;
gps::Shader depthMapShader;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;
gps::Shader reflectShader;

GLuint depthMapTexture;



GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

bool firstMouse = true;
void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	position.x = xpos;
	position.y = ypos;

	if (firstMouse)
	{
		prevPosition.x = position.x;
		prevPosition.y = position.y;
		firstMouse = false;
	}
	float xoffset = position.x - prevPosition.x;
	float yoffset = prevPosition.y - position.y;
	prevPosition.x = position.x;
	prevPosition.y = position.y;

	myCamera.rotate(yoffset, xoffset);
}
void processMovement()
{

	if (pressedKeys[GLFW_KEY_Q]) {
		angle += 0.1f;
		if (angle > 360.0f)
			angle -= 360.0f;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angle -= 0.1f;
		if (angle < 0.0f)
			angle += 360.0f;
	}

	if (pressedKeys[GLFW_KEY_Z]) {
		angleY += 0.1f;
		if (angleY > 360.0f)
			angleY -= 360.0f;
	}

	if (pressedKeys[GLFW_KEY_X]) {
		angleY -= 0.1f;
		if (angleY < 0.0f)
			angleY += 360.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_F]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	}

	if (pressedKeys[GLFW_KEY_G]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	if (pressedKeys[GLFW_KEY_H]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}

	if (pressedKeys[GLFW_KEY_J]) {

		lightAngle += 0.3f;
		if (lightAngle > 360.0f)
			lightAngle -= 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		myCustomShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	}
	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle -= 0.3f;
		if (lightAngle < 0.0f)
			lightAngle += 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		myCustomShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	}
	if (pressedKeys[GLFW_KEY_C]) {
		fog =1;
		myCustomShader.useShaderProgram();
		glUniform1i(fogLoc, fog);

	}
	if (pressedKeys[GLFW_KEY_V]) {
		fog = 0;
		myCustomShader.useShaderProgram();
		glUniform1i(fogLoc, fog);
	}

	if (pressedKeys[GLFW_KEY_UP]) {
		lightColor = glm::vec3(1,1,1);
		myCustomShader.useShaderProgram();
		glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	}
	if (pressedKeys[GLFW_KEY_DOWN]) {
		lightColor = glm::vec3(0.2,0,1);
		myCustomShader.useShaderProgram();
		glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
	}

	if (pressedKeys[GLFW_KEY_LEFT_SHIFT]) {
		
	}
}
glm::mat4 computeLightSpaceTrMatrix()
{
	const GLfloat near_plane = 1.0f, far_plane = 10.0f;
	glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);

	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	glm::mat4 lightView = glm::lookAt(myCamera.getCameraTarget() + 1.0f * lightDirTr, myCamera.getCameraTarget(), glm::vec3(0.0f, 1.0f, 0.0f));

	return lightProjection * lightView;
}
bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	//for Mac OS X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example",NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwMakeContextCurrent(glWindow);

	glfwWindowHint(GLFW_SAMPLES, 4);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);

    glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels()
{
	tree = gps::Model3D("objects/tree/Tree.obj", "objects/tree/");
	ground = gps::Model3D("objects/ground/ground.obj", "objects/ground/");
	bench = gps::Model3D("objects/bench/ConcreteBench-L3.obj", "objects/bench/");
	myModel4 = gps::Model3D("objects/pavement/ground.obj", "objects/pavement/");
	bin = gps::Model3D("objects/bin/untitled.obj", "objects/bin/");
	lamp = gps::Model3D("objects/lamp4/lamp1.obj", "objects/lamp4/");
	cloud = gps::Model3D("objects/new_cloud/cloud1.obj", "objects/new_cloud/");
	foisor = gps::Model3D("objects/foisor/Gazebo.obj", "objects/foisor/");
	hedge = gps::Model3D("objects/hedge/10449_Rectangular_Box_Hedge_v1_iterations-2.obj", "objects/hedge/");
	neighborhood = gps::Model3D("objects/neighborhood3/untitled1.obj", "objects/neighborhood3/");
	leagan = gps::Model3D("objects/leagan/leagan_gata.obj", "objects/leagan/");
	birds = gps::Model3D("objects/birds/leagan1.obj", "objects/birds/");

}

void initFramebuffer()
{
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initShaders()
{
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	depthMapShader.loadShader("shaders/simpleDepthMap.vert", "shaders/simpleDepthMap.frag");
}
void initSkyBox()
{
	std::vector<const GLchar*> faces;
	faces.push_back("skybox/hills/right.tga");
	faces.push_back("skybox/hills/left.tga");
	faces.push_back("skybox/hills/top.tga");
	faces.push_back("skybox/hills/bottom.tga");
	faces.push_back("skybox/hills/back.tga");
	faces.push_back("skybox/hills/front.tga");
	mySkyBox.Load(faces);
}
void initUniforms()
{
	myCustomShader.useShaderProgram();

	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");

	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");

	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");

	lightDirMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDirMatrix");

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 2.0f);
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	// set FOG
	fog = 1; //white light
	fogLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fog");
	glUniform1i(fogLoc, fog);


}

float delta1 = 0.0f;
float movementSpeed = 2; // units per second
int up = 1;
double lastTimeStamp = glfwGetTime();


void updateDelta1(double elapsedSeconds) {
	delta1 = delta1 + movementSpeed * elapsedSeconds;

}

void light_rds()
{
	depthMapShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);


	///---------------------------------------------------------------------------------------------------

	glm::mat3 model1 = model;

	model = glm::translate(model, glm::vec3(1, -1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(depthMapShader);//mymodel



	model = glm::translate(model, glm::vec3(2, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(depthMapShader);

	model = glm::translate(model, glm::vec3(2, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(depthMapShader);

	glm::mat3 model2 = model;

	ground.Draw(depthMapShader);

	model = glm::translate(model, glm::vec3(2, -2, 0));
	model = glm::scale(model, glm::vec3(0.03, 0.03, 0.03));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	lamp.Draw(depthMapShader);


	model = glm::translate(model, glm::vec3(200, 0, 0));
	model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	lamp.Draw(depthMapShader);

	model = model2;

	model = glm::translate(model, glm::vec3(9.5, 0, 0));
	model = glm::scale(model, glm::vec3(0.01, 0.01, 0.01));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	model = glm::translate(model, glm::vec3(-100, 0, -100));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	bench.Draw(depthMapShader);

	model = glm::translate(model, glm::vec3(0, -600, 0));
	model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0, 0, 1));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	bench.Draw(depthMapShader);

	model = model2;

	model = glm::translate(model, glm::vec3(10, 0, 0));
	model = glm::translate(model, glm::vec3(0, -1, 0));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	bin.Draw(depthMapShader); 

	
	glm::mat3 model4 = model;
	model = glm::translate(model, glm::vec3(1, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(depthMapShader);

	model = glm::translate(model, glm::vec3(2, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(depthMapShader);

	model = glm::translate(model, glm::vec3(2, 0, 5));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(depthMapShader);

	model = model1;
	
	model = glm::translate(model, glm::vec3(-1, 0, 0));
	model = glm::scale(model, glm::vec3(0.01, 0.01, 0.01));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	model = glm::translate(model, glm::vec3(-100, 0, -100));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	bench.Draw(depthMapShader);

	model = glm::translate(model, glm::vec3(0, -500, 0));
	model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0, 0, 1));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	bench.Draw(depthMapShader);

	model = model1;

	model = glm::translate(model, glm::vec3(-0.5, 0, 0));
	model = glm::translate(model, glm::vec3(0, -1, 0));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	bin.Draw(depthMapShader); 

	model = glm::translate(model, glm::vec3(-5, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	bin.Draw(depthMapShader); 

	model = glm::translate(model, glm::vec3(0, 0, -30));
	model = glm::translate(model, glm::vec3(-1, 0, 15));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(depthMapShader);

	model = glm::translate(model, glm::vec3(0, 0, 20));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(depthMapShader);

	model = glm::translate(model, glm::vec3(0, 0, 10));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(depthMapShader);

	model = model1;

	model = glm::translate(model, glm::vec3(-3.2, -1, 0));
	model = glm::scale(model, glm::vec3(0.02, 0.02, 0.02));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	lamp.Draw(depthMapShader);

	model = glm::translate(model, glm::vec3(250, 0, 0));
	model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	lamp.Draw(depthMapShader);

	glm::mat3 model3 = model;

	//--------------------------------------------------------------------------
	model = model2;

	model = glm::translate(model, glm::vec3(-15, -1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	ground.Draw(depthMapShader);

	model = model2;

	model = glm::translate(model, glm::vec3(-5, -1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(depthMapShader);

	model = glm::translate(model, glm::vec3(-2, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(depthMapShader); 

	model = glm::translate(model, glm::vec3(-2, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(depthMapShader);

	model = model2;

	model = glm::translate(model, glm::vec3(-5, -1, 5));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(depthMapShader);  

	model = glm::translate(model, glm::vec3(-2, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(depthMapShader);

	model = glm::translate(model, glm::vec3(-2, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(depthMapShader);

	glm::mat3 model7 = model;

	model = glm::translate(model, glm::vec3(-2, 0, 0));
	model = glm::scale(model, glm::vec3(0.005, 0.005, 0.005));
	model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	foisor.Draw(depthMapShader); 

	glm::mat3 model6 = model;

	model = model1;

	model = glm::translate(model, glm::vec3(0, -1, -5));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(depthMapShader);

	model = glm::translate(model, glm::vec3(2, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(depthMapShader);

	model = glm::translate(model, glm::vec3(0, 5, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(depthMapShader);

	glm::mat3 model5 = model;

	model = model1;

	model = glm::translate(model, glm::vec3(14.8, -1, 0));
	model = glm::scale(model, glm::vec3(0.005, 0.005, 0.005));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(depthMapShader);

	model = glm::translate(model, glm::vec3(200, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(depthMapShader);

	model = glm::translate(model, glm::vec3(400, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(depthMapShader);

	model = glm::translate(model, glm::vec3(200, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(depthMapShader);

	model = glm::translate(model, glm::vec3(400, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(depthMapShader);

	model = glm::translate(model, glm::vec3(200, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(depthMapShader);

	model = glm::translate(model, glm::vec3(400, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(depthMapShader);

	model = glm::translate(model, glm::vec3(150, 100, 0));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(depthMapShader);

	for (int i = 0; i < 38; i++)
	{
		model = glm::translate(model, glm::vec3(200, 0, 0));
		//send model matrix data to shader	
		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		hedge.Draw(depthMapShader);

	}
	model = glm::translate(model, glm::vec3(150, 100, 0));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(depthMapShader);

	for (int i = 0; i < 19; i++)
	{
		model = glm::translate(model, glm::vec3(200, 0, 0));
		//send model matrix data to shader	
		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		hedge.Draw(depthMapShader);

	}

	model = glm::translate(model, glm::vec3(0, 100, 0));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(depthMapShader);

	for (int i = 0; i < 38; i++)
	{
		model = glm::translate(model, glm::vec3(200, 0, 0));
		//send model matrix data to shader	
		glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		hedge.Draw(depthMapShader);

	}

	model = glm::translate(model, glm::vec3(150, 100, 0));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(depthMapShader);

	model = glm::translate(model, glm::vec3(200, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(depthMapShader);

	model = glm::translate(model, glm::vec3(800, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(depthMapShader);

	model = glm::translate(model, glm::vec3(200, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(depthMapShader);



	model = model1;

	model = glm::translate(model, glm::vec3(22.9, -1.15, 0));
	model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
	model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	neighborhood.Draw(depthMapShader);

	glm::mat3 model8 = model;

	model = glm::translate(model, glm::vec3(0, 0, -10));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	neighborhood.Draw(depthMapShader);

									
	model = glm::translate(model, glm::vec3(0, 0, 20));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	neighborhood.Draw(depthMapShader);

	double currentTimeStamp = glfwGetTime();

	model = model1;

	model = glm::scale(model, glm::vec3(0.01, 0.01, 0.01));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
	model = glm::translate(model, glm::vec3(-500, 500, -100));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	leagan.Draw(depthMapShader);

	model = model1;
	updateDelta1(currentTimeStamp - lastTimeStamp);
	lastTimeStamp = currentTimeStamp;

	model = glm::translate(model, glm::vec3(1, 10, 0));
	model = glm::translate(model, glm::vec3(0, 0, delta1));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	birds.Draw(depthMapShader);

///---------------------------------------------------------------------------------------------------




	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}




void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	processMovement();

	light_rds();

	myCustomShader.useShaderProgram();
	//send lightSpace matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	//send view matrix to shader
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"),
		1,
		GL_FALSE,
		glm::value_ptr(view));

	//compute light direction transformation matrix
	lightDirMatrix = glm::mat3(glm::inverseTranspose(view));
	//send lightDir matrix data to shader
	glUniformMatrix3fv(lightDirMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightDirMatrix));

	glViewport(0, 0, retina_width, retina_height);
	myCustomShader.useShaderProgram();

	//bind the depth map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);


	//initialize the model matrix
	model = glm::mat4(1.0f);
	//create model matrix
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(angleY), glm::vec3(0, 0, 1));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	glm::mat3 model1 = model;
				   
	model = glm::translate(model, glm::vec3(1, -1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(2, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(2, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);
	
	glm::mat3 model2 = model;

	ground.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(2,0, 0));
	model = glm::scale(model, glm::vec3(0.02, 0.02, 0.02));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	lamp.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(300,0, 0));
	model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0, 1,0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	lamp.Draw(myCustomShader);

	model = model2;

	model = glm::translate(model, glm::vec3(9.5, 0, 0));
	model = glm::scale(model, glm::vec3(0.01, 0.01, 0.01));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	model = glm::translate(model, glm::vec3(-100, 0, -100));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	bench.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(0, -600, 0));
	model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0, 0,1 ));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	bench.Draw(myCustomShader);

	model = model2;

	model = glm::translate(model, glm::vec3(10, 0, 0));
	model = glm::translate(model, glm::vec3(0, -1, 0));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	bin.Draw(myCustomShader); 

	glm::mat3 model4 = model;

	model = glm::translate(model, glm::vec3(1, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(2, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(2, 0, 5));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);

	model = model1;
	

	model = glm::translate(model, glm::vec3(-1, 0, 0));
	model = glm::scale(model, glm::vec3(0.01,0.01,0.01));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	model = glm::translate(model, glm::vec3(-100, 0, -100));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	bench.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(0, -500,0));
	model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0, 0, 1));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	bench.Draw(myCustomShader);

	model = model1;

	model = glm::translate(model, glm::vec3(-0.5,0 , 0));
	model = glm::translate(model, glm::vec3(0, -1, 0));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1,0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	bin.Draw(myCustomShader); 
	
	model = glm::translate(model, glm::vec3(-5, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	bin.Draw(myCustomShader); 

	model = glm::translate(model, glm::vec3(0, 0, -30));//-10
	model = glm::translate(model, glm::vec3(-1, 0, 15));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(0, 0, 20));//-2
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(0, 0, 10));//-2
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);

	model = model1;
	model = glm::translate(model, glm::vec3(-3.2,-1, 0));
	model = glm::scale(model, glm::vec3(0.02, 0.02, 0.02));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	lamp.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(250, 0, 0));
	model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	lamp.Draw(myCustomShader);


	glm::mat3 model3 = model;

	//-------------------------------------------------------------
	model = model2;

	model = glm::translate(model, glm::vec3(-15, -1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	ground.Draw(myCustomShader);

	model = model2;

	model = glm::translate(model, glm::vec3(-5, -1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(-2, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(-2, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);

	//opposite

	model = model2;

	model = glm::translate(model, glm::vec3(-5, -1, 5));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(-2, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(-2, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);

	glm::mat3 model7 = model;

	model = glm::translate(model, glm::vec3(-2, 0, 0));
	model = glm::scale(model, glm::vec3(0.005, 0.005, 0.005));
	model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	foisor.Draw(myCustomShader);  

	glm::mat3 model6 = model;

	model = model1;

	model = glm::translate(model, glm::vec3(0, -1, -5));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(2, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(0, 5, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	tree.Draw(myCustomShader);
	
	glm::mat3 model5 = model;
	
	model = model1;

	model = glm::translate(model, glm::vec3(14.8, -1, 0));
	model = glm::scale(model, glm::vec3(0.005, 0.005, 0.005));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(200, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(400, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(200, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(400, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(200, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(400, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(150, 100, 0));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(myCustomShader);

	for (int i = 0; i < 38; i++)
	{
		model = glm::translate(model, glm::vec3(200, 0, 0));
		//send model matrix data to shader	
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		hedge.Draw(myCustomShader);

	}

	model = glm::translate(model, glm::vec3(150, 100, 0));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(myCustomShader);

	for (int i = 0; i < 19; i++)
	{
		model = glm::translate(model, glm::vec3(200, 0, 0));
		//send model matrix data to shader	
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		hedge.Draw(myCustomShader);

	}

	model = glm::translate(model, glm::vec3(0, 100, 0));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(myCustomShader);

	for (int i = 0; i < 38; i++)
	{
		model = glm::translate(model, glm::vec3(200, 0, 0));
		//send model matrix data to shader	
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		hedge.Draw(myCustomShader);

	}

	model = glm::translate(model, glm::vec3(150, 100, 0));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(200, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(800, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(200, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	hedge.Draw(myCustomShader);
	
	model = model1;

	model = glm::translate(model, glm::vec3(22.9,-1.15, 0));
	model = glm::scale(model, glm::vec3(0.5,0.5,0.5));
	model = glm::rotate(model, glm::radians(-180.0f), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	neighborhood.Draw(myCustomShader);

	glm::mat3 model8 = model;

	model = glm::translate(model, glm::vec3(0,0, -10));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	neighborhood.Draw(myCustomShader);


	model = glm::translate(model, glm::vec3(0, 0, 20));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	neighborhood.Draw(myCustomShader);

	model = model1;

	model = glm::scale(model, glm::vec3(0.01, 0.01, 0.01));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	model = glm::translate(model, glm::vec3(-500, 500, -100));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	leagan.Draw(myCustomShader);

	double currentTimeStamp = glfwGetTime();
	model = model1;
	updateDelta1(currentTimeStamp - lastTimeStamp);
	lastTimeStamp = currentTimeStamp;
	

	model = glm::translate(model, glm::vec3(1, 10, 0));
	model = glm::translate(model, glm::vec3(0, 0, delta1));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	birds.Draw(myCustomShader);

	skyboxShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
		glm::value_ptr(view));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
		glm::value_ptr(projection));

	//initialize the model matrix
	model = glm::mat4(1.0f);
	//create model matrix
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(angleY), glm::vec3(0, 0, 1));
	//send model matrix data to shader	
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	mySkyBox.Draw(skyboxShader, view, projection);
}

int main(int argc, const char * argv[]) {

	initOpenGLWindow();
	initOpenGLState();
	initModels();
	initShaders();
	initSkyBox();
	myCustomShader.useShaderProgram();
	initUniforms();

	initFramebuffer();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	while (!glfwWindowShouldClose(glWindow)) {
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	//close GL context and any other GLFW resources
	glfwTerminate();

	return 0;
}
