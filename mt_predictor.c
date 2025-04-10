#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <errno.h>
#include "mt19937ar.h"

#define PY_HEAD       16       // PyObject_HEAD size
#define IDX_BYTES      4
#define STATE_WORDS  624
#define STATE_BYTES (STATE_WORDS * sizeof(uint32_t))
#define CAND_BYTES  (PY_HEAD + IDX_BYTES + STATE_BYTES)

static inline double python_random(void) {
    uint32_t a = genrand_int32() >> 5;
    uint32_t b = genrand_int32() >> 6;
    return (a * 67108864.0 + b) / 9007199254740992.0;
}

static int try_candidate(uint8_t *base, size_t off) {
    int idx = *(int *)(base + off + PY_HEAD);
    if (idx < 0 || idx > STATE_WORDS) return 0;

    uint32_t *st = (uint32_t *)(base + off + PY_HEAD + IDX_BYTES);
    if (st[0] != 0x80000000U) return 0;  // Python特有の初期状態チェック

    mt_set_state(st, idx);

    double r1 = python_random();
    double r2 = python_random();
    if (r1 <= 1e-12 || r1 >= 0.999999999999) return 0;

    double r3 = python_random();
    printf("[+] state @ offset 0x%zx\n", off);
    printf("    next[1] random.random() = %.17f\n", r1);
    printf("    next[2] random.random() = %.17f\n", r2);
    printf("    next[3] random.random() = %.17f\n", r3);
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pid>\n", argv[0]);
        return 1;
    }
    pid_t pid = atoi(argv[1]);
    char maps_path[64], mem_path[64];
    snprintf(maps_path, sizeof maps_path, "/proc/%d/maps", pid);
    snprintf(mem_path, sizeof mem_path, "/proc/%d/mem", pid);

    FILE *maps = fopen(maps_path, "r");
    if (!maps) { perror("fopen maps"); return 1; }

    int memfd = open(mem_path, O_RDONLY);
    if (memfd < 0) { perror("open mem"); fclose(maps); return 1; }

    char line[256];
    while (fgets(line, sizeof line, maps)) {
        unsigned long start, end;
        char perm[5];
        if (sscanf(line, "%lx-%lx %4s", &start, &end, perm) != 3)
            continue;
        if (strstr(perm, "rw") == NULL) continue;

        size_t region = end - start;
        uint8_t *buf = malloc(region);
        if (!buf) continue;

        if (pread(memfd, buf, region, start) != (ssize_t)region) {
            free(buf);
            continue;
        }

        for (size_t off = 0; off + CAND_BYTES <= region; off += 4) {
            if (try_candidate(buf, off)) {
                free(buf);
                close(memfd);
                fclose(maps);
                return 0;
            }
        }
        free(buf);
    }

    puts("[-] MT state not found.");
    close(memfd);
    fclose(maps);
    return 1;
}
