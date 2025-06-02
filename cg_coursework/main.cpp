#include "window.h";
#include "adapter.h"
#include "shader.h"
#include "vertex.h"
#include "Timer.h"
#include "gamecontrol.h"
#include "GEMModel.h"
#include <iostream>
#include "texture.h"
#include "sky.h"

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
	AllocConsole();
	FILE* pConsole = nullptr;
	freopen_s(&pConsole, "CONOUT$", "w", stdout);

	Matrix world;
	bool running = true;
	Window win;
	DXcore dx;
	win.create("zzx", 1024, 1024);
	dx.init(1024, 1024, win.hwnd, false);
	Shaders shader(&dx);
	shader.init("PS.txt", "VS.txt");
	Shaders tex(&dx);
	tex.init("TEX.txt", "VS.txt");
	Camera camera(45, 1, 0.1f, 700.f, 20, shader);

	TextureManager tm;
	vector<GEMModel*> models;

	//Plane plane;
	//plane.init(dx);
	Cube cube;
	cube.init(dx, 1, Vec3(0, 0, 0));
	//Sphere sphere;
	//sphere.init(dx);
	Sky sky;
	sky.init(&dx, tm, 450);
	Ground ground;
	ground.init(&dx, tm, 10);

	//GEMModel model;
	//model.init("Resources/TRex/TRex.gem", dx, Vec3(0, 0, 0));
	GEMModel tree;
	tree.init("Resources/Pine/pine.gem", dx, Vec3(7, 0, 10));
	models.push_back(&tree);
	Trex trex;
	trex.init("Resources/TRex/TRex.gem", dx, Vec3(2, 0, 1));
	models.push_back(&trex);

	//tm.load(&dx, model.textureFilenames);
	tm.load(&dx, trex.textureFilenames);
	tm.load(&dx, tree.textureFilenames);
	AnimationInstance instanceTrex;
	//instanceTrex.init(&(model.animation));
	instanceTrex.init(&(trex.animation));
	AnimationInstance instanceTree;
	instanceTree.init(&(tree.animation));

	Sampler linear;
	linear.init(&dx);

	//vector<ModelData> modeldatas = LoadModelsFromFile("Resources/modelloader.txt");

	gm gamemanager;
	TIMER timer;

	while (running) {
		gamemanager.dt = timer.getDeltaTime();
		dx.clear();
		win.processMessages();
		
		if (win.keyPressed('Q')) {
			running = false;
		}
		shader.apply(); //switch the shader
		//plane.Draw(dx, shader, world);
		cube.Draw(dx, shader, world);
		//sphere.Draw(dx, shader, world);
		tex.apply();
		//model.Draw(dx, tex, instanceTrex, gamemanager.dt, world, tm, "Run");
		trex.Draw(dx, tex, instanceTrex, gamemanager.dt, world, tm, camera.Getposition());
		tree.Draw(dx, tex, instanceTree, gamemanager.dt, world, tm, "", 0.01);
		sky.Draw(camera, dx, tex, world, tm);
		ground.Draw(dx, shader, world, tm);

		camera.gameloop(win, gamemanager.dt, shader, models);
		dx.present();
	}
}