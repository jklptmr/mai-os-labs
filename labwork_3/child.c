#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

struct data {
    float val;
    int cmd;
};

int main(int argc, char *argv[]) {
    if (argc != 5) {
        return 1;
    }

    int fd = shm_open(argv[1], O_RDWR, 0);
    if (fd == -1) {
        perror("shm_open");
        return 1;
    }

    struct data *shm = mmap(NULL, sizeof(struct data), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    close(fd);

    sem_t *sem_c = sem_open(argv[2], 0);
    sem_t *sem_p = sem_open(argv[3], 0);
    if (sem_c == SEM_FAILED || sem_p == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }

    FILE *f = fopen(argv[4], "w");
    if (f == NULL) {
        perror("fopen");
        shm->cmd = -1;
        sem_post(sem_p);
        return 1;
    }

    shm->cmd = 0;
    sem_post(sem_p);

    float res = 0;
    int first = 1;

    while (1) {
        sem_wait(sem_c);

        if (shm->cmd == 2) {
            break;
        }

        if (shm->cmd == 0) {
            if (first) {
                res = shm->val;
                first = 0;
            } else {
                if (shm->val == 0) {
                    shm->cmd = -1;
                    sem_post(sem_p);
                    break;
                }
                res = res / shm->val;
            }
            sem_post(sem_p);
        } else if (shm->cmd == 1) {
            fprintf(f, "%f\n", res);
            fflush(f);
            first = 1;
            sem_post(sem_p);
        }
    }

    fclose(f);
    munmap(shm, sizeof(struct data));
    sem_close(sem_c);
    sem_close(sem_p);

    return 0;
}
