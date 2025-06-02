#pragma once
#include <iostream>
#include "vertex.h"
#include "collision.h"
#include "texture.h"

class GEMModel
{
public:
    Vec3 position;
    float theta[3]; //Order is x, y, z;
    float scfac; // x, y, z share the same scale factor;
    AABB aabb;
    std::vector<Mesh> meshes;
    Animation animation;
    std::vector<GEMLoader::GEMMesh> gemmeshes;
    GEMLoader::GEMAnimation gemanimation;
    GEMLoader::GEMModelLoader loader;
    std::vector<std::string> textureFilenames;
    std::vector<std::string> actions;
    bool animated = false;
    Matrix World;


    void init(string filename, DXcore& dx, Vec3 vec, float _scfac = 1, float _thetax = 0, float _thetay = 0, float _thetaz = 0) {
        position = vec;
        World = Matrix::Translation(vec);
        theta[0] = _thetax;
        theta[1] = _thetay;
        theta[2] = _thetaz;
        scfac = _scfac;
        aabb.reset();
        loader.load(filename, gemmeshes, gemanimation);
        memcpy(&(animation.skeleton.globalInverse), &(gemanimation.globalInverse), sizeof(Matrix));
        for (int i = 0; i < gemmeshes.size(); i++) {
            Mesh mesh;
            if (gemmeshes[i].isAnimated()) {
                animated = true;
                std::vector<ANIMATED_VERTEX> vertices;
                for (int j = 0; j < gemmeshes[i].verticesAnimated.size(); j++) {
                    ANIMATED_VERTEX v;
                    memcpy(&v, &gemmeshes[i].verticesAnimated[j], sizeof(GEMLoader::GEMAnimatedVertex));
                    vertices.push_back(v);
                    aabb.extend(v.pos);
                }
                textureFilenames.push_back(gemmeshes[i].material.find("diffuse").getValue());
                mesh.init(vertices, gemmeshes[i].indices, dx);
                meshes.push_back(mesh);
            }
            //diffuse, specular, ambient
            else {
                std::vector<STATIC_VERTEX> vertices;
                for (int j = 0; j < gemmeshes[i].verticesStatic.size(); j++) {
                    STATIC_VERTEX v;
                    memcpy(&v, &gemmeshes[i].verticesStatic[j], sizeof(GEMLoader::GEMStaticVertex));
                    vertices.push_back(v);
                    aabb.extend(v.pos);
                }
                textureFilenames.push_back(gemmeshes[i].material.find("diffuse").getValue());
                mesh.init(vertices, gemmeshes[i].indices, dx);
                meshes.push_back(mesh);
            }
        }
        aabb.inimax = aabb.max;
        aabb.inimin = aabb.min;

        for (int i = 0; i < gemanimation.bones.size(); i++)
        {
            Bone bone;
            bone.name = gemanimation.bones[i].name;
            memcpy(&bone.offset, &gemanimation.bones[i].offset, 16 * sizeof(float));
            bone.parentIndex = gemanimation.bones[i].parentIndex;
            animation.skeleton.bones.push_back(bone);
        }
        for (int i = 0; i < gemanimation.animations.size(); i++)
        {
            std::string name = gemanimation.animations[i].name;
            AnimationSequence aseq;
            aseq.ticksPerSecond = gemanimation.animations[i].ticksPerSecond;
            for (int n = 0; n < gemanimation.animations[i].frames.size(); n++)
            {
                AnimationFrame frame;
                for (int index = 0; index < gemanimation.animations[i].frames[n].positions.size(); index++)
                {
                    Vec3 p;
                    Quaternion q;
                    Vec3 s;
                    memcpy(&p, &gemanimation.animations[i].frames[n].positions[index], sizeof(Vec3));
                    frame.positions.push_back(p);
                    memcpy(&q, &gemanimation.animations[i].frames[n].rotations[index], sizeof(Quaternion));
                    frame.rotations.push_back(q);
                    memcpy(&s, &gemanimation.animations[i].frames[n].scales[index], sizeof(Vec3));
                    frame.scales.push_back(s);
                }
                aseq.frames.push_back(frame);
            }
            animation.animations.insert({ name, aseq });
            actions.push_back(name);
        }
    }

