#include "camera/camera.h"

Cam::Cam(const Maths::Vec3_SSE& position) : _position(position) {}


Maths::Vec3_SSE Cam::forward() const {
	return Maths::Vec3_SSE(std::cos(_pitch) * std::sin(_yaw),
		std::sin(_pitch),
		std::cos(_pitch) * std::cos(_yaw));
}

Maths::Vec3_SSE Cam::right() const {
	return forward().Cross(_up.Normalize());
}

Maths::Matrix4x4_SSE Cam::viewMatrix() const {
	return Maths::Matrix4x4_SSE::LookAt(_position, _position + forward(), _up);
}

void Cam::update(float dt) {
	const bool* keys = SDL_GetKeyboardState(nullptr);
	Maths::Vec3_SSE move(0, 0, 0);
	
	if (keys[SDL_SCANCODE_W]) move += forward();
	if (keys[SDL_SCANCODE_S]) move -= forward();
	if (keys[SDL_SCANCODE_A]) move -= right();
	if (keys[SDL_SCANCODE_D]) move += right();

	if (keys[SDL_SCANCODE_SPACE]) move += _up;
	if (keys[SDL_SCANCODE_LSHIFT]) move -= _up;

	if (move.MagnitudeSquared() > 0.0f) {		// Normalize() lance une exception sur le vecteur nul
		move = move.Normalize() * _speed * dt;
		_position += move;
	}
}

void Cam::rotate(float dx, float dy) {
	_yaw -= dx * _sensitivity;
	_pitch -= dy * _sensitivity;
	
	if (_pitch > 1.5f) _pitch = 1.5f;
	if (_pitch < -1.5f) _pitch = -1.5f;
}