#pragma once
#include "cgmath.h"
#include "shader.h"

struct STATIC_VERTEX
{
	Vec3 pos;
	Vec3 normal;
	Vec3 tangent;
	float tu;
	float tv;
	unsigned int BonesID[4] = { 0,0,0,0 };
	float boneWeights[4] = { 0.f,0.f,0.f,0.f };
	int type = 0;
};

struct ANIMATED_VERTEX
{
	Vec3 pos;
	Vec3 normal;
	Vec3 tangent;
	float tu;
	float tv;
	unsigned int bonesIDs[4];
	float boneWeights[4];
	int type = 1;
};

struct VS_OUTPUT
{
	Vec4 position;
	Vec3 normal;
	Vec3 tangent;
	Vec2 TexCoords;
};

struct Bone
{
	std::string name;
	Matrix offset;
	int parentIndex;
};

struct Skeleton
{
	std::vector<Bone> bones;
	Matrix globalInverse;
};

struct AnimationFrame
{
	std::vector<Vec3> positions;
	std::vector<Quaternion> rotations;
	std::vector<Vec3> scales;
};

class AnimationSequence
{
public:
	std::vector<AnimationFrame> frames;
	float ticksPerSecond;
	Vec3 interpolate(Vec3 p1, Vec3 p2, float t) {
		return ((p1 * (1.f - t)) + (p2 * t));
	}
	Quaternion interpolate(Quaternion q1, Quaternion q2, float t) {
		return Quaternion::slerp(q1, q2, t);
	}
	float duration() {
		return ((float)frames.size() / ticksPerSecond);
	}
	void calcFrame(float t, int& frame, float& interpolationFact) {
		//t represents the elapsed time in seconds
		//ticksPerSecond represents the number of animation ticks per second.
		interpolationFact = t * ticksPerSecond;
		frame = (int)floorf(interpolationFact);
		interpolationFact = interpolationFact - (float)frame;
		frame = min(frame, frames.size() - 1);
	}
	int nextFrame(int frame) {
		return min(frame + 1, frames.size() - 1);
	}
	Matrix interpolateBoneToGlobal(Matrix* matrices, int baseFrame, float interpolationFact, Skeleton* skeleton, int boneIndex) {
		int nextFrameIndex = nextFrame(baseFrame);
		Matrix scale = Matrix::Scale(interpolate(frames[baseFrame].scales[boneIndex], frames[nextFrameIndex].scales[boneIndex], interpolationFact));
		Matrix rotation = interpolate(frames[baseFrame].rotations[boneIndex], frames[nextFrameIndex].rotations[boneIndex], interpolationFact).toMatrix();
		Matrix translation = Matrix::Translation(interpolate(frames[baseFrame].positions[boneIndex], frames[nextFrameIndex].positions[boneIndex], interpolationFact));
		Matrix local = translation * rotation * scale; //Scale->Rotate->Translate, order matter!
		if (skeleton->bones[boneIndex].parentIndex > -1) {
			Matrix global = matrices[skeleton->bones[boneIndex].parentIndex] * local; ///!!!!!!!
			return global;
		}
		return local; //What's local, what's globle
	}
};

class Animation
{
public:
	std::map<std::string, AnimationSequence> animations;
	Skeleton skeleton;

	void calcFrame(std::string name, float t, int& frame, float& interpolationFact) {
		animations[name].calcFrame(t, frame, interpolationFact);
	}
	Matrix interpolateBoneToGlobal(std::string name, Matrix* matrices, int baseFrame, float interpolationFact, int boneIndex) {
		return animations[name].interpolateBoneToGlobal(matrices, baseFrame, interpolationFact, &skeleton, boneIndex);
	}
	void calcFinalTransforms(Matrix* matrices) {
		for (int i = 0; i < skeleton.bones.size(); i++) {
			matrices[i] = skeleton.globalInverse * matrices[i] * skeleton.bones[i].offset; //!!!!!!
		}
	}
};

class AnimationInstance {
public:
	Animation* animation;
	std::string currentAnimation;
	float t;
	Matrix matrices[256];
	void init(Animation* _animation) {
		animation = _animation;
	}
	void resetAnimationTime() {
		t = 0;
	}

	bool animationFinished() {
		if (t > animation->animations[currentAnimation].duration())
		{
			return true;
		}
		return false;
	}

