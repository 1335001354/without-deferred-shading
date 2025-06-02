#pragma once
#include <fstream>
#include <sstream>
#include <vector>
#include "GEMModel.h"

struct ModelData {
	string type;
	string modelPath;
	Vec3 pos;
	float scale;
	float rotX, rotY, rotZ;
};

std::vector<ModelData> LoadModelsFromFile(const string filePath) {
	std::vector<ModelData> models;
	std::ifstream file(filePath);
	if (!file.is_open()) {
		std::cerr << "Error:Unable to open file " << filePath << std::endl;
		return models;
	}
	string line;
	string type;
	string modelPath;
	int num = 0;
	while (std::getline(file, line)) {
		if (line.empty() || line[0] == '#') continue;
		std::istringstream stream(line);
		if (line[0] == '/') {
			stream >> type >> modelPath >> num;
		}
		for (int i = 0; i < num; i++) {
			std::getline(file, line);
			std::istringstream stream(line);
			ModelData md;
			md.type = type;
			md.modelPath = modelPath;
			stream >> md.pos.x >> md.pos.y >> md.pos.z >> md.scale >> md.rotX >> md.rotY >> md.rotZ;
			models.push_back(md);
		}
	}
	file.close();
	return models;
}

class modelmanager //All the models in the same modelmanager share the same shader
{
private:
	std::vector<GEMModel*> models;
	std::vector<Shape*> shapes;
	std::vector<AnimationInstance*> instances;
	std::vector<string> types;
	Shaders* shader;

public:
	modelmanager() :shader(nullptr) {}
	~modelmanager() {
		for (auto& model : models) {
			delete model;
		}
		models.clear();
		for (auto& instance : instances) {
			delete instance;
		}
		instances.clear();
		for (auto& shape : shapes) {
			delete shape;
		}
		shapes.clear();
		delete shader;
		shader = nullptr;
	}

	void initmodel(string filename, DXcore& dx, string ps, string vs) {
		vector<ModelData> datas = LoadModelsFromFile(filename);
		shader = new Shaders(&dx);
		shader->init(ps, vs);
		for (int i = 0; i < datas.size(); i++) {
			GEMModel model;
			AnimationInstance* instance = new AnimationInstance;
			model.init(datas[i].modelPath, dx, datas[i].pos, datas[i].scale, datas[i].rotX, datas[i].rotY, datas[i].rotZ);
			models.push_back(&model);
			instance->init(&(model.animation));
			instances.push_back(instance);
		}
	}



	void initshape(string filename, DXcore& dx, string ps, string vs) {

	}

	void Draw(DXcore& dx, float dt) {
		shader->apply();
		for (auto& pair : models) {
			//pair.second->Draw(dx, *shader, instances.find(pair.first), dt, )
		}
	}
};