#pragma once

#define MELODY_SEMITONE (1.05946f)	// 半音
#define MELODY_OCTAVE (2.0f)		// オクターブ

#define MELODY_C (261.626f)	// ド (C4)
#define MELODY_D (293.665f)	// レ (D4)
#define MELODY_E (329.628f)	// ミ (E4)
#define MELODY_F (349.228f)	// ファ (F4)
#define MELODY_G (391.995f)	// ソ (G4)
#define MELODY_A (440.000f)	// ラ (A4)
#define MELODY_B (493.883f)	// シ (B4)

extern float melody_freq;
extern float melody_volume;

void melodyControlTask();