	void update(std::string name, float dt) {
		if (name == currentAnimation) t += dt;
		else {
			currentAnimation = name;
			t = 0;
		}
		if (animationFinished() == true) {
			resetAnimationTime();
		}
		int frame = 0;
		float interpolationFact = 0;
		animation->calcFrame(name, t, frame, interpolationFact);
		for (int i = 0; i < animation->skeleton.bones.size(); i++) {
			matrices[i] = animation->interpolateBoneToGlobal(name, matrices, frame, interpolationFact, i);
		}

		animation->calcFinalTransforms(matrices);
	}
};

class Mesh
{
public:
	ID3D11Buffer* indexBuffer;
	ID3D11Buffer* vertexBuffer;
	//ID3D11Buffer* streamOutputBuffer;
	ID3D11Buffer* stagingBuffer;
	//AABB aabb;
	int indicesSize;
	//int vertexSize;
	UINT strides;

	void init(void* vertices, int vertexSizeInBytes, int numVertices, unsigned int* indices, int numIndices, DXcore& dx) {
		D3D11_BUFFER_DESC bd;
		memset(&bd, 0, sizeof(D3D11_BUFFER_DESC));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(unsigned int) * numIndices;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		D3D11_SUBRESOURCE_DATA data;
		memset(&data, 0, sizeof(D3D11_SUBRESOURCE_DATA));
		data.pSysMem = indices;
		dx.device->CreateBuffer(&bd, &data, &indexBuffer);
		bd.ByteWidth = vertexSizeInBytes * numVertices;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		data.pSysMem = vertices;
		dx.device->CreateBuffer(&bd, &data, &vertexBuffer);
		indicesSize = numIndices;
		strides = vertexSizeInBytes;
		//vertexSize = numVertices;
		//SetupOutputBuffer(dx);
		//SetupStagingBuffer(dx);
		//aabb.reset();
	}

	void init(std::vector<STATIC_VERTEX> vertices, std::vector<unsigned int> indices, DXcore& dx)
	{
		for (int i = 0; i < vertices.size(); i++) {
			//aabb.extend(vertices[i].pos);
		}
		init(&vertices[0], sizeof(STATIC_VERTEX), vertices.size(), &indices[0], indices.size(), dx);
	}

	void init(std::vector<ANIMATED_VERTEX> vertices, std::vector<unsigned int> indices, DXcore& dx)
	{
		for (int i = 0; i < vertices.size(); i++) {
			//aabb.extend(vertices[i].pos);
		}
		init(&vertices[0], sizeof(ANIMATED_VERTEX), vertices.size(), &indices[0], indices.size(), dx);
	}

	void Draw(DXcore& dx) {
		UINT offsets = 0;
		dx.devicecontext->IASetVertexBuffers(0, 1, &vertexBuffer, &strides, &offsets);
		dx.devicecontext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		//dx.devicecontext->SOSetTargets(1, &streamOutputBuffer, &offsets);
		dx.devicecontext->DrawIndexed(indicesSize, 0, 0);
		//CopyData(dx);
		//cleanSO(dx);
	}

	//void SetupOutputBuffer(DXcore& dx) {
	//	D3D11_BUFFER_DESC soBufferDesc = {};
	//	soBufferDesc.ByteWidth = sizeof(VS_OUTPUT) * vertexSize;
	//	soBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	//	soBufferDesc.BindFlags = D3D11_BIND_STREAM_OUTPUT;
	//	soBufferDesc.CPUAccessFlags = 0;
	//	soBufferDesc.MiscFlags = 0;
	//	soBufferDesc.StructureByteStride = sizeof(VS_OUTPUT);

	//	HRESULT hr = dx.device->CreateBuffer(&soBufferDesc, nullptr, &streamOutputBuffer);
	//}

	//void SetupStagingBuffer(DXcore& dx) {
	//	D3D11_BUFFER_DESC stagingDesc = {};
	//	stagingDesc.Usage = D3D11_USAGE_STAGING;                 // CPU-accessible
	//	stagingDesc.ByteWidth = sizeof(VS_OUTPUT) * vertexSize;  // Set the correct size
	//	stagingDesc.BindFlags = 0;                               // No binding flags
	//	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;      // Allow reading from the CPU

	//	HRESULT hr = dx.device->CreateBuffer(&stagingDesc, nullptr, &stagingBuffer);
	//}

	//void CopyData(DXcore& dx) {
	//	dx.devicecontext->CopyResource(stagingBuffer, streamOutputBuffer);
	//	D3D11_MAPPED_SUBRESOURCE mappedResource = {};
	//	HRESULT hr = dx.devicecontext->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &mappedResource);
	//	VS_OUTPUT* vertices = reinterpret_cast<VS_OUTPUT*>(mappedResource.pData);
	//	for (size_t i = 0; i < vertexSize; i++) {
	//		aabb.extend(Vec4(vertices[i].position.x, vertices[i].position.y, vertices[i].position.z, vertices[i].position.w));
	//	}
	//}

