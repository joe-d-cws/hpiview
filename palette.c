#include <windows.h>

PALETTEENTRY TAPalette[256] = {
{0x00, 0x00, 0x00, 0x00},
{0x80, 0x00, 0x00, 0x00},
{0x00, 0x80, 0x00, 0x00},
{0x80, 0x80, 0x00, 0x00},
{0x00, 0x00, 0x80, 0x00},
{0x80, 0x00, 0x80, 0x00},
{0x00, 0x80, 0x80, 0x00},
{0x80, 0x80, 0x80, 0x00},
{0xC0, 0xDC, 0xC0, 0x00},
{0x54, 0x54, 0xFC, 0x00},
{0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x00},
{0xFF, 0xEB, 0xF3, 0x00},
{0xEB, 0xC7, 0xD3, 0x00},
{0xD7, 0xA3, 0xB3, 0x00},
{0xC3, 0x87, 0x97, 0x00},
{0xAF, 0x6F, 0x7F, 0x00},
{0x9B, 0x5B, 0x63, 0x00},
{0x8B, 0x47, 0x4F, 0x00},
{0x7B, 0x3B, 0x47, 0x00},
{0x6F, 0x33, 0x3B, 0x00},
{0x63, 0x2B, 0x33, 0x00},
{0x57, 0x23, 0x2B, 0x00},
{0x4B, 0x1B, 0x27, 0x00},
{0x3B, 0x17, 0x1F, 0x00},
{0x2F, 0x0F, 0x17, 0x00},
{0x23, 0x0B, 0x0F, 0x00},
{0x17, 0x07, 0x0B, 0x00},
{0x73, 0xFF, 0xDF, 0x00},
{0x57, 0xE7, 0xBF, 0x00},
{0x43, 0xCF, 0x9F, 0x00},
{0x2F, 0xB7, 0x83, 0x00},
{0x1F, 0x9F, 0x67, 0x00},
{0x13, 0x8B, 0x4F, 0x00},
{0x0F, 0x77, 0x3F, 0x00},
{0x0B, 0x6B, 0x37, 0x00},
{0x07, 0x5F, 0x2F, 0x00},
{0x07, 0x53, 0x2B, 0x00},
{0x00, 0x47, 0x27, 0x00},
{0x00, 0x3F, 0x23, 0x00},
{0x00, 0x33, 0x1B, 0x00},
{0x00, 0x27, 0x17, 0x00},
{0x00, 0x1B, 0x0F, 0x00},
{0x00, 0x13, 0x0B, 0x00},
{0xE3, 0xEF, 0xFF, 0x00},
{0xC7, 0xDF, 0xE7, 0x00},
{0xAF, 0xCF, 0xCB, 0x00},
{0x93, 0xB7, 0xA7, 0x00},
{0x7F, 0x9F, 0x83, 0x00},
{0x6B, 0x87, 0x67, 0x00},
{0x5F, 0x6F, 0x53, 0x00},
{0x5F, 0x63, 0x47, 0x00},
{0x5B, 0x57, 0x3B, 0x00},
{0x53, 0x43, 0x33, 0x00},
{0x47, 0x3B, 0x2B, 0x00},
{0x3B, 0x33, 0x23, 0x00},
{0x2F, 0x2B, 0x1B, 0x00},
{0x23, 0x1F, 0x13, 0x00},
{0x17, 0x13, 0x0F, 0x00},
{0x0B, 0x0B, 0x07, 0x00},
{0xFB, 0xFB, 0xD7, 0x00},
{0xDF, 0xDF, 0xB7, 0x00},
{0xC3, 0xC3, 0x9B, 0x00},
{0xAB, 0xAB, 0x83, 0x00},
{0x93, 0x93, 0x6F, 0x00},
{0x77, 0x77, 0x57, 0x00},
{0x63, 0x63, 0x43, 0x00},
{0x53, 0x53, 0x33, 0x00},
{0x43, 0x43, 0x23, 0x00},
{0x33, 0x33, 0x17, 0x00},
{0x23, 0x23, 0x0F, 0x00},
{0x1B, 0x1B, 0x07, 0x00},
{0x17, 0x17, 0x07, 0x00},
{0x13, 0x13, 0x00, 0x00},
{0x0F, 0x0F, 0x00, 0x00},
{0x0B, 0x0B, 0x00, 0x00},
{0xFB, 0xFB, 0xFB, 0x00},
{0xEB, 0xEB, 0xEB, 0x00},
{0xDB, 0xDB, 0xDB, 0x00},
{0xCB, 0xCB, 0xCB, 0x00},
{0xBB, 0xBB, 0xBB, 0x00},
{0xAB, 0xAB, 0xAB, 0x00},
{0x9B, 0x9B, 0x9B, 0x00},
{0x8B, 0x8B, 0x8B, 0x00},
{0x7B, 0x7B, 0x7B, 0x00},
{0x6B, 0x6B, 0x6B, 0x00},
{0x5B, 0x5B, 0x5B, 0x00},
{0x4B, 0x4B, 0x4B, 0x00},
{0x3B, 0x3B, 0x3B, 0x00},
{0x2B, 0x2B, 0x2B, 0x00},
{0x1F, 0x1F, 0x1F, 0x00},
{0x0F, 0x0F, 0x0F, 0x00},
{0xEB, 0xF3, 0xFF, 0x00},
{0xCB, 0xE3, 0xFF, 0x00},
{0xAF, 0xCF, 0xFF, 0x00},
{0x97, 0xB3, 0xFF, 0x00},
{0x7B, 0x97, 0xFF, 0x00},
{0x67, 0x7F, 0xFF, 0x00},
{0x53, 0x6B, 0xEF, 0x00},
{0x3F, 0x5B, 0xE3, 0x00},
{0x33, 0x4B, 0xD7, 0x00},
{0x23, 0x3B, 0xCB, 0x00},
{0x17, 0x2F, 0xAF, 0x00},
{0x0F, 0x27, 0x97, 0x00},
{0x07, 0x1F, 0x7B, 0x00},
{0x07, 0x17, 0x63, 0x00},
{0x00, 0x0F, 0x47, 0x00},
{0x00, 0x0B, 0x2F, 0x00},
{0xE3, 0xF7, 0xFF, 0x00},
{0xBF, 0xDB, 0xE7, 0x00},
{0x9F, 0xBF, 0xCF, 0x00},
{0x83, 0xA7, 0xB7, 0x00},
{0x6B, 0x8F, 0xA3, 0x00},
{0x53, 0x77, 0x8B, 0x00},
{0x3F, 0x5F, 0x73, 0x00},
{0x2F, 0x4B, 0x5F, 0x00},
{0x27, 0x3F, 0x57, 0x00},
{0x23, 0x37, 0x4F, 0x00},
{0x1F, 0x2F, 0x47, 0x00},
{0x1B, 0x27, 0x3F, 0x00},
{0x17, 0x1F, 0x37, 0x00},
{0x13, 0x1B, 0x2F, 0x00},
{0x0F, 0x13, 0x27, 0x00},
{0x0B, 0x0F, 0x1F, 0x00},
{0xD7, 0xEF, 0xFF, 0x00},
{0xBB, 0xE3, 0xEF, 0x00},
{0x9B, 0xCB, 0xDF, 0x00},
{0x83, 0xB7, 0xCF, 0x00},
{0x6B, 0xA3, 0xC3, 0x00},
{0x53, 0x8F, 0xB3, 0x00},
{0x3F, 0x7B, 0xA3, 0x00},
{0x2F, 0x6B, 0x97, 0x00},
{0x23, 0x5B, 0x87, 0x00},
{0x1B, 0x4B, 0x77, 0x00},
{0x13, 0x3F, 0x67, 0x00},
{0x0B, 0x33, 0x57, 0x00},
{0x07, 0x27, 0x47, 0x00},
{0x00, 0x1B, 0x37, 0x00},
{0x00, 0x13, 0x27, 0x00},
{0x00, 0x0B, 0x1B, 0x00},
{0xFF, 0xE7, 0xFF, 0x00},
{0xE7, 0xC7, 0xEB, 0x00},
{0xD3, 0xAB, 0xD7, 0x00},
{0xBB, 0x93, 0xC3, 0x00},
{0xA7, 0x7B, 0xB3, 0x00},
{0x8F, 0x63, 0x9F, 0x00},
{0x77, 0x4B, 0x8F, 0x00},
{0x63, 0x3B, 0x7F, 0x00},
{0x4F, 0x2B, 0x6F, 0x00},
{0x43, 0x1F, 0x63, 0x00},
{0x37, 0x17, 0x57, 0x00},
{0x2B, 0x0F, 0x47, 0x00},
{0x1F, 0x07, 0x3B, 0x00},
{0x13, 0x00, 0x2B, 0x00},
{0x0B, 0x00, 0x1F, 0x00},
{0x07, 0x00, 0x13, 0x00},
{0xD7, 0xFF, 0xA7, 0x00},
{0xAB, 0xE7, 0x7F, 0x00},
{0x83, 0xD3, 0x5B, 0x00},
{0x67, 0xBF, 0x3F, 0x00},
{0x4B, 0xAB, 0x2B, 0x00},
{0x43, 0x97, 0x2B, 0x00},
{0x37, 0x87, 0x27, 0x00},
{0x2F, 0x77, 0x1B, 0x00},
{0x2B, 0x67, 0x13, 0x00},
{0x23, 0x5B, 0x0F, 0x00},
{0x1F, 0x4F, 0x0B, 0x00},
{0x1B, 0x43, 0x07, 0x00},
{0x17, 0x33, 0x00, 0x00},
{0x0F, 0x27, 0x00, 0x00},
{0x0B, 0x1B, 0x00, 0x00},
{0x07, 0x0F, 0x00, 0x00},
{0xFF, 0xE3, 0x9F, 0x00},
{0xE3, 0xC7, 0x73, 0x00},
{0xCB, 0xAF, 0x53, 0x00},
{0xB3, 0x97, 0x3F, 0x00},
{0x9B, 0x83, 0x2F, 0x00},
{0x83, 0x6F, 0x23, 0x00},
{0x6B, 0x5B, 0x17, 0x00},
{0x53, 0x47, 0x0F, 0x00},
{0x4B, 0x3B, 0x0B, 0x00},
{0x43, 0x33, 0x07, 0x00},
{0x3B, 0x2B, 0x07, 0x00},
{0x37, 0x23, 0x00, 0x00},
{0x2F, 0x1B, 0x00, 0x00},
{0x27, 0x13, 0x00, 0x00},
{0x1F, 0x0F, 0x00, 0x00},
{0x1B, 0x0B, 0x00, 0x00},
{0xFF, 0xFF, 0xA3, 0x00},
{0xFB, 0xF3, 0x83, 0x00},
{0xF7, 0xE3, 0x67, 0x00},
{0xF3, 0xD3, 0x4F, 0x00},
{0xEF, 0xBB, 0x33, 0x00},
{0xEF, 0xA7, 0x1B, 0x00},
{0xEB, 0x8F, 0x13, 0x00},
{0xE7, 0x7B, 0x0F, 0x00},
{0xDF, 0x4F, 0x07, 0x00},
{0xD7, 0x23, 0x00, 0x00},
{0xBF, 0x1F, 0x00, 0x00},
{0xA7, 0x1B, 0x00, 0x00},
{0x93, 0x17, 0x00, 0x00},
{0x7B, 0x13, 0x00, 0x00},
{0x63, 0x13, 0x00, 0x00},
{0x4F, 0x0F, 0x00, 0x00},
{0xFF, 0xFF, 0x00, 0x00},
{0xFF, 0xBF, 0x00, 0x00},
{0xFF, 0x83, 0x00, 0x00},
{0xFF, 0x47, 0x00, 0x00},
{0xD3, 0x2B, 0x00, 0x00},
{0xAB, 0x17, 0x00, 0x00},
{0x7F, 0x07, 0x00, 0x00},
{0x57, 0x00, 0x00, 0x00},
{0xDF, 0xCB, 0xFF, 0x00},
{0xBB, 0x9F, 0xDF, 0x00},
{0x9B, 0x77, 0xBF, 0x00},
{0x7F, 0x57, 0x9F, 0x00},
{0x67, 0x3B, 0x7F, 0x00},
{0x4B, 0x23, 0x5F, 0x00},
{0x33, 0x13, 0x3F, 0x00},
{0x1B, 0x07, 0x1F, 0x00},
{0xD3, 0xDB, 0xFF, 0x00},
{0x87, 0x9F, 0xF7, 0x00},
{0x43, 0x6F, 0xEF, 0x00},
{0x17, 0x47, 0xE7, 0x00},
{0x0B, 0x2B, 0xBB, 0x00},
{0x07, 0x17, 0x8F, 0x00},
{0x00, 0x07, 0x63, 0x00},
{0x00, 0x00, 0x37, 0x00},
{0x7B, 0xFF, 0x77, 0x00},
{0x53, 0xDF, 0x4F, 0x00},
{0x33, 0xBF, 0x2B, 0x00},
{0x1B, 0x9F, 0x13, 0x00},
{0x1B, 0x7F, 0x0B, 0x00},
{0x17, 0x5F, 0x07, 0x00},
{0x13, 0x3F, 0x00, 0x00},
{0x0B, 0x1F, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x00},
{0xFF, 0xFB, 0xF0, 0x00},
{0xA0, 0xA0, 0xA4, 0x00},
{0x80, 0x80, 0x80, 0x00},
{0xFF, 0x00, 0x00, 0x00},
{0x00, 0xFF, 0x00, 0x00},
{0xFF, 0xFF, 0x00, 0x00},
{0x00, 0x00, 0xFF, 0x00},
{0xFF, 0x00, 0xFF, 0x00},
{0x00, 0xFF, 0xFF, 0x00},
{0xFF, 0xFF, 0xFF, 0x00}};

