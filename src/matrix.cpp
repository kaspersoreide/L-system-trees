#include "matrix.h"
#include <math.h>

//creates a 3x3 rotation matrix from input vec3, angle is length of vector
//based on http://www.cs.cornell.edu/courses/cs4620/2012fa/lectures/05transformRotations.pdf
//and also https://www.cs.brandeis.edu/~cs155/Lecture_06.pdf
mat3 srotate(vec3 rot) {
	//first we make an orthonormal frame,
	//3 perpendicular unit vectors u, v, and w
	//u is parallell to rotation axis
	float a = length(rot);
	if (a == 0.0f) return mat3(1.0f);
	vec3 u = rot / a;
	vec3 v = cross(u, vec3(u[1], u[2], u[0]));
	vec3 w = cross(v, u);

	mat3 F = transpose(mat3(u, v, w));
	//
	mat3 R;
	if (a < 0.005) {
		//simplification for small angle, sin(x) = x and cos ~ 1
		R = mat3(
			vec3(1.0, 0.0, 0.0),
			vec3(0.0, 1.0, a),
			vec3(0.0, -a, 1.0)
		);
	}
	else {
		float cos = std::cosf(a);
		float sin = std::sinf(a);
		R = mat3(
			vec3(1.0, 0.0, 0.0),
			vec3(0.0, cos, sin),
			vec3(0.0, -sin, cos)
		);
	}

	//F^T sends u to x-axis
	return inverse(F) * R * F;
}

mat4 translateR(mat3 R, vec3 p) {
	mat4 result(0.0f);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			result[i][j] = R[i][j];
		}
		result[3][i] = p[i];
	}
	result[3][3] = 1.0;
	return result;
}