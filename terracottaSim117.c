#include <stdio.h>
#include <stdint.h>

static inline void setSeed(uint64_t *seed, uint64_t value)
{
    *seed = (value ^ 0x5deece66d) & ((1ULL << 48) - 1);
}

static inline int next(uint64_t *seed, const int bits)
{
    *seed = (*seed * 0x5deece66d + 0xb) & ((1ULL << 48) - 1);
    return (int) ((int64_t)*seed >> (48 - bits));
}

/*
static inline void stepFoward(uint64_t *seed)
{
    *seed = (*seed * 0x5deece66d + 0xb) & ((1ULL << 48) - 1);
}
*/

static inline int nextInt(uint64_t *seed, const int n)
{
    int bits, val;
    const int m = n - 1;

    if ((m & n) == 0) {
        uint64_t x = n * (uint64_t)next(seed, 31);
        return (int) ((int64_t) x >> 31);
    }

    do {
        bits = next(seed, 31);
        val = bits % n;
    }
    while (bits - val + m < 0);
    return val;
}

int nextBoolean(uint64_t *seed)
{
    return next(seed, 1) != 0;
}

/*
void simplexNoiseStep(uint64_t *seed)
{
    *seed = (*seed * 0x5deece66d + 0xb) & ((1ULL << 48) - 1);
    *seed = (*seed * 0x5deece66d + 0xb) & ((1ULL << 48) - 1);
    *seed = (*seed * 0x5deece66d + 0xb) & ((1ULL << 48) - 1);
    *seed = (*seed * 0x5deece66d + 0xb) & ((1ULL << 48) - 1);
    *seed = (*seed * 0x5deece66d + 0xb) & ((1ULL << 48) - 1);
    *seed = (*seed * 0x5deece66d + 0xb) & ((1ULL << 48) - 1);
    for(int n = 0; n < 256; ++n)
    {
        nextInt(seed, 256 - n);
    }
}

void printArr(int buffer[64])
{
    for(int j = 0; j < 64; j++)
    {
        printf("%d", buffer[j]);
    }
    printf("\n");
}
*/

void generateBands(int clayBands[64], uint64_t seed)
{
    enum terracottaTypes
    {
        TERRACOTTA,
        ORANGE_TERRACOTTA,
        YELLOW_TERRACOTTA,
        BROWN_TERRACOTTA,
        RED_TERRACOTTA,
        LIGHT_GRAY_TERRACOTTA,
        WHITE_TERRACOTTA
    };
    //uint64_t seed = l;
    int rand1;
    int rand2;
    int rand3;

    //setSeed(&seed, l);
    //simplexNoiseStep(&seed);

    //fill array with terracotta

    for(int i = 0; i < 64; i++)
    {
        clayBands[i] = TERRACOTTA;
    }

    //add orange 
    for (int i = 0; i < 64; ++i) {
        if ((i += nextInt(&seed, 5) + 1) >= 64) continue;
        clayBands[i] = ORANGE_TERRACOTTA;
    }

    rand1 = nextInt(&seed, 4) + 2;
    for (int i = 0; i < rand1; ++i) {
        rand2 = nextInt(&seed, 3) + 1;
        rand3 = nextInt(&seed, 64);
        for (int i = 0; rand3 + i < 64 && i < rand2; ++i) {
            clayBands[rand3 + i] = YELLOW_TERRACOTTA;
        }
    }

    rand1 = nextInt(&seed, 4) + 2;
    for (int i = 0; i < rand1; ++i) {
        rand2 = nextInt(&seed, 3) + 2;
        rand3 = nextInt(&seed, 64);
        for (int i = 0; rand3 + i < 64 && i < rand2; ++i) {
            clayBands[rand3 + i] = BROWN_TERRACOTTA;
        }
    }

    rand1 = nextInt(&seed, 4) + 2;
    for (int i = 0; i < rand1; ++i) {
        rand2 = nextInt(&seed, 3) + 1;
        rand3 = nextInt(&seed, 64);
        for (int i = 0; rand3 + i < 64 && i < rand2; ++i) {
            clayBands[rand3 + i] = RED_TERRACOTTA;
        }
    }

    rand1 = nextInt(&seed, 3) + 3;
    rand2 = 0;
    for (int i = 0; i < rand1; ++i) {
        rand2 += nextInt(&seed, 16) + 4;
        if(rand2 < 64)
        { 
            clayBands[rand2] = WHITE_TERRACOTTA;
            
            if ((rand2 > 1) && nextBoolean(&seed))
            {
                clayBands[rand2 - 1] = LIGHT_GRAY_TERRACOTTA;
            }
            if ((rand2 < 63) && nextBoolean(&seed))
            {
                clayBands[rand2 + 1] = LIGHT_GRAY_TERRACOTTA;
            }
        }
    }
}

uint64_t findActualSeed(uint64_t seed)
{
    for(int n = 0; n < 262; n++)
    {
        seed = ((seed - 0xb) * 0xdfe05bcb1365) & ((1L << 48) - 1);
    }

    seed = (seed ^ 0x5deece66d);
    return seed;
}

int main()
{
    int buffer[64];
    for(uint64_t i = 0; i < (1ULL << 48); i++)
    {
        generateBands(buffer, i);
        if(buffer[5] != 4) continue;
        if(buffer[6] != 4) continue;
        if(buffer[7] != 4) continue;
        if(buffer[8] != 4) continue;
        if(buffer[9] != 4) continue;
        if(buffer[10] != 4) continue;
        if(buffer[11] != 4) continue;
        if(buffer[12] != 4) continue;
        if(buffer[13] != 4) continue;
        if(buffer[14] != 4) continue;
        if(buffer[15] != 4) continue;
        if(buffer[16] != 4) continue;
        if(buffer[17] != 4) continue;
        if(buffer[18] != 4) continue;
        printf("%ld\n", findActualSeed(i));
    }
    return 0;
}