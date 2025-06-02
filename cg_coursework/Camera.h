#pragma once
#include <d3d11.h>
#include "cgmath.h"
#include "window.h"
#include "shader.h"
#include "GEMModel.h"


class Camera
{
public:
	Matrix viewMatrix;
	Matrix projectionMatrix;
	Vec3 from; //key board control;
	Vec3 lookat; //mouse control
	Vec3 velocity;
	sphere collision_sphere;

	const float mouseSensitivity = 0.003;

	float pitch; //Rotation around X
	float yaw;   //Rotation around Y
	float radius;//Distance from the center;

	Camera(float fovY, float aspectRario, float nearZ, float farZ, float radius, Shaders& shader);
	void init(Shaders& shader) {
		from = Vec3(0, 1, -30); //The viewMatrix and ProjectionMatrix has been established
		lookat = Vec3(-1, 0, 0.25);
		collision_sphere.position = from;
		collision_sphere.radius = 5;
		velocity = Vec3(0, 0, 0);
		Update(shader);
	}
	//void Update(float deltaTime, float _x, float _y, float _z);
	void Update(Shaders& shader);
	void fromControl(Window& window, float dt);
	void lookatControl(Window& window);

	void collision_handle(vector<GEMModel*> models);

	void gameloop(Window& window, float dt, Shaders& shader, vector<GEMModel*> models);
	const Matrix& GetViewMatrix() const { return viewMatrix; }
	const Matrix& GetProjectionMatrix() const { return projectionMatrix; }
	const Vec3& Getposition() const { return from; }
};