    void updateWorld(Shaders& shader, Matrix& w, float sfac = 1, float thetax = 0, float thetay = 0, float thetaz = 0) {
        w = Matrix::Translation(position.x, position.y, position.z);
        w = w * Matrix::Rotation(0, thetax) * Matrix::Rotation(1, thetay) * Matrix::Rotation(2, thetaz);
        w = w * Matrix::Scale(sfac, sfac, sfac);
        World = w;
        aabb.max = w.mulVec(aabb.inimax);
        aabb.min = w.mulVec(aabb.inimin);
        shader.updateConstantVS("World", "W", &w);
        shader.uploadConstantVS("World", "W");
    }

    void updateBones(Shaders& shader, AnimationInstance& instance, float dt, string actiontype) {
        instance.update(actiontype, dt);
        shader.updateConstantVS("animatedMeshBuffer", "bones", instance.matrices);
        shader.uploadConstantVS("animatedMeshBuffer", "bones");
    }

    void Draw(DXcore& dx, Shaders& shader, AnimationInstance& instance, float dt, Matrix& world, TextureManager& tm, string actiontype, float sfac = 1, float thetax = 0, float thetay = 0, float thetaz = 0) {
        if (animated) {
            updateBones(shader, instance, dt, actiontype);
            updateWorld(shader, world, sfac, thetax, thetay, thetaz);
            for (int i = 0; i < meshes.size(); i++)
            {
                shader.updateShaderResources(dx, "tex", tm.find(textureFilenames[i]));
                meshes[i].Draw(dx);
            }
        }
        else {
            updateWorld(shader, world, sfac, thetax, thetay, thetaz);
            for (int i = 0; i < meshes.size(); i++)
            {
                shader.updateShaderResources(dx, "tex", tm.find(textureFilenames[i]));
                meshes[i].Draw(dx);
            }
        }
    }


    void listAnimationNames()
    {
        for (int i = 0; i < gemanimation.animations.size(); i++)
        {
            std::cout << gemanimation.animations[i].name << std::endl;
        }
    }
};

class Trex : public GEMModel
{
public:
    float action_time = 0;
    string currentaction = "walk";
    Vec3 velocity = Vec3(-1, 0, 0.25).normalize();
    //Vec3 initialDirection = Vec3(-1, 0, 0.25).normalize();
    Vec3 initialDirection = Vec3(0, 0, 1).normalize();

    void actiondefine(Vec3 cp, float dt) {
        float distance = cp.distance(position);
        if (distance < 144) {
            currentaction = "attack";
        }
        else if (distance >= 49 && distance < 4900) {
            currentaction = "Run";
            velocity = Vec3((cp - position).x, 0, (cp - position).z).normalize() * 5 * dt;
            position += velocity;
        }
        else {
            currentaction = "walk";
            if (action_time > 0) {
                action_time -= dt;
                velocity = velocity.normalize() * 2 * dt;
                position += velocity;
            }
            else {
                velocity = Vec3(rand() % 10, 0, rand() % 10);
                action_time = 5;
            }
        }
    }

    void updateWorld(Shaders& shader, Matrix& w, Vec3 cp, float dt) {
        actiondefine(cp, dt);
        float theta = acos(initialDirection.dot(velocity.normalize()));
        w = Matrix::Translation(position);
        w = w * Matrix::Rotation(1, theta);
        World = w;
        aabb.max = w.mulVec(aabb.inimax);
        aabb.min = w.mulVec(aabb.inimin);
        shader.updateConstantVS("World", "W", &w);
        shader.uploadConstantVS("World", "W");
    }

    void Draw(DXcore& dx, Shaders& shader, AnimationInstance& instance, float dt, Matrix& world, TextureManager& tm, Vec3 cp) {
        updateWorld(shader, world, cp, dt);
        updateBones(shader, instance, dt, currentaction);
        for (int i = 0; i < meshes.size(); i++)
        {
            shader.updateShaderResources(dx, "tex", tm.find(textureFilenames[i]));
            meshes[i].Draw(dx);
        }
    }

};