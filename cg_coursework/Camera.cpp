#include "Camera.h"

Camera::Camera(float fovY, float aspectRatio, float nearZ, float farZ, float radius, Shaders& shader) : pitch(0.f), yaw(0.f), radius(radius) {
	projectionMatrix = Matrix(1 / (tan(fovY * M_PI / 360) * aspectRatio), 0, 0, 0, 0, 1 / tan(fovY * M_PI / 360), 0, 0, 0, 0, farZ / (farZ - nearZ), -1 * farZ * nearZ / (farZ - nearZ), 0, 0, 1, 0);
	init(shader);
}

void Camera::Update(Shaders& shader) {
	//Create the view matrix
	viewMatrix.ViewMatrix(from, lookat); //look at = to - from
	Matrix VP = projectionMatrix.mul(viewMatrix);
	shader.updateConstantVS("viewProjection", "VP", &VP);
	shader.uploadConstantVS("viewProjection", "VP");
}

void Camera::fromControl(Window& window, float dt) {
	float speed = 15 * dt;
	if (window.keyPressed('W')) {
		velocity = lookat * speed;
		//from += velocity;
	}
	if (window.keyPressed('S')) {
		velocity = lookat * -speed;
		//from += velocity;
	}
	if (window.keyPressed('A')) {
		Vec3 right(viewMatrix[0], viewMatrix[1], viewMatrix[2]);
		velocity = right * -speed;
		//from += velocity;
	}
	if (window.keyPressed('D')) {
		Vec3 right(viewMatrix[0], viewMatrix[1], viewMatrix[2]);
		velocity = right * speed;
		//from += velocity;
	}
	if (window.keyPressed(' ')) {
		velocity = Vec3(0, 1, 0) * speed;
		//from += velocity;
	}
	if (window.keyPressed(VK_SHIFT)) {
		velocity = Vec3(0, 1, 0) * -speed;
		//from += velocity;
	}
	if (window.keyPressed('R')) {
		from = Vec3(5, 5, 5);
	}
}

void Camera::lookatControl(Window& window) {
	if (window.dirty) {
		yaw = window.deltaX * mouseSensitivity;
		pitch = 1 * window.deltaY * mouseSensitivity;
		window.dirty = false;
	}
	else {
		yaw = 0;
		pitch = 0;
	}
	Matrix rx, ry;
	rx = rx.Rotation(Vec3(viewMatrix[0], viewMatrix[1], viewMatrix[2]), pitch); //It should rotate by the right vector
	ry = ry.Rotation(1, yaw);
	Matrix ro = rx.mul(ry);
	lookat = lookat.normalize();
	if (lookat.dot(Vec3(0, 1, 0)) >= 0.95) {
		lookat = ry.mulVec(lookat);
		lookat.y -= 0.001;
	}
	else if (lookat.dot(Vec3(0, 1, 0)) <= -0.95) {
		lookat = ry.mulVec(lookat);
		lookat.y += 0.001;
	}
	else {
		lookat = ro.mulVec(lookat);
	}

}

void Camera::collision_handle(vector<GEMModel*> models) {
	for (int i = 0; i < models.size(); i++) {
		Vec3 collision_vector = collision_sphere.intersects(models[i]->aabb, models[i]->World);
		if (collision_vector.x == 0 && collision_vector.y == 0 && collision_vector.z == 0) {
			from += velocity;
			collision_sphere.position = from;
		}
		else{
			from += collision_vector * 0.1;
			collision_sphere.position = from;
			cout << "collision vector: " << collision_vector.x << " " << collision_vector.y << " " << collision_vector.z << endl;
		}
		velocity = Vec3(0, 0, 0);
	}
}

void Camera::gameloop(Window& window, float dt, Shaders& shader, vector<GEMModel*> models) {
	lookatControl(window);
	fromControl(window, dt);
	collision_handle(models);
	Update(shader);
}