	//void cleanSO(DXcore& dx){
	//	ID3D11Buffer* nullBuffer = nullptr;
	//	UINT nullOffset = 0;
	//	dx.devicecontext->SOSetTargets(1, &nullBuffer, &nullOffset);
	//}

};

class Shape
{
public:
	Mesh mesh;
	Vec3 position;

	void updateWorld(Shaders& shader, Matrix& w) {
		w = Matrix::Translation(position.x, position.y, position.z);
		shader.updateConstantVS("World", "W", &w);
		shader.uploadConstantVS("World", "W");
	}

	void Draw(DXcore& dx, Shaders& shader, Matrix& w) {
		updateWorld(shader, w);
		mesh.Draw(dx);
	}

	STATIC_VERTEX addVertex(Vec3 p, Vec3 n, float tu, float tv) {
		STATIC_VERTEX v;
		v.pos = p;
		v.normal = n;
		//Frame frame; //shading frame
		//frame.fromVector(n);
		v.tangent = Vec3(0, 0, 0); // For now
		v.tu = tu;
		v.tv = tv;
		return v;
	}
};

class Plane : public Shape
{
public:
	Vec3 p0;
	Vec3 p1;
	Vec3 p2;
	Vec3 p3;

	void init(DXcore& dx, Vec3 _position) {
		position = _position;
		p0 = Vec3(-15, 0, -15);
		p1 = Vec3(15, 0, -15);
		p2 = Vec3(-15, 0, 15);
		p3 = Vec3(15, 0, 15);
		std::vector<STATIC_VERTEX> vertices;
		vertices.push_back(addVertex(p0, Vec3(0, 1, 0), 0, 0));
		vertices.push_back(addVertex(p1, Vec3(0, 1, 0), 1, 0));
		vertices.push_back(addVertex(p2, Vec3(0, 1, 0), 0, 1));
		vertices.push_back(addVertex(p3, Vec3(0, 1, 0), 1, 1));
		std::vector<unsigned int> indices;
		indices.push_back(2); indices.push_back(1); indices.push_back(0);
		indices.push_back(1); indices.push_back(2); indices.push_back(3);
		mesh.init(vertices, indices, dx);
	}

};

class Cube : public Shape
{
public:
	Vec3 p0;
	Vec3 p1;
	Vec3 p2;
	Vec3 p3;
	Vec3 p4;
	Vec3 p5;
	Vec3 p6;
	Vec3 p7;

	void sizeconfirm(float fac) {
		p0 *= fac;
		p1 *= fac;
		p2 *= fac;
		p3 *= fac;
		p4 *= fac;
		p5 *= fac;
		p6 *= fac;
		p7 *= fac;

	}

