#pragma once

#include "arm_math.h"

typedef struct {
	float u;
	float v;
	float w;
} FOC_UVW;

typedef struct {
	float a;
	float b;
} FOC_AB;

typedef struct {
	float d;
	float q;
} FOC_DQ;

void FOC_DQtoAB(const FOC_DQ *dq, float sin, float cos, FOC_AB *ab);
void FOC_ABtoUVW(const FOC_AB *ab, FOC_UVW *uvw);

FOC_AB FOC_UVWtoAB(float u, float v, float w);
FOC_DQ FOC_ABtoDQ(const FOC_AB *ab, float sin, float cos);