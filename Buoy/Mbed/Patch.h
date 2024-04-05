#ifndef PATCH_H
#define PATCH_H

#define SI446X_PATCH_CMDS                   \
0x08,0x04,0x21,0x71,0x4B,0x00,0x00,0xBA,0x9E, \
0x08,0x05,0x48,0x23,0x2E,0x2B,0x90,0xB1,0x4E, \
0x08,0xEA,0x3F,0xB9,0xE8,0x8B,0xA9,0xCA,0xD6, \
0x08,0x05,0xD2,0xE5,0xBE,0xD1,0x27,0x55,0x82, \
0x08,0xE5,0x56,0x2A,0x3B,0x76,0x76,0x96,0x48, \
0x08,0x05,0x8E,0x26,0xD8,0x5D,0x01,0xA7,0x88, \
0x08,0xE2,0x89,0xCC,0x63,0x79,0x95,0x00,0x4B, \
0x08,0x05,0xE0,0x75,0xCD,0xA4,0xB9,0x46,0xBC, \
0x08,0xEA,0xD3,0x37,0xD2,0x9A,0x89,0x82,0xEA, \
0x08,0x05,0x0C,0xAE,0x4C,0xF5,0xF6,0x3C,0xB3, \
0x08,0xE9,0xA7,0x70,0xDF,0xF1,0x14,0x4F,0x04, \
0x08,0x05,0xFE,0x5B,0xDF,0x47,0x0A,0x7C,0x5B, \
0x08,0xE2,0xFB,0x3E,0x21,0xA2,0x1B,0xAA,0x93, \
0x08,0x05,0xBF,0xFD,0xAB,0x69,0x6C,0xA8,0x5A, \
0x08,0xE2,0x66,0xB7,0x2E,0x2C,0x45,0x2D,0xFB, \
0x08,0x05,0x0D,0x55,0xBD,0xC2,0x37,0x00,0x72, \
0x08,0xE2,0xFF,0x57,0x4D,0x7C,0x6C,0x00,0x2C, \
0x08,0x05,0x9E,0xF2,0x46,0xFD,0xD3,0x16,0x1B, \
0x08,0xEA,0x16,0x7F,0x67,0x4D,0xE5,0xE2,0xC8, \
0x08,0x05,0x37,0x33,0x1C,0xFA,0xBB,0xEE,0xEF, \
0x08,0xEA,0x00,0x5F,0xBE,0xA4,0xFC,0xBF,0xC1, \
0x08,0x05,0x95,0x12,0x2F,0x0A,0xCF,0x55,0x8C, \
0x08,0xE7,0x70,0xC2,0xD4,0xF0,0x81,0x95,0xC8, \
0x08,0xE7,0x72,0x00,0xF9,0x8D,0x15,0x01,0xA3, \
0x08,0xE7,0x18,0xE5,0x6C,0x51,0x1F,0x86,0x9F, \
0x08,0xE7,0xDD,0x37,0x59,0x4B,0xAD,0xB0,0x9C, \
0x08,0xE7,0xC8,0xE8,0x84,0xCD,0x55,0x41,0x83, \
0x08,0xEF,0x4F,0x8E,0x38,0xCB,0x37,0x02,0x87, \
0x08,0xE7,0xF5,0x00,0x88,0x4C,0x09,0x65,0xCE, \
0x08,0xEF,0xDD,0xBC,0x65,0x62,0xAC,0x75,0x62, \
0x08,0xE7,0xC0,0xF1,0x5D,0x98,0xB0,0xDD,0x43, \
0x08,0xE7,0x19,0xB4,0xF8,0x9F,0x6D,0x8C,0xCB, \
0x08,0xE1,0xDE,0x63,0xC2,0x32,0xC6,0xE4,0x2F, \
0x08,0x05,0xF4,0x33,0xB7,0x2E,0x72,0x9A,0xF9, \
0x08,0xE7,0x65,0xD9,0x38,0xB8,0xFE,0x31,0x16, \
0x08,0xE7,0xF3,0x06,0x2D,0xF5,0xFE,0x0C,0x38, \
0x08,0xE7,0x70,0x4F,0xE7,0x49,0xB4,0x58,0x39, \
0x08,0xEF,0xF1,0x46,0xA9,0x23,0x38,0x64,0xC0, \
0x08,0xE7,0x09,0x4E,0x04,0xD3,0x46,0x15,0x02, \
0x08,0xEF,0x8D,0xC7,0x20,0xC3,0x90,0x87,0x4D, \
0x08,0xEF,0x00,0xAB,0x7F,0x27,0x02,0xC6,0xA0, \
0x08,0xE7,0x23,0xA6,0xA6,0xA4,0x27,0x11,0x7D, \
0x08,0xEF,0xB3,0xF1,0x9E,0x6A,0xB3,0x19,0xAF, \
0x08,0xE7,0xAB,0xF5,0x15,0x78,0x5E,0x48,0xF8, \
0x08,0xEF,0x5B,0xB1,0x2E,0xAF,0x2A,0xFF,0x16, \
0x08,0xE7,0x30,0x62,0x5C,0x82,0x7A,0x3F,0x83, \
0x08,0xEF,0x91,0xA7,0xD3,0x1B,0x64,0x85,0xBE, \
0x08,0xE7,0x4D,0x81,0x94,0xE4,0xAA,0xE8,0xDB, \
0x08,0xEF,0xA0,0xCC,0x4A,0x23,0xA5,0x7E,0x36, \
0x08,0xEF,0x0C,0x72,0x4C,0xFB,0x26,0x5A,0xEC, \
0x08,0xEF,0x0E,0x42,0xFA,0xAF,0x49,0xA0,0xA8, \
0x08,0xE7,0x6D,0x12,0xDF,0x2B,0x0C,0x61,0x58, \
0x08,0xEA,0xB6,0x9B,0xDE,0x81,0xB9,0xFF,0xFF, \
0x08,0x05,0x04,0xEB,0xD8,0x12,0xD6,0x8D,0xE0, \
0x08,0xEC,0x29,0x66,0x4B,0xDE,0xB7,0xDE,0x36, \
0x08,0x05,0x0D,0x28,0xB9,0x0A,0x89,0x31,0x1A

#endif
