#ifndef AUDIO_MAP_H
#define AUDIO_MAP_H

#include <stdint.h>

const uint8_t LengthCounterMap[32] = {
    0x0A, 0xFE, 0x14, 0x02,
    0x28, 0x04, 0x50, 0x06,
    0xA0, 0x08, 0x3C, 0x0A,
    0x0E, 0x0C, 0x1A, 0x0E,
    0x0C, 0x10, 0x18, 0x12,
    0x30, 0x14, 0x60, 0x16,
    0xC0, 0x18, 0x48, 0x1A,
    0x10, 0x1C, 0x20, 0x1E,
};

const bool PulseWave[4][8] = {
    {0, 1, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 0, 0, 0},
    {1, 0, 0, 1, 1, 1, 1, 1}
};

const uint8_t TriangleWave[32] = {
    15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

const uint16_t NoisePeriodMap[16] = {4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068};
const uint16_t DPCMPeriodMap[16] = {428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54};

#define SAMPLE_PER_SEC 44100
#define CPU_CYCLE_PER_SEC 1789773
#define SAMPLE_PER_CLOCK (SAMPLE_PER_SEC / 240 + 1)

#endif
