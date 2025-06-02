#pragma once
#include "vertex.h"
#include "Camera.h"
#include "texture.h"

class Sky : public Sphere
{
public:
	string filepath = "Textures/sky.png";
	void init(DXcore* dx, TextureManager& tm, float fac) {
		tm.load(dx, filepath);
		Sphere::init(*dx, fac, Vec3(0, 0, 0));
	}
	void update(Camera& camera, Shaders& shader, Matrix& w) {
		position = camera.Getposition();
		Shape::updateWorld(shader, w);
	}

	void Draw(Camera& camera, DXcore& dx, Shaders& shader, Matrix& w, TextureManager& tm) {
		update(camera, shader, w);
		shader.updateShaderResources(dx, "tex", tm.find(filepath));
		mesh.Draw(dx);
	}
};

class Ground
{
public:
	string filepath = "Textures/ground.png";
	vector<Plane> ground;
	void init(DXcore* dx, TextureManager& tm, int width) {
		tm.load(dx, filepath);
		for (int i = 0; i < width * width; i++) {
			Vec3 position = Vec3(i % width * 15, 0, i / width * 15);
			Plane templane;
			templane.init(*dx, position);
			ground.push_back(templane);
		}
	}
	void Draw(DXcore& dx, Shaders& shader, Matrix& w, TextureManager& tm) {
		shader.updateShaderResources(dx, "tex", tm.find(filepath));
		for (int i = 0; i < ground.size(); i++) {
			ground[i].Draw(dx, shader, w);
		}
	}
};