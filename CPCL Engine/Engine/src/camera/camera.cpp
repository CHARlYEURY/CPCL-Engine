#include "camera/camera.h"

Cam::Cam(const Maths::Vec3_SSE& position) : _position(position) {}


Maths::Vec3_SSE Cam::forward() const {
	return Maths::Vec3_SSE(std::cos(_pitch) * std::sin(_yaw),
		std::sin(_pitch),
		std::cos(_pitch) * std::cos(_yaw));
}

Maths::Vec3_SSE Cam::right() const {
	return forward().Cross(Maths::Vec3_SSE(0.0f, 1.0f, 0.0f).Normalize());
}

Maths::Matrix4x4_SSE Cam::viewMatrix() const {
	return Maths::Matrix4x4_SSE::LookAt(_position, _position + forward(), _up);
}