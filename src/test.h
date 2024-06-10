#pragma once
#include "lsystem.h"
#include "turtle.h"

void testLsystem(bool parallel, int iterations, int measurements);

void testInterpreter(int iterations, int measurements, uint segmentsPerNode, uint verticesPerSegment);

void testLeafTexture(int resolution, int measurements, GLuint leafShader);