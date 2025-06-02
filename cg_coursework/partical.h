#pragma once
#include "cgmath.h"
#include <vector>
#include "adapter.h"
#include "Camera.h"

float Random(float a, float b) {
	int A = a * 100;
	int B = b * 100;
	return a + (float)(rand() % (B - A))/100;
}

class Particle
{
public:
	Vec3 position;
	Vec3 velocity;
	float size;
	float life;
	Vec4 color;
	int type; //0 = muzzle flash, 1 = spark, 2 = smoke

	void init(Vec3 origin) {
		position = origin;
		velocity = { 0,0,0 };
		life = 0.2;
		size = 2.0;
		color = Vec4(1.0, 0.9, 0.6, 1.0);
	}
};

struct ParticleVertex
{
	Vec3 position;
	float size;
	Vec4 color;
};

class gunshot
{
	std::vector<Particle> particles;
	ID3D11Buffer* particleVertexBuffer;

	void init(DXcore& dx, int maxParticles) {
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = sizeof(ParticleVertex) * maxParticles; // Maximum particles
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		dx.device->CreateBuffer(&bufferDesc, nullptr, &particleVertexBuffer);
	}

	void SpawnGunshotParticles(Vec3 origin, int sparkCount) {
		Particle muzzleFlash;
		muzzleFlash.position = origin;
		muzzleFlash.velocity = { 0.0f, 0.0f, 0.0f }; // Static
		muzzleFlash.life = 0.2f;
		muzzleFlash.size = 2.0f;
		muzzleFlash.color = { 1.0f, 0.9f, 0.6f, 1.0f }; // Bright yellow
		muzzleFlash.type = 0;
		particles.push_back(muzzleFlash);

		//sparks
		for (int i = 0; i < sparkCount; i++) {
			Particle spark;
			spark.position = origin;
			spark.velocity = {
			Random(-5.0f, 5.0f),
			Random(2.0f, 10.0f),
			Random(-5.0f, 5.0f)
			};
			spark.life = Random(0.2, 0.5);
			spark.size = 0.1f;
			spark.color = Vec4(1, 0.8, 0.2, 1.0);
			spark.type = 1;
			particles.push_back(spark);
		}

		// Smoke
		for (int i = 0; i < 5; i++) {
			Particle smoke;
			smoke.position = origin;
			smoke.velocity = {
				Random(-1.0f, 1.0f),
				Random(0.5f, 2.0f),
				Random(-1.0f, 1.0f)
			};
			smoke.life = Random(0.5f, 1.5f);
			smoke.size = Random(0.5f, 1.0f);
			smoke.color = { 0.5f, 0.5f, 0.5f, 0.8f }; // Gray
			smoke.type = 2;
			particles.push_back(smoke);
		}
	}

	void UpdateParticles(float deltaTime) {
		for (size_t i = 0; i < particles.size(); i++) {

			Particle& particle = particles[i];
			particle.position.x += particle.velocity.x * deltaTime;
			particle.position.y += particle.velocity.y * deltaTime;
			particle.position.z += particle.velocity.z * deltaTime;

			particle.life -= deltaTime;

			//Fade color over time;
			float lifeRatio = particle.life > 0 ? particle.life / 1.0 : 0.f;
			if (particle.type == 1) {
				particle.color = { 1.0f * lifeRatio, 0.8f * lifeRatio, 0.2f * lifeRatio, 1.0f };
			}
			else if (particle.type == 2) { // Smoke
				particle.color.w = lifeRatio; // Fade transparency
			}
			if (particle.life <= 0.0f) {
				particles[i] = particles.back();
				particles.pop_back();
			}
		}
	}

	void UploadParticlesToGPU(DXcore& dx, Camera& camera) {
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		dx.devicecontext->Map(particleVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

		//Copy particle data into the buffer
		ParticleVertex* vertices = static_cast<ParticleVertex*>(mappedResource.pData);
		for (size_t i = 0; i < particles.size(); i++) {
			vertices[i].position = particles[i].position + camera.from + camera.lookat;
			vertices[i].size = particles[i].size;
			vertices[i].color = { particles[i].color.x, particles[i].color.y, particles[i].color.z, particles[i].color.w };
		}

		//Unmap the buffer
		dx.devicecontext->Unmap(particleVertexBuffer, 0);
	}

};

