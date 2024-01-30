#pragma once
#include <glm/glm.hpp>

using namespace glm;

class Camera {
public:
	Camera();
	void move();
	mat4 getModel() { return Model; }
	mat4 getView();
	mat4 getVP();
	bool rot[6], mov[6];
private:
	mat4 Model, Projection;
	vec3 pos, vel, spin;
	mat3 Rotation;
};