#pragma once
#include <cfloat>
#include "cgmath.h"
#include <D3D11.h>


class AABB
{
public:
	Vec3 max;
	Vec3 inimax;
	Vec3 min;
	Vec3 inimin;

	AABB() { reset(); }

	void reset()
	{
		max = Vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		min = Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	}

	void extend(const Vec3& p) {
		max = Max(max, p);
		min = Min(min, p);
	}

	bool intersects(const AABB& other, Matrix world) const {
		Matrix World = world.invert();
		Vec3 Max = World.mulVec(other.max);
		Vec3 Min = World.mulVec(other.min);
		return !(max.x < Min.x || min.x > other.max.x || // Separation on X-axis
			max.y < other.min.y || min.y > other.max.y || // Separation on Y-axis
			max.z < other.min.z || min.z > other.max.z);  // Separation on Z-axis
	}
	Vec3 Max(Vec3 v1, Vec3 v2) {
		return Vec3(max(v1.x, v2.x), max(v1.y, v2.y), max(v1.z, v2.z));
	}

	Vec3 Min(Vec3 v1, Vec3 v2) {
		return Vec3(min(v1.x, v2.x), min(v1.y, v2.y), min(v1.z, v2.z));
	}
};

class sphere
{
public:
	Vec3 position;
	float radius;

	void init(Vec3 _position, float _radius) {
		position = _position;
		radius = _radius;
	}

	void update(Vec3 _position) {
		position = _position;
	}

	Vec3 intersects(const AABB& other, Matrix world) { //world is aabb's model
		Matrix World = world.invert();
		Vec3 Maxp = World.mulVec(other.max);
		Vec3 Minp = World.mulVec(other.min);
		Vec3 Position = World.mulVec(position);
		//cout << "position " << position.x << " " << position.y << " " << position.z << endl;
		//cout << "Position " << Position.x << " " << Position.y << " " << Position.z << endl;
 		float closestX = max(Minp.x, min(Position.x, Maxp.x));
		float closestY = max(Minp.y, min(Position.y, Maxp.y));
		float closestZ = max(Minp.z, min(Position.z, Maxp.z));
		Vec3 dist = Vec3(Position.x - closestX, Position.y - closestY, Position.z - closestZ);
		//cout << "distance: " << dist.x << " " << dist.y << " " << dist.z << endl;
		if (dist.lengthsquare() <= radius * radius) {
			cout << "closest: " << closestX  << " " << closestY << " " << closestZ << endl;
			return (position - world.mulVec(Vec3(closestX, closestY, closestZ))).normalize();
		}
		return Vec3(0, 0, 0); //The function return a normalized vector whose direction is from the cloest point on the box to the sphere's center.
	}
};