PALETTEENTRY TAKPalette[256] = {
{0x02, 0x02, 0x02, 0x00},
{0x06, 0x06, 0x06, 0x00},
{0x0C, 0x0C, 0x0C, 0x00},
{0x12, 0x12, 0x12, 0x00},
{0x16, 0x16, 0x16, 0x00},
{0x1C, 0x1C, 0x1C, 0x00},
{0x66, 0x66, 0x66, 0x00},
{0x06, 0x02, 0x02, 0x00},
{0x31, 0x06, 0x05, 0x00},
{0xFF, 0x00, 0xFF, 0x00},
{0x90, 0x0F, 0x0C, 0x00},
{0x45, 0x06, 0x05, 0x00},
{0x5F, 0x08, 0x07, 0x00},
{0x20, 0x04, 0x03, 0x00},
{0x71, 0x13, 0x0D, 0x00},
{0xA2, 0x18, 0x0F, 0x00},
{0x0B, 0x03, 0x02, 0x00},
{0xB9, 0x26, 0x15, 0x00},
{0x9F, 0x36, 0x25, 0x00},
{0x26, 0x1A, 0x18, 0x00},
{0x3E, 0x15, 0x0E, 0x00},
{0x56, 0x1C, 0x11, 0x00},
{0xCC, 0x4E, 0x33, 0x00},
{0xDC, 0x77, 0x59, 0x00},
{0x24, 0x12, 0x0D, 0x00},
{0xFD, 0x97, 0x69, 0x00},
{0x37, 0x15, 0x06, 0x00},
{0x1A, 0x14, 0x11, 0x00},
{0xFB, 0xB9, 0x90, 0x00},
{0x34, 0x2A, 0x22, 0x00},
{0x2A, 0x15, 0x03, 0x00},
{0x32, 0x22, 0x13, 0x00},
{0x13, 0x0A, 0x02, 0x00},
{0x3E, 0x20, 0x03, 0x00},
{0x2A, 0x1A, 0x0A, 0x00},
{0x4B, 0x29, 0x05, 0x00},
{0x5E, 0x3C, 0x16, 0x00},
{0x6D, 0x45, 0x17, 0x00},
{0x3C, 0x26, 0x0B, 0x00},
{0x21, 0x13, 0x02, 0x00},
{0x4A, 0x3A, 0x22, 0x00},
{0x42, 0x32, 0x1A, 0x00},
{0x32, 0x22, 0x0A, 0x00},
{0x52, 0x43, 0x2A, 0x00},
{0x1B, 0x14, 0x09, 0x00},
{0x2E, 0x22, 0x0F, 0x00},
{0x15, 0x0F, 0x05, 0x00},
{0x2A, 0x1B, 0x02, 0x00},
{0x33, 0x2A, 0x1A, 0x00},
{0x3A, 0x32, 0x22, 0x00},
{0x52, 0x42, 0x22, 0x00},
{0x3E, 0x31, 0x16, 0x00},
{0x4A, 0x3A, 0x1A, 0x00},
{0x22, 0x1A, 0x0A, 0x00},
{0x42, 0x32, 0x12, 0x00},
{0x2E, 0x22, 0x0A, 0x00},
{0x3A, 0x2A, 0x0A, 0x00},
{0x1A, 0x12, 0x02, 0x00},
{0x32, 0x22, 0x02, 0x00},
{0x4A, 0x44, 0x36, 0x00},
{0x5A, 0x4B, 0x2A, 0x00},
{0x35, 0x2A, 0x12, 0x00},
{0x46, 0x36, 0x13, 0x00},
{0x36, 0x2A, 0x0E, 0x00},
{0x2E, 0x22, 0x06, 0x00},
{0x54, 0x4A, 0x32, 0x00},
{0x42, 0x39, 0x23, 0x00},
{0x28, 0x22, 0x13, 0x00},
{0x27, 0x20, 0x0F, 0x00},
{0x46, 0x3A, 0x1B, 0x00},
{0x52, 0x42, 0x1A, 0x00},
{0x4A, 0x3A, 0x12, 0x00},
{0x42, 0x32, 0x0A, 0x00},
{0x2E, 0x21, 0x02, 0x00},
{0x52, 0x4F, 0x47, 0x00},
{0x5C, 0x52, 0x34, 0x00},
{0x42, 0x39, 0x1E, 0x00},
{0x3A, 0x32, 0x1A, 0x00},
{0x4E, 0x42, 0x20, 0x00},
{0x5A, 0x4B, 0x22, 0x00},
{0x46, 0x3A, 0x16, 0x00},
{0x2F, 0x26, 0x0E, 0x00},
{0x3E, 0x32, 0x12, 0x00},
{0x5A, 0x4A, 0x1A, 0x00},
{0x2A, 0x22, 0x0A, 0x00},
{0x3E, 0x32, 0x0E, 0x00},
{0x52, 0x42, 0x12, 0x00},
{0x22, 0x1A, 0x02, 0x00},
{0x3A, 0x2B, 0x03, 0x00},
{0x82, 0x78, 0x57, 0x00},
{0x6B, 0x62, 0x44, 0x00},
{0x65, 0x5A, 0x35, 0x00},
{0x3E, 0x36, 0x1B, 0x00},
{0x38, 0x30, 0x16, 0x00},
{0x4E, 0x40, 0x12, 0x00},
{0x26, 0x1E, 0x05, 0x00},
{0x44, 0x35, 0x03, 0x00},
{0x91, 0x88, 0x65, 0x00},
{0x83, 0x76, 0x49, 0x00},
{0x4A, 0x42, 0x24, 0x00},
{0x50, 0x47, 0x27, 0x00},
{0x53, 0x48, 0x1E, 0x00},
{0x3F, 0x36, 0x16, 0x00},
{0x52, 0x46, 0x1A, 0x00},
{0x4E, 0x42, 0x16, 0x00},
{0x37, 0x2E, 0x0F, 0x00},
{0x2E, 0x26, 0x0A, 0x00},
{0x75, 0x6C, 0x47, 0x00},
{0x52, 0x4A, 0x2A, 0x00},
{0x14, 0x12, 0x0A, 0x00},
{0x6E, 0x64, 0x37, 0x00},
{0x46, 0x3E, 0x1E, 0x00},
{0x49, 0x41, 0x1E, 0x00},
{0x42, 0x3A, 0x1A, 0x00},
{0x63, 0x58, 0x27, 0x00},
{0x46, 0x3E, 0x1A, 0x00},
{0x3A, 0x32, 0x12, 0x00},
{0x48, 0x3E, 0x16, 0x00},
{0x3E, 0x36, 0x12, 0x00},
{0x31, 0x2A, 0x0E, 0x00},
{0x3F, 0x36, 0x0E, 0x00},
{0x32, 0x2A, 0x0A, 0x00},
{0x7A, 0x68, 0x18, 0x00},
{0x5E, 0x4F, 0x0D, 0x00},
{0x2F, 0x27, 0x06, 0x00},
{0x4B, 0x3D, 0x05, 0x00},
{0x2A, 0x22, 0x02, 0x00},
{0x2E, 0x26, 0x02, 0x00},
{0x72, 0x5E, 0x03, 0x00},
{0xA2, 0x98, 0x64, 0x00},
{0x4E, 0x46, 0x1F, 0x00},
{0x4A, 0x42, 0x1A, 0x00},
{0x42, 0x3A, 0x12, 0x00},
{0x52, 0x48, 0x12, 0x00},
{0x69, 0x5A, 0x15, 0x00},
{0x3A, 0x32, 0x0A, 0x00},
{0x36, 0x2E, 0x09, 0x00},
{0x32, 0x2A, 0x03, 0x00},
{0xDC, 0xD9, 0xC8, 0x00},
{0xAB, 0xA5, 0x7F, 0x00},
{0xAC, 0xA1, 0x62, 0x00},
{0x99, 0x90, 0x56, 0x00},
{0x52, 0x4B, 0x22, 0x00},
{0xC2, 0xB1, 0x4A, 0x00},
{0x4E, 0x46, 0x1A, 0x00},
{0x52, 0x4A, 0x1A, 0x00},
{0x4E, 0x46, 0x16, 0x00},
{0x9D, 0x8C, 0x2C, 0x00},
{0x46, 0x3E, 0x12, 0x00},
{0x4A, 0x42, 0x12, 0x00},
{0x42, 0x3A, 0x0B, 0x00},
{0x3E, 0x37, 0x0A, 0x00},
{0x52, 0x48, 0x0D, 0x00},
{0x3A, 0x32, 0x03, 0x00},
{0x1B, 0x1A, 0x13, 0x00},
{0xA1, 0x98, 0x5A, 0x00},
{0x7B, 0x73, 0x3C, 0x00},
{0x5A, 0x53, 0x24, 0x00},
{0x6C, 0x64, 0x2B, 0x00},
{0x1C, 0x1A, 0x0A, 0x00},
{0x32, 0x2E, 0x0F, 0x00},
{0x2A, 0x26, 0x0A, 0x00},
{0x2A, 0x26, 0x06, 0x00},
{0xF3, 0xF1, 0xE0, 0x00},
{0x2B, 0x2A, 0x21, 0x00},
{0x3A, 0x37, 0x19, 0x00},
{0x42, 0x3E, 0x1A, 0x00},
{0x42, 0x3E, 0x16, 0x00},
{0x3A, 0x36, 0x12, 0x00},
{0x58, 0x52, 0x1A, 0x00},
{0x3A, 0x36, 0x0E, 0x00},
{0x32, 0x2E, 0x0A, 0x00},
{0x33, 0x2E, 0x02, 0x00},
{0xE6, 0xDE, 0x76, 0x00},
{0x4A, 0x47, 0x22, 0x00},
{0x4A, 0x46, 0x1A, 0x00},
{0x4A, 0x46, 0x16, 0x00},
{0xD7, 0xCC, 0x3B, 0x00},
{0x42, 0x3E, 0x12, 0x00},
{0x4A, 0x46, 0x12, 0x00},
{0x47, 0x42, 0x0A, 0x00},
{0xE4, 0xE2, 0xB6, 0x00},
{0xCD, 0xCC, 0xB0, 0x00},
{0xB9, 0xB8, 0x9E, 0x00},
{0xD6, 0xD5, 0xAD, 0x00},
{0xEA, 0xEA, 0xBE, 0x00},
{0x42, 0x42, 0x22, 0x00},
{0x2A, 0x2A, 0x14, 0x00},
{0x42, 0x42, 0x1A, 0x00},
{0x32, 0x32, 0x12, 0x00},
{0x4A, 0x4A, 0x1A, 0x00},
{0x06, 0x06, 0x02, 0x00},
{0x3A, 0x3A, 0x12, 0x00},
{0x22, 0x22, 0x0A, 0x00},
{0x42, 0x42, 0x12, 0x00},
{0x4A, 0x4A, 0x12, 0x00},
{0x2A, 0x2A, 0x0A, 0x00},
{0x0A, 0x0A, 0x02, 0x00},
{0x32, 0x32, 0x0A, 0x00},
{0x3A, 0x3A, 0x0A, 0x00},
{0x12, 0x12, 0x02, 0x00},
{0x1A, 0x1A, 0x02, 0x00},
{0x22, 0x22, 0x02, 0x00},
{0x2A, 0x2A, 0x02, 0x00},
{0x4E, 0x52, 0x13, 0x00},
{0x49, 0x51, 0x24, 0x00},
{0x36, 0x3E, 0x12, 0x00},
{0x2A, 0x32, 0x0A, 0x00},
{0x22, 0x2A, 0x02, 0x00},
{0xFB, 0xFC, 0xFA, 0x00},
{0x20, 0x25, 0x1E, 0x00},
{0xB1, 0xB3, 0xB1, 0x00},
{0x0D, 0x12, 0x0D, 0x00},
{0x06, 0x0A, 0x06, 0x00},
{0x02, 0x06, 0x02, 0x00},
{0x14, 0x1B, 0x15, 0x00},
{0x06, 0x72, 0x2E, 0x00},
{0x0A, 0xC2, 0x4E, 0x00},
{0x0A, 0xE6, 0x5E, 0x00},
{0x06, 0x96, 0x3E, 0x00},
{0x96, 0x97, 0x97, 0x00},
{0x2D, 0x2E, 0x2E, 0x00},
{0x02, 0x06, 0x06, 0x00},
{0xCC, 0xCE, 0xCF, 0x00},
{0x5A, 0xC2, 0xFA, 0x00},
{0xB5, 0xBE, 0xC3, 0x00},
{0x06, 0x0A, 0x0D, 0x00},
{0x3B, 0x3C, 0x3D, 0x00},
{0x33, 0x34, 0x35, 0x00},
{0x80, 0x85, 0x8A, 0x00},
{0x0D, 0x10, 0x13, 0x00},
{0x37, 0x4B, 0x5F, 0x00},
{0x1E, 0x25, 0x2D, 0x00},
{0x8E, 0xA0, 0xB8, 0x00},
{0xDB, 0xE1, 0xEA, 0x00},
{0x5F, 0x6B, 0x7D, 0x00},
{0x52, 0x5C, 0x6F, 0x00},
{0x15, 0x17, 0x1B, 0x00},
{0x58, 0x59, 0x5C, 0x00},
{0x2A, 0x5A, 0xE4, 0x00},
{0x31, 0x39, 0x52, 0x00},
{0x2A, 0x30, 0x46, 0x00},
{0x42, 0x43, 0x49, 0x00},
{0x05, 0x07, 0x14, 0x00},
{0x4B, 0x4C, 0x53, 0x00},
{0xAF, 0xB5, 0xF2, 0x00},
{0x24, 0x25, 0x37, 0x00},
{0x19, 0x19, 0x24, 0x00},
{0x10, 0x10, 0x1C, 0x00},
{0x02, 0x02, 0x06, 0x00},
{0x02, 0x02, 0x0A, 0x00},
{0x22, 0x21, 0x23, 0x00},
{0x76, 0x75, 0x76, 0x00},
{0x29, 0x25, 0x29, 0x00},
{0x12, 0x0E, 0x12, 0x00},
{0x06, 0x02, 0x06, 0x00}};

COLORREF TAColor[256];

COLORREF TAKColor[256];
