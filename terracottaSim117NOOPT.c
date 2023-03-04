#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#define THREADS 16

static inline void setSeed(uint64_t *seed, uint64_t value)
{
    *seed = (value ^ 0x5deece66d) & ((1ULL << 48) - 1);
}

static inline int next(uint64_t *seed, const int bits)
{
    *seed = (*seed * 0x5deece66d + 0xb) & ((1ULL << 48) - 1);
    return (int) ((int64_t)*seed >> (48 - bits));
}

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

void simplexNoiseStep(uint64_t *seed)
{
    next(seed, 1);
    next(seed, 1);
    next(seed, 1);
    next(seed, 1);
    next(seed, 1);
    next(seed, 1);
    for(int n = 0; n < 256; ++n)
    {
        nextInt(seed, 256 - n);
    }
}

void generateBands(int clayBands[64], uint64_t l)
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
    uint64_t seed;
    int rand1;
    int rand2;
    int rand3;


    setSeed(&seed, l);

    simplexNoiseStep(&seed);

    for(int i = 0; i < 64; i++)
    {
        clayBands[i] = TERRACOTTA;
    }

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

void * spawnThread(void * threadNumPointer)
{
    int threadNum = *(int*)threadNumPointer;
    int buffer[64];
    for(uint64_t i = threadNum; i < (1ULL << 32); i += THREADS)
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
        printf("%ld\n", i);
    }
    return NULL;
}

int main()
{
    int* threadNumArray = malloc(4 * THREADS);
    pthread_t threadIdArray[THREADS];
    for(int i = 0; i < THREADS; i++)
    {
        threadNumArray[i] = i;
    }

    for(int i = 0; i < THREADS; i++)
    {
        pthread_create(&threadIdArray[i], NULL, spawnThread, &threadNumArray[i]);
    }

    for(int i = 0; i < THREADS; i++)
    {
        pthread_join(threadIdArray[i], NULL);
    }

    return 0;
}