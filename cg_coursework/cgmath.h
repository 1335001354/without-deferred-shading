#pragma once
#include <iostream>
#include <cmath>

#define SQ(x) (x) * (x)
#define _USE_MATH_DEFINES
#define M_PI 3.1415926

using namespace std;

template<typename T>
static T lerp(const T a, const T b, float t) {
	return a * (1.f - t) + (b * t);
}

struct Vec2 {
	float x, y;
	Vec2() {}
	Vec2(const float _x, const float _y) :x(_x), y(_y) {}
};

struct Vec3
{
	float x, y, z;
	Vec3() {}
	Vec3(const float _x, const float _y, const float _z) : x(_x), y(_y), z(_z) {}

	float lengthsquare() {
		return x * x + y * y + z * z;
	}

	float length() {
		return sqrtf(lengthsquare());
	}

	Vec3 normalize() {
		float len = 1.f / length();
		return Vec3(x * len, y * len, z * len);
	}

	Vec3 Cross(const Vec3& v1)
	{
		return Vec3(y * v1.z - z * v1.y,
			z * v1.x - x * v1.z,
			x * v1.y - y * v1.x);
	}

	float dot(Vec3 v) {
		return x * v.x + y * v.y + z * v.z;
	}

	float distance(Vec3 v) {
		return (x - v.x) * (x - v.x) + (y - v.y) * (y - v.y) + (z - v.z) * (z - v.z);
	}

	void set_value(float _x, float _y, float _z) {
		x = _x;
		y = _y;
		z = _z;
	}

	bool operator==(const Vec3 v) {
		if (x == v.x && y == v.y && z == v.z)return true;
		else return false;
	}

