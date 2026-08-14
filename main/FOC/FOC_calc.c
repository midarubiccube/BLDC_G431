/*
 * FOC.c
 *
 *  Created on: Aug 21, 2025
 *      Author: Doraemonjayo
 */

#include "FOC_calc.h"

// 普通は以下のようになるはず
//static const float UVW_UNIT_VECTOR[3][2] = {
//		{1.0f, 0.0f},
//		{-0.5f, 0.86602540378f},
//		{-0.5f, -0.86602540378f}
//};

// 謎
static const float UVW_UNIT_VECTOR[3][2] = {
	{0.5f, 0.86602540378f},
	{-1.0f, 0.0f},
	{0.5f, -0.86602540378f}
};

void FOC_DQtoAB(const FOC_DQ *dq, float sin, float cos, FOC_AB *ab) {
	ab->a = dq->d * cos - dq->q * sin;
	ab->b = dq->d * sin + dq->q * cos;
}

void FOC_ABtoUVW(const FOC_AB *ab, FOC_UVW *uvw) {
	uvw->u = ab->a * UVW_UNIT_VECTOR[0][0] + ab->b * UVW_UNIT_VECTOR[0][1];
	uvw->v = ab->a * UVW_UNIT_VECTOR[1][0] + ab->b * UVW_UNIT_VECTOR[1][1];
	uvw->w = ab->a * UVW_UNIT_VECTOR[2][0] + ab->b * UVW_UNIT_VECTOR[2][1];
}

FOC_AB FOC_UVWtoAB(float u, float v, float w) {
	FOC_AB ab;
	ab.a = (u * UVW_UNIT_VECTOR[0][0] + v * UVW_UNIT_VECTOR[1][0] + w * UVW_UNIT_VECTOR[2][0]) / 1.5f;
	ab.b = (u * UVW_UNIT_VECTOR[0][1] + v * UVW_UNIT_VECTOR[1][1] + w * UVW_UNIT_VECTOR[2][1]) / 1.5f;
	return ab;
}

FOC_DQ FOC_ABtoDQ(const FOC_AB *ab, float sin, float cos) {
	FOC_DQ dq;
	dq.d = ab->a * cos + ab->b * sin;
	dq.q = -ab->a * sin + ab->b * cos;
	return dq;
}
