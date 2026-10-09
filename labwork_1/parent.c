#include <stdint.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>

static char SERVER_PROGRAM_NAME[] = "child";

static uint32_t str_length(const char *s) {
	uint32_t len = 0;
	while (s[len] != '\0') {
		++len;
	}
	return len;
}

static void write_all(int32_t fd, const char *buf, size_t size) {
	size_t written = 0;
	while (written < size) {
		ssize_t res = write(fd, buf + written, size - written);
		if (res <= 0) {
			break;
		}
		written += (size_t)res;
	}
}

static ssize_t read_line(int32_t fd, char *buf, size_t max_size) {
	size_t idx = 0;
	char ch;
	while (idx + 1 < max_size) {
		ssize_t bytes = read(fd, &ch, 1);
		if (bytes <= 0) {
			if (idx == 0) {
				return -1;
			}
			break;
		}
		if (ch == '\r') {
			continue;
		}
		if (ch == '\n') {
			break;
		}
		buf[idx++] = ch;
	}
	buf[idx] = '\0';
	return (ssize_t)idx;
}

int main(int argc, char **argv) {
	char filename[1024];

	if (argc > 1) {
		uint32_t i = 0;
		while (argv[1][i] != '\0' && i < sizeof(filename) - 1) {
			filename[i] = argv[1][i];
			++i;
		}
		filename[i] = '\0';
	} else {
		const char prompt[] = "Enter output filename: ";
		write_all(STDOUT_FILENO, prompt, sizeof(prompt) - 1);

		ssize_t len = read_line(STDIN_FILENO, filename, sizeof(filename));
		if (len <= 0) {
			const char msg[] = "error: invalid filename\n";
			write_all(STDERR_FILENO, msg, sizeof(msg) - 1);
			exit(EXIT_FAILURE);
		}
	}

	char progpath[1024];
	{
		ssize_t len = readlink("/proc/self/exe", progpath, sizeof(progpath) - 1);
		if (len <= 0) {
			progpath[0] = '.';
			progpath[1] = '\0';
		} else {
			while (len > 0 && progpath[len] != '/') {
				--len;
			}
			progpath[len] = '\0';
		}
	}

	// NOTE: Open pipes
	int client_to_server[2];
	if (pipe(client_to_server) == -1) {
		const char msg[] = "error: failed to create pipe\n";
		write_all(STDERR_FILENO, msg, sizeof(msg) - 1);
		exit(EXIT_FAILURE);
	}

	int server_to_client[2];
	if (pipe(server_to_client) == -1) {
		const char msg[] = "error: failed to create pipe\n";
		write_all(STDERR_FILENO, msg, sizeof(msg) - 1);
		exit(EXIT_FAILURE);
	}

	const pid_t child = fork();

	switch (child) {
	case -1: {
		const char msg[] = "error: failed to spawn new process\n";
		write_all(STDERR_FILENO, msg, sizeof(msg) - 1);
		exit(EXIT_FAILURE);
	} break;

	case 0: {
		close(client_to_server[1]);
		close(server_to_client[0]);

		if (dup2(client_to_server[0], STDIN_FILENO) == -1) {
			_exit(EXIT_FAILURE);
		}
		close(client_to_server[0]);

		if (dup2(server_to_client[1], STDOUT_FILENO) == -1) {
			_exit(EXIT_FAILURE);
		}
		close(server_to_client[1]);

		{
			char path[1024];
			uint32_t p_len = str_length(progpath);
			uint32_t s_len = str_length(SERVER_PROGRAM_NAME);

			for (uint32_t i = 0; i < p_len; ++i) {
				path[i] = progpath[i];
			}
			path[p_len] = '/';
			for (uint32_t i = 0; i < s_len; ++i) {
				path[p_len + 1 + i] = SERVER_PROGRAM_NAME[i];
			}
			path[p_len + 1 + s_len] = '\0';

			char *const args[] = {SERVER_PROGRAM_NAME, filename, NULL};
			int32_t status = execv(path, args);

			if (status == -1) {
				const char msg[] = "error: failed to exec into child executable\n";
				write_all(STDERR_FILENO, msg, sizeof(msg) - 1);
				_exit(EXIT_FAILURE);
			}
		}
	} break;

	default: { 
		close(client_to_server[0]);
		close(server_to_client[1]);

		const char prompt[] = "Enter float numbers (empty line to exit):\n";
		write_all(STDOUT_FILENO, prompt, sizeof(prompt) - 1);

		char buf[4096];
		ssize_t bytes;

		while ((bytes = read_line(STDIN_FILENO, buf, sizeof(buf))) >= 0) {
			if (buf[0] == '\0') {
				break;
			}

			write_all(client_to_server[1], buf, (size_t)bytes);
			write_all(client_to_server[1], "\n", 1);

			char status_byte = 0;
			ssize_t res = read(server_to_client[0], &status_byte, 1);

			if (res <= 0 || status_byte == 'E') {
				const char msg[] = "error: division by zero detected! Terminating.\n";
				write_all(STDERR_FILENO, msg, sizeof(msg) - 1);
				break;
			}
		}

		close(client_to_server[1]);
		close(server_to_client[0]);

		wait(NULL);
	} break;
	}

	return 0;
}