	Vec3 operator-(const Vec3 v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
	Vec3 operator+(const Vec3 v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
	Vec3 operator*(const float a) const { return Vec3(x * a, y * a, z * a); }
	void operator-=(const Vec3 v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
	}
	void operator+=(const Vec3 v) {
		x += v.x;
		y += v.y;
		z += v.z;
	}
	void operator*=(const float a) {
		x *= a;
		y *= a;
		z *= a;
	}


	Vec3* Schmit_orthono() {
		Vec3 u(0, 1, 0);
		Vec3 gaze = normalize();
		Vec3 right = u.Cross(gaze).normalize();
		Vec3 up = gaze.Cross(right);
		Vec3 frame[3] = { gaze, up, right };
		return frame;
	}
};

struct Vec4 : public Vec3
{
	float w;
	Vec4() {}
	Vec4(Vec3 v, float _w) {
		x = v.x;
		y = v.y;
		z = v.z;
		w = _w;
	}
	Vec4(const float _x, const float _y, const float _z, const float _w) : Vec3(_x, _y, _z) {
		w = _w;
	}

};

class Matrix
{
public:
	union
	{
		float a[4][4];
		float m[16];
	};

	Matrix() {
		for (int i = 0; i < 16; i++) {
			m[i] = 0;
		}
	}
	Matrix(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33) {
		m[0] = m00;
		m[1] = m01;
		m[2] = m02;
		m[3] = m03;
		m[4] = m10;
		m[5] = m11;
		m[6] = m12;
		m[7] = m13;
		m[8] = m20;
		m[9] = m21;
		m[10] = m22;
		m[11] = m23;
		m[12] = m30;
		m[13] = m31;
		m[14] = m32;
		m[15] = m33;
	}
	Matrix(float* otherMatrix) {
		memcpy(&m, otherMatrix, 16 * sizeof(float));
	}


	float& operator[](int index) {
		return m[index];
	}

	void vectorstobase(const Vec4 v0, const Vec4 v1, const Vec4 v2) {
		m[0] = v0.x;
		m[1] = v1.x;
		m[2] = v2.x;
		m[4] = v0.y;
		m[5] = v1.y;
		m[6] = v2.y;
		m[8] = v0.z;
		m[9] = v1.z;
		m[10] = v2.z;
	}

	void translation_set(const float _x, const float _y, const float _z) {
		m[3] = _x;
		m[7] = _y;
		m[11] = _z;
	}

	void identity() {
		memset(this, 0, 16);
		m[0] = 1;
		m[5] = 1;
		m[10] = 1;
		m[15] = 1;
	}

	Vec3 mulVec(const Vec3& v)
	{
		float w = m[12] * v.x + m[13] * v.y + m[14] * v.z + m[15];
		return Vec3(
			(v.x * m[0] + v.y * m[1] + v.z * m[2] + m[3]) / w,
			(v.x * m[4] + v.y * m[5] + v.z * m[6] + m[7]) / w,
			(v.x * m[8] + v.y * m[9] + v.z * m[10] + m[11] / w));
	}

	/*Vec3 mulVec(const Vec3& v)
	{
		return Vec3(
			(v.x * m[0] + v.y * m[1] + v.z * m[2]),
			(v.x * m[4] + v.y * m[5] + v.z * m[6]),
			(v.x * m[8] + v.y * m[9] + v.z * m[10]));
	}*/

	Matrix mul(const Matrix& matrix) const
	{
		Matrix ret;

		ret[0] = m[0] * matrix.m[0] + m[1] * matrix.m[4] + m[2] * matrix.m[8] + m[3] * matrix.m[12];
		ret[1] = m[0] * matrix.m[1] + m[1] * matrix.m[5] + m[2] * matrix.m[9] + m[3] * matrix.m[13];
		ret[2] = m[0] * matrix.m[2] + m[1] * matrix.m[6] + m[2] * matrix.m[10] + m[3] * matrix.m[14];
		ret[3] = m[0] * matrix.m[3] + m[1] * matrix.m[7] + m[2] * matrix.m[11] + m[3] * matrix.m[15];
		ret[4] = m[4] * matrix.m[0] + m[5] * matrix.m[4] + m[6] * matrix.m[8] + m[7] * matrix.m[12];
		ret[5] = m[4] * matrix.m[1] + m[5] * matrix.m[5] + m[6] * matrix.m[9] + m[7] * matrix.m[13];
		ret[6] = m[4] * matrix.m[2] + m[5] * matrix.m[6] + m[6] * matrix.m[10] + m[7] * matrix.m[14];
		ret[7] = m[4] * matrix.m[3] + m[5] * matrix.m[7] + m[6] * matrix.m[11] + m[7] * matrix.m[15];
		ret[8] = m[8] * matrix.m[0] + m[9] * matrix.m[4] + m[10] * matrix.m[8] + m[11] * matrix.m[12];
		ret[9] = m[8] * matrix.m[1] + m[9] * matrix.m[5] + m[10] * matrix.m[9] + m[11] * matrix.m[13];
		ret[10] = m[8] * matrix.m[2] + m[9] * matrix.m[6] + m[10] * matrix.m[10] + m[11] * matrix.m[14];
		ret[11] = m[8] * matrix.m[3] + m[9] * matrix.m[7] + m[10] * matrix.m[11] + m[11] * matrix.m[15];
		ret[12] = m[12] * matrix.m[0] + m[13] * matrix.m[4] + m[14] * matrix.m[8] + m[15] * matrix.m[12];
		ret[13] = m[12] * matrix.m[1] + m[13] * matrix.m[5] + m[14] * matrix.m[9] + m[15] * matrix.m[13];
		ret[14] = m[12] * matrix.m[2] + m[13] * matrix.m[6] + m[14] * matrix.m[10] + m[15] * matrix.m[14];
		ret[15] = m[12] * matrix.m[3] + m[13] * matrix.m[7] + m[14] * matrix.m[11] + m[15] * matrix.m[15];
		return ret;
	}


	Matrix operator*(const Matrix& matrix) {
		return mul(matrix);
	}


	Matrix Transpose() {
		Matrix _transpose;
		for (int i = 0; i < 4; i++) {
			for (int n = 0; n < 4; n++) {
				_transpose.a[n][i] = a[i][n];
			}
		}
		return _transpose;
	}

	float determinant() {
		return
			a[0][3] * a[1][2] * a[2][1] * a[3][0] - a[0][2] * a[1][3] * a[2][1] * a[3][0] -
			a[0][3] * a[1][1] * a[2][2] * a[3][0] + a[0][1] * a[1][3] * a[2][2] * a[3][0] +
			a[0][2] * a[1][1] * a[2][3] * a[3][0] - a[0][1] * a[1][2] * a[2][3] * a[3][0] -
			a[0][3] * a[1][2] * a[2][0] * a[3][1] + a[0][2] * a[1][3] * a[2][0] * a[3][1] +
			a[0][3] * a[1][0] * a[2][2] * a[3][1] - a[0][0] * a[1][3] * a[2][2] * a[3][1] -
			a[0][2] * a[1][0] * a[2][3] * a[3][1] + a[0][0] * a[1][2] * a[2][3] * a[3][1] +
			a[0][3] * a[1][1] * a[2][0] * a[3][2] - a[0][1] * a[1][3] * a[2][0] * a[3][2] -
			a[0][3] * a[1][0] * a[2][1] * a[3][2] + a[0][0] * a[1][3] * a[2][1] * a[3][2] +
			a[0][1] * a[1][0] * a[2][3] * a[3][2] - a[0][0] * a[1][1] * a[2][3] * a[3][2] -
			a[0][2] * a[1][1] * a[2][0] * a[3][3] + a[0][1] * a[1][2] * a[2][0] * a[3][3] +
			a[0][2] * a[1][0] * a[2][1] * a[3][3] - a[0][0] * a[1][2] * a[2][1] * a[3][3] -
			a[0][1] * a[1][0] * a[2][2] * a[3][3] + a[0][0] * a[1][1] * a[2][2] * a[3][3];
	}


	static Matrix Translation(float _x, float _y, float _z) {
		Matrix tr;
		tr.identity();
		tr[3] = _x;
		tr[7] = _y;
		tr[11] = _z;
		return tr;
	}

	static Matrix Translation(Vec3 vec) {
		Matrix tr;
		tr.identity();
		tr[3] = vec.x;
		tr[7] = vec.y;
		tr[11] = vec.z;
		return tr;
	}

	static Matrix Rotation(int xyz, float _theta) {
		Matrix ro;
		float theta = _theta;
		double cos = std::cos(theta);
		double sin = std::sin(theta);
		ro.identity();
		if (xyz == 0) {
			ro.m[5] = cos;
			ro.m[6] = -1 * sin;
			ro.m[9] = sin;
			ro.m[10] = cos;
		}
		else if (xyz == 1) {
			ro.m[0] = cos;
			ro.m[2] = sin;
			ro.m[8] = -1 * sin;
			ro.m[10] = cos;
		}
		else if (xyz == 2) {
			ro.m[0] = cos;
			ro.m[1] = -sin;
			ro.m[4] = sin;
			ro.m[5] = cos;
		}
		return ro;
	}

	static Matrix Rotation(Vec3 axis, float theta) {
		double cos = std::cos(theta);
		double sin = std::sin(theta);
		double t = 1 - cos;
		Matrix ro;
		ro.identity();
		ro[0] = t * axis.x * axis.x + cos;
		ro[1] = t * axis.x * axis.y - sin * axis.z;
		ro[2] = t * axis.x * axis.z + sin * axis.y;
		ro[4] = t * axis.x * axis.y + sin * axis.z;
		ro[5] = t * axis.y * axis.y + cos;
		ro[6] = t * axis.y * axis.z - sin * axis.x;
		ro[8] = t * axis.x * axis.z - sin * axis.y;
		ro[9] = t * axis.y * axis.z + sin * axis.x;
		ro[10] = t * axis.z * axis.z + cos;
		return ro;
	}

	static Matrix Scale(float xratio, float yratio, float zratio) {
		Matrix sc;
		sc.identity();
		sc.m[0] = xratio;
		sc.m[5] = yratio;
		sc.m[10] = zratio;
		return sc;
	}

	static Matrix Scale(Vec3 v) {
		Matrix sc;
		sc.identity();
		sc.m[0] = v.x;
		sc.m[5] = v.y;
		sc.m[10] = v.z;
		return sc;
	}

	const void ViewMatrix(Vec3 from, Vec3 lookat) {
		lookat.normalize();
		Vec3* frame = lookat.Schmit_orthono();
		lookat = frame[0];
		Vec3 up = frame[1];
		Vec3 right = frame[2];

		m[0] = right.x;
		m[1] = right.y;
		m[2] = right.z;
		m[3] = -1 * right.dot(from);
		m[4] = up.x;
		m[5] = up.y;
		m[6] = up.z;
		m[7] = -1 * up.dot(from);
		m[8] = lookat.x;
		m[9] = lookat.y;
		m[10] = lookat.z;
		m[11] = -1 * lookat.dot(from);
		m[15] = 1;

	}


	Matrix invert()
	{
		Matrix inv;
		inv.m[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
		inv.m[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
		inv.m[8] = m[4] * m[9] * m[15] - m[8] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
		inv.m[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
		inv.m[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
		inv.m[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
		inv.m[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
		inv.m[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
		inv.m[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
		inv.m[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
		inv.m[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
		inv.m[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
		inv.m[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
		inv.m[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
		inv.m[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
		inv.m[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];
		float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
		if (det == 0) {
			// Handle this case
		}
		det = 1.0 / det;
		for (int i = 0; i < 16; i++) {
			inv[i] = inv[i] * det;
		}
		return inv;
	}

};

class Quaternion
{
public:
	float w, x, y, z;

	Quaternion() : w(1.f), x(0.f), y(0.f), z(0.f) {}
	Quaternion(float _w, float _x, float _y, float _z) : w(_w), x(_x), y(_y), z(_z) {}

	float norm() const {
		return std::sqrt(w * w + x * x + y * y + z * z);
	}

	Quaternion normalize() const {
		float n = norm();
		if (n == 0.f) return Quaternion();
		return Quaternion(w / n, x / n, y / n, z / n);
	}

	Quaternion conjugate() const {
		return Quaternion(w, -x, -y, -z);
	}

	Quaternion operator*(const Quaternion& q) const {
		return Quaternion(
			w * q.w - x * q.x - y * q.y - z * q.z, // Scalar part
			w * q.x + x * q.w + y * q.z - z * q.y, // Vector part (x)
			w * q.y - x * q.z + y * q.w + z * q.x, // Vector part (y)
			w * q.z + x * q.y - y * q.x + z * q.w  // Vector part (z)
		);
	}

	Vec3 rotateVector(Vec3 vec) const {
		Quaternion vectorQuat(0, vec.x, vec.y, vec.z);
		Quaternion result = (*this) * vectorQuat * conjugate();
		return Vec3(result.x, result.y, result.z);
	}

	static Quaternion slerp(const Quaternion& q1, const Quaternion& q2, float t) {
		//clamp t between 0 and 1
		t = std::fmax(0.f, std::fmin(1.f, t));
		float dot = q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;
		if (dot > 1.0f) dot = 1.0f;
		if (dot < -1.0f) dot = -1.0f;
		//If the dot product is negative, negate one quaternion to ensure shortest path
		Quaternion q2Modified = (dot < 0.0f) ? Quaternion(-q2.w, -q2.x, -q2.y, -q2.z) : q2;
		//Interpolate
		float theta = std::acos(dot);
		float sinTheta = std::sqrt(1.f - dot * dot);
		if (std::fabs(sinTheta) < 1e-6) { // If sin(theta) is near zero, use linear interpolation
			return Quaternion(
				(1 - t) * q1.w + t * q2Modified.w,
				(1 - t) * q1.x + t * q2Modified.x,
				(1 - t) * q1.y + t * q2Modified.y,
				(1 - t) * q1.z + t * q2Modified.z
			).normalize();
		}
		else {
			float a = std::sin((1 - t) * theta) / sinTheta;
			float b = std::sin(t * theta) / sinTheta;
			return Quaternion(
				a * q1.w + b * q2Modified.w,
				a * q1.x + b * q2Modified.x,
				a * q1.y + b * q2Modified.y,
				a * q1.z + b * q2Modified.z
			).normalize();
		}
	}

	Matrix toMatrix() const {
		Quaternion q = this->normalize();
		float x = q.w;
		float y = q.x;
		float z = q.y;
		float w = q.z;
		float xx = x * x;
		float yy = y * y;
		float zz = z * z;
		float xy = x * y;
		float xz = x * z;
		float yz = y * z;
		float wx = w * x;
		float wy = w * y;
		float wz = w * z;
		return Matrix(1.0f - 2.0f * (yy + zz), 2.0f * (xy - wz), 2.0f * (xz + wy), 0.0f, 2.0f * (xy + wz), 1.0f - 2.0f * (xx + zz), 2.0f * (yz - wx), 0.0f, 2.0f * (xz - wy), 2.0f * (yz + wx), 1.0f - 2.0f * (xx + yy), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	}
};