	void init(DXcore& dx, float fac, Vec3 _position) {
		position = _position;
		p0 = Vec3(-1.0f, -1.0f, -1.0f);
		p1 = Vec3(1.0f, -1.0f, -1.0f);
		p2 = Vec3(1.0f, 1.0f, -1.0f);
		p3 = Vec3(-1.0f, 1.0f, -1.0f);
		p4 = Vec3(-1.0f, -1.0f, 1.0f);
		p5 = Vec3(1.0f, -1.0f, 1.0f);
		p6 = Vec3(1.0f, 1.0f, 1.0f);
		p7 = Vec3(-1.0f, 1.0f, 1.0f);
		sizeconfirm(fac);
		std::vector<STATIC_VERTEX> vertices;
		std::vector<unsigned int> indices;

		vertices.push_back(addVertex(p0, Vec3(0.0f, 0.0f, -1.0f), 0.0f, 1.0f));
		vertices.push_back(addVertex(p1, Vec3(0.0f, 0.0f, -1.0f), 1.0f, 1.0f));
		vertices.push_back(addVertex(p2, Vec3(0.0f, 0.0f, -1.0f), 1.0f, 0.0f));
		vertices.push_back(addVertex(p3, Vec3(0.0f, 0.0f, -1.0f), 0.0f, 0.0f));

		vertices.push_back(addVertex(p5, Vec3(0.0f, 0.0f, 1.0f), 0.0f, 1.0f));
		vertices.push_back(addVertex(p4, Vec3(0.0f, 0.0f, 1.0f), 1.0f, 1.0f));
		vertices.push_back(addVertex(p7, Vec3(0.0f, 0.0f, 1.0f), 1.0f, 0.0f));
		vertices.push_back(addVertex(p6, Vec3(0.0f, 0.0f, 1.0f), 0.0f, 0.0f));

		vertices.push_back(addVertex(p4, Vec3(-1.0f, 0.0f, 0.0f), 0.0f, 1.0f));
		vertices.push_back(addVertex(p0, Vec3(-1.0f, 0.0f, 0.0f), 1.0f, 1.0f));
		vertices.push_back(addVertex(p3, Vec3(-1.0f, 0.0f, 0.0f), 1.0f, 0.0f));
		vertices.push_back(addVertex(p7, Vec3(-1.0f, 0.0f, 0.0f), 0.0f, 0.0f));

		vertices.push_back(addVertex(p1, Vec3(1.0f, 0.0f, 0.0f), 0.0f, 1.0f));
		vertices.push_back(addVertex(p5, Vec3(1.0f, 0.0f, 0.0f), 1.0f, 1.0f));
		vertices.push_back(addVertex(p6, Vec3(1.0f, 0.0f, 0.0f), 1.0f, 0.0f));
		vertices.push_back(addVertex(p2, Vec3(1.0f, 0.0f, 0.0f), 0.0f, 0.0f));

		vertices.push_back(addVertex(p3, Vec3(0.0f, 1.0f, 0.0f), 0.0f, 1.0f));
		vertices.push_back(addVertex(p2, Vec3(0.0f, 1.0f, 0.0f), 1.0f, 1.0f));
		vertices.push_back(addVertex(p6, Vec3(0.0f, 1.0f, 0.0f), 1.0f, 0.0f));
		vertices.push_back(addVertex(p7, Vec3(0.0f, 1.0f, 0.0f), 0.0f, 0.0f));

		vertices.push_back(addVertex(p4, Vec3(0.0f, -1.0f, 0.0f), 0.0f, 1.0f));
		vertices.push_back(addVertex(p5, Vec3(0.0f, -1.0f, 0.0f), 1.0f, 1.0f));
		vertices.push_back(addVertex(p1, Vec3(0.0f, -1.0f, 0.0f), 1.0f, 0.0f));
		vertices.push_back(addVertex(p0, Vec3(0.0f, -1.0f, 0.0f), 0.0f, 0.0f));

		//Add indices
		indices.push_back(0); indices.push_back(1); indices.push_back(2);
		indices.push_back(0); indices.push_back(2); indices.push_back(3);
		indices.push_back(4); indices.push_back(5); indices.push_back(6);
		indices.push_back(4); indices.push_back(6); indices.push_back(7);
		indices.push_back(8); indices.push_back(9); indices.push_back(10);
		indices.push_back(8); indices.push_back(10); indices.push_back(11);
		indices.push_back(12); indices.push_back(13); indices.push_back(14);
		indices.push_back(12); indices.push_back(14); indices.push_back(15);
		indices.push_back(16); indices.push_back(17); indices.push_back(18);
		indices.push_back(16); indices.push_back(18); indices.push_back(19);
		indices.push_back(20); indices.push_back(21); indices.push_back(22);
		indices.push_back(20); indices.push_back(22); indices.push_back(23);
		mesh.init(vertices, indices, dx);
	}
};

class Sphere : public Shape
{
public:
	int rings;
	int segments;
	float radius;
	Vec3 position;

	void init(DXcore& dx, float fac, Vec3 _position) {

		std::vector<STATIC_VERTEX> vertices;
		std::vector<unsigned int> indices;

		rings = 16;
		segments = 32;
		radius = fac;

		for (int lat = 0; lat <= rings; lat++) {
			float theta = lat * M_PI / rings;
			float sinTheta = sinf(theta);
			float cosTheta = cosf(theta);
			for (int lon = 0; lon <= segments; lon++) {
				float phi = lon * 2.0f * M_PI / segments;
				float sinPhi = sinf(phi);
				float cosPhi = cosf(phi);
				Vec3 position(radius * sinTheta * cosPhi, radius * cosTheta, radius * sinTheta * sinPhi);
				Vec3 normal = position.normalize();
				float tu = (float)lon / segments;
				float tv = (float)lat / rings;
				vertices.push_back(addVertex(position, normal, tu, tv));
			}
		}

		for (int lat = 0; lat < rings; lat++)
		{
			for (int lon = 0; lon < segments; lon++)
			{
				int current = lat * (segments + 1) + lon;
				int next = current + segments + 1;
				indices.push_back(current);
				indices.push_back(next);
				indices.push_back(current + 1);
				indices.push_back(current + 1);
				indices.push_back(next);
				indices.push_back(next + 1);
			}
		}
		mesh.init(vertices, indices, dx);
	}

};