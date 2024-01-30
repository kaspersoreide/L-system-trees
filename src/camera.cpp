#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "glutils.h"
#include "matrix.h"

Camera::Camera() {
	vel = spin = vec3(0.0);
	pos = vec3(0.0, 0.0, -5.0);
	Rotation = mat3(1.0f);
	Model = mat4(1.0f);
	float fov = PI * 80.0f / 180;
	Projection = perspective(fov, 16.0f / 9, 0.01f, 100.0f);
}

void Camera::move() {
	const float rotAmount = 0.0007f;
	const float movAmount = 0.0005f;
	vec3 right = Rotation * vec4(1.0, 0.0, 0.0, 0.0);
	vec3 up = Rotation * vec4(0.0, 1.0, 0.0, 0.0);
	vec3 forward = Rotation * vec4(0.0, 0.0, 1.0, 0.0);
	vec3 dSpin(0.0f);
	if (rot[0]) dSpin += rotAmount * forward;
	if (rot[1]) dSpin -= rotAmount * forward;
	if (rot[2]) dSpin -= rotAmount * up;
	if (rot[3]) dSpin += rotAmount * up;
	if (rot[4]) dSpin -= rotAmount * right;
	if (rot[5]) dSpin += rotAmount * right;

	if (mov[0]) vel -= movAmount * forward;
	if (mov[1]) vel += movAmount * forward;
	if (mov[2]) vel -= movAmount * right;
	if (mov[3]) vel += movAmount * right;
	if (mov[4]) vel += movAmount * up;
	if (mov[5]) vel -= movAmount * up;

	vel *= 0.99f;
	spin *= 0.99f;
	spin += dSpin;
	Rotation = srotate(spin) * Rotation;

	pos += vel;
	Model = translateR(Rotation, pos);
}

mat4 Camera::getView() {
	return inverse(Model);
}

mat4 Camera::getVP() {
	return Projection * getView();
}