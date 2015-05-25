#ifndef BITMAP_H
#define BITMAP_H

// for 32-bit 
// 64-bit should be 6
#define BITMAP_SHIFT    5
#define BITMAP_MASK     ((1UL << BITMAP_SHIFT) - 1)

// bits to words
#define BITMAP_B2W(nbits) \
    ((nbits+BITMAP_MASK) >> BITMAP_SHIFT)

// word in map
#define BITMAP_WIM(bitmap, index) \
    bitmap[(index) >> BITMAP_SHIFT]
// bit in word
#define BITMAP_BIW(index) \
    (1UL << ((index) & BITMAP_MASK))

// set bit
#define BITMAP_SET(bitmap, index) \
    BITMAP_WIM(bitmap, index) |= BITMAP_BIW(index)

// clear bit
#define BITMAP_CLEAR(bitmap, index) \
    BITMAP_WIM(bitmap, index) &= ~BITMAP_BIW(index)

// test bit
#define BITMAP_TEST(bitmap, index) \
    ((BITMAP_WIM(bitmap, index) & BITMAP_BIW(index)) ? 1 : 0)

#endif
