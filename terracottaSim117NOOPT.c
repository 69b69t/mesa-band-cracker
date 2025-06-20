#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdatomic.h>

#define THREADS 16
#define PROGRESS_INTERVAL (1ULL << 28)
#define TOTAL_SEEDS (1ULL << 32)

FILE *outputFile;
pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER;

atomic_uint_fast64_t globalCount = 0;
time_t startTime;

static inline void setSeed(uint64_t *seed, uint64_t value) {
    *seed = (value ^ 0x5deece66d) & ((1ULL << 48) - 1);
}

static inline int next(uint64_t *seed, const int bits) {
    *seed = (*seed * 0x5deece66d + 0xb) & ((1ULL << 48) - 1);
    return (int)((int64_t)*seed >> (48 - bits));
}

static inline int nextInt(uint64_t *seed, const int n) {
    int bits, val;
    const int m = n - 1;
    if ((m & n) == 0) {
        uint64_t x = n * (uint64_t)next(seed, 31);
        return (int)((int64_t)x >> 31);
    }
    do {
        bits = next(seed, 31);
        val = bits % n;
    } while (bits - val + m < 0);
    return val;
}

int nextBoolean(uint64_t *seed) {
    return next(seed, 1) != 0;
}

void simplexNoiseStep(uint64_t *seed) {
    for (int i = 0; i < 6; i++) next(seed, 1);
    for (int n = 0; n < 256; ++n) nextInt(seed, 256 - n);
}

void generateBands(int clayBands[64], uint64_t l) {
    enum terracottaTypes {
        TERRACOTTA, ORANGE_TERRACOTTA, YELLOW_TERRACOTTA,
        BROWN_TERRACOTTA, RED_TERRACOTTA, LIGHT_GRAY_TERRACOTTA, WHITE_TERRACOTTA
    };

    uint64_t seed;
    setSeed(&seed, l);
    simplexNoiseStep(&seed);

    for (int i = 0; i < 64; i++) clayBands[i] = TERRACOTTA;

    for (int i = 0; i < 64; ++i) {
        if ((i += nextInt(&seed, 5) + 1) >= 64) continue;
        clayBands[i] = ORANGE_TERRACOTTA;
    }

    int rand1 = nextInt(&seed, 4) + 2;
    for (int i = 0; i < rand1; ++i) {
        int rand2 = nextInt(&seed, 3) + 1;
        int rand3 = nextInt(&seed, 64);
        for (int j = 0; rand3 + j < 64 && j < rand2; ++j)
            clayBands[rand3 + j] = YELLOW_TERRACOTTA;
    }

    rand1 = nextInt(&seed, 4) + 2;
    for (int i = 0; i < rand1; ++i) {
        int rand2 = nextInt(&seed, 3) + 2;
        int rand3 = nextInt(&seed, 64);
        for (int j = 0; rand3 + j < 64 && j < rand2; ++j)
            clayBands[rand3 + j] = BROWN_TERRACOTTA;
    }

    rand1 = nextInt(&seed, 4) + 2;
    for (int i = 0; i < rand1; ++i) {
        int rand2 = nextInt(&seed, 3) + 1;
        int rand3 = nextInt(&seed, 64);
        for (int j = 0; rand3 + j < 64 && j < rand2; ++j)
            clayBands[rand3 + j] = RED_TERRACOTTA;
    }

    rand1 = nextInt(&seed, 3) + 3;
    int rand2 = 0;
    for (int i = 0; i < rand1; ++i) {
        rand2 += nextInt(&seed, 16) + 4;
        if (rand2 < 64) {
            clayBands[rand2] = WHITE_TERRACOTTA;
            if (rand2 > 1 && nextBoolean(&seed))
                clayBands[rand2 - 1] = LIGHT_GRAY_TERRACOTTA;
            if (rand2 < 63 && nextBoolean(&seed))
                clayBands[rand2 + 1] = LIGHT_GRAY_TERRACOTTA;
        }
    }
}

void *spawnThread(void *arg) {
    int threadNum = *(int *)arg;
    int buffer[64];

    for (uint64_t i = threadNum; i < TOTAL_SEEDS; i += THREADS) {
        generateBands(buffer, i);

        if (buffer[5] == 4 && buffer[6] == 4 && buffer[7] == 4 && buffer[8] == 4 &&
            buffer[9] == 4 && buffer[10] == 4 && buffer[11] == 4 && buffer[12] == 4 &&
            buffer[13] == 4 && buffer[14] == 4 && buffer[15] == 4 && buffer[16] == 4) {
            
            pthread_mutex_lock(&fileMutex);
            fprintf(outputFile, "%llu\n", i);
            fflush(outputFile);
            pthread_mutex_unlock(&fileMutex);
        }

        uint64_t globalProgress = atomic_fetch_add(&globalCount, 1);
        if ((globalProgress % PROGRESS_INTERVAL) == 0) {
            time_t now = time(NULL);
            double elapsed = difftime(now, startTime);
            double percent = (100.0 * globalProgress) / TOTAL_SEEDS;
            double est_total_time = elapsed / (percent / 100.0);
            double est_remaining = est_total_time - elapsed;

            printf("[Progress] %.2f%% done | Elapsed: %.0fs | ETA: %.0fs\n",
                percent, elapsed, est_remaining);
        }
    }

    printf("[Thread %2d] Done.\n", threadNum);
    return NULL;
}

int main() {
    printf("[*] Starting Mesa Band Half Seed Cracker");

    outputFile = fopen("found_seeds.txt", "w");
    if (!outputFile) {
        perror("uh oh");
        return 1;
    }

    startTime = time(NULL);

    pthread_t threads[THREADS];
    int threadIds[THREADS];

    for (int i = 0; i < THREADS; i++) {
        threadIds[i] = i;
        if (pthread_create(&threads[i], NULL, spawnThread, &threadIds[i]) != 0) {
            perror("uh oh");
            return 1;
        }
    }

    for (int i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    fclose(outputFile);
    printf("[*] Done. Results saved to 'found_seeds.txt'\n");
    return 0;
}
