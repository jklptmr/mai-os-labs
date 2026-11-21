#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>

struct data {
    float val;
    int cmd;
};

int main() {
    char fname[256];
    if (scanf("%s", fname) != 1) {
        return 1;
    }

    char shm_name[32], sem_c_name[32], sem_p_name[32];
    int my_pid = getpid();
    snprintf(shm_name, 32, "/shm_%d", my_pid);
    snprintf(sem_c_name, 32, "/sc_%d", my_pid);
    snprintf(sem_p_name, 32, "/sp_%d", my_pid);

    int fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        return 1;
    }

    if (ftruncate(fd, sizeof(struct data)) == -1) {
        perror("ftruncate");
        return 1;
    }

    struct data *shm = mmap(NULL, sizeof(struct data), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    close(fd);

    sem_t *sem_c = sem_open(sem_c_name, O_CREAT, 0666, 0);
    sem_t *sem_p = sem_open(sem_p_name, O_CREAT, 0666, 0);
    if (sem_c == SEM_FAILED || sem_p == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        execl("./child", "child", shm_name, sem_c_name, sem_p_name, fname, NULL);
        perror("execl");
        shm->cmd = -1;
        sem_post(sem_p);
        return 1;
    }

    sem_wait(sem_p);

    float x;
    while (shm->cmd != -1 && scanf("%f", &x) == 1) {
        shm->val = x;
        shm->cmd = 0;
        sem_post(sem_c);
        sem_wait(sem_p);

        if (shm->cmd == -1) {
            printf("Деление на ноль\n");
            break;
        }

        int c = getchar();
        if (c == '\n' || c == EOF) {
            shm->cmd = 1;
            sem_post(sem_c);
            sem_wait(sem_p);
            if (c == EOF) {
                break;
            }
        }
    }

    if (shm->cmd != -1) {
        shm->cmd = 2;
        sem_post(sem_c);
    }

    wait(NULL);

    munmap(shm, sizeof(struct data));
    shm_unlink(shm_name);
    sem_close(sem_c);
    sem_close(sem_p);
    sem_unlink(sem_c_name);
    sem_unlink(sem_p_name);

    return 0;
}
