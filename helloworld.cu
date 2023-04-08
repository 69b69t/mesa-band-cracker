#include <stdio.h>
#include <stdint.h>

__device__ void setSeed(uint64_t *seed, const uint64_t value)
{
    *seed = (value ^ 0x5deece66d) & ((1ULL << 48) - 1);
}

__device__ int next(uint64_t *seed, const int bits)
{
    *seed = (*seed * 0x5deece66d + 0xb) & ((1ULL << 48) - 1);
    return (int) ((int64_t)*seed >> (48 - bits));
}


//make a lot of hardcoded nextints so ofast can do magic with optimizing out the modulo
__device__ int nextInt(uint64_t *seed, const int n)
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

//NEXTINT 3
__device__ int nextInt3(uint64_t *seed)
{
    int bits, val;
    do {
        bits = next(seed, 31);
        val = bits % 3;
    }
    while (bits - val + 2 < 0);
    return val;
}

//NEXTINT 5
__device__ int nextInt5(uint64_t *seed)
{
    int bits, val;
    do {
        bits = next(seed, 31);
        val = bits % 5;
    }
    while (bits - val + 4 < 0);
    return val;
}

//NEXTINT 4
__device__ int nextInt4(uint64_t *seed)
{
    uint64_t x = 4 * (uint64_t)next(seed, 31);
    return (int) ((int64_t) x >> 31);
}

//NEXTINT 4
__device__ int nextInt16(uint64_t *seed)
{
    uint64_t x = 16 * (uint64_t)next(seed, 31);
    return (int) ((int64_t) x >> 31);
}

//NEXTINT 64
__device__ int nextInt64(uint64_t *seed)
{
    uint64_t x = 64 * (uint64_t)next(seed, 31);
    return (int) ((int64_t) x >> 31);
}

__device__ void simplexNoiseStep(uint64_t *seed)
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

__device__ void printArr(const int arr[64])
{
    for(int i = 0; i < 64; i++)
    {
        printf("%d", arr[i]);
    }
}

__device__ int isViable(const int clayBands[64], const int checkNum)
{
    const int checkAgainst[64] =
    {
        0, 0, 1, 0, 0, 0, 0, 1,
        0, 0, 2, 5, 6, 0, 1, 2,
        0, 0, 1, 0, 0, 0, 1, 0,
        0, 0, 0, 1, 0, 1, 6, -1,
        -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1
    };

    for(int i = 0; i < 31; i++)
    {
        if(checkAgainst[i] == checkNum)
        {
            if(checkAgainst[i] != clayBands[i]) return 0;
        }
    }
    return 1;
}

__device__ int nextBoolean(uint64_t *seed)
{
    return next(seed, 1) != 0;
}

__device__ void addOrange(int clayBands[64], uint64_t *seed)
{
    for (int i = 0; i < 64; i++) {
        if ((i += nextInt5(seed) + 1) >= 64) continue;
        clayBands[i] = 1;
    }
}

__device__ void addBrown(int clayBands[64], uint64_t *seed)
{
    int rand1 = nextInt4(seed) + 2;
    int rand2;
    int rand3;
    for (int i = 0; i < rand1; i++) {
        rand2 = nextInt3(seed) + 2;
        rand3 = nextInt64(seed);
        for (int j = 0; rand3 + j < 64 && j < rand2; j++) {
            clayBands[rand3 + j] = 3;
        }
    }
}

__device__ void addColor(int clayBands[64], uint64_t *seed, const int color)
{
    int rand1 = nextInt4(seed) + 2;
    int rand2;
    int rand3;
    for (int i = 0; i < rand1; i++) {
        rand2 = nextInt3(seed) + 1;
        rand3 = nextInt64(seed);
        for (int j = 0; rand3 + j < 64 && j < rand2; j++) {
            clayBands[rand3 + j] = color;
        }
    }
}

__device__ void addWhite(int clayBands[64], uint64_t *seed)
{
    int rand1 = nextInt3(seed) + 3;
    int rand2 = 0;
    for (int i = 0; i < rand1; i++) {
        rand2 += nextInt16(seed) + 4;
        if(rand2 < 64)
        { 
            clayBands[rand2] = 6;
            
            if ((rand2 > 1) && nextBoolean(seed))
            {
                clayBands[rand2 - 1] = 5;
            }
            if ((rand2 < 63) && nextBoolean(seed))
            {
                clayBands[rand2 + 1] = 5;
            }
        }
    }
}

//modulos (nextInt)
// 3 4 5 64

__device__ uint64_t findActualSeed(uint64_t seed)
{
    for(int n = 0; n < 262; n++)
    {
        seed = ((seed - 0xb) * 0xdfe05bcb1365) & ((1ULL << 48) - 1);
    }

    seed = (seed ^ 0x5deece66d);
    return seed;
}

__device__ void generateBands(const uint64_t l)
{
    uint64_t seed;
    seed = l;
    int clayBands[64] = {0};

    //setSeed(&seed, l);

    //simplexNoiseStep(&seed);

    //check correct terracotta per layer
    addOrange(clayBands, &seed);

    //can i remove this?
    //if(isViable(clayBands, 0) == 0) return;
    
    if(!isViable(clayBands, 1)) return;

    addColor(clayBands, &seed, 2);
    //if(!isViable(clayBands, 0)) return;
    //if(!isViable(clayBands, 1)) return;
    if(!isViable(clayBands, 2)) return;

    addBrown(clayBands, &seed);
    //if(!isViable(clayBands, 0)) return;
    //if(!isViable(clayBands, 1)) return;
    //if(!isViable(clayBands, 2)) return;
    if(!isViable(clayBands, 3)) return;

    addColor(clayBands, &seed, 4);
    //if(!isViable(clayBands, 0)) return;
    //if(!isViable(clayBands, 1)) return;
    //if(!isViable(clayBands, 2)) return;
    //if(!isViable(clayBands, 3)) return;
    if(!isViable(clayBands, 4)) return;

    addWhite(clayBands, &seed);
    if(!isViable(clayBands, 0)) return;
    if(!isViable(clayBands, 1)) return;
    if(!isViable(clayBands, 2)) return;
    if(!isViable(clayBands, 3)) return;
    if(!isViable(clayBands, 4)) return;
    if(!isViable(clayBands, 5)) return;
    if(!isViable(clayBands, 6)) return;

    printf("%lld\n", findActualSeed(l));
}

__global__ void craccky(uint64_t bigOffset)
{
    uint64_t idX = bigOffset * 1048576;
    idX += ((uint64_t)threadIdx.x + (uint64_t)blockIdx.x * 1024);
    generateBands(idX);
}

int main()
{
    cudaSetDevice(0);
    for(int i = 0; i < 268435456; i++)
    {
        //does 2^20 seeds, out of 2^48, so only(!) need to run it 2^28 times (268435456)
        //to do 2^32, run 2^12 times (4096)
        craccky<<<1024,1024>>>(i);
        cudaStreamSynchronize(0);
        if((0x0000ffff & i) == 0) printf("help he voices: (%.14f%%)\n", 100*(i / 268435456.0));
    }
    return 0;
}