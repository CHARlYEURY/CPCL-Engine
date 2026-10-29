#pragma once
#include "Matrix4x4_SSE.h"
#include "Vec3_SSE.h"

class Cam {

private:
	Maths::Vec3_SSE _position;
	float _yaw, _pitch = 0.0f;
	float _speed = 3.0f;
	float _sensitivity = 0.03f;

	Maths::Vec3_SSE _up = Maths::Vec3_SSE(0.0f, 1.0f, 0.0f);
public:

	explicit Cam(const Maths::Vec3_SSE& position = Maths::Vec3_SSE(0, 0, 3));

	Maths::Vec3_SSE forward() const;
	Maths::Vec3_SSE right() const;
	Maths::Matrix4x4_SSE viewMatrix() const;
	const Maths::Vec3_SSE& position() { return _position; }

	void update(float dt);
	void rotate(float dx, float dy);

};