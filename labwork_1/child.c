#include <stdint.h>
#include <stdbool.h>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

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

static void float_to_str(float val, char *buf) {
	if (val < 0.0f) {
		*buf++ = '-';
		val = -val;
	}

	int64_t int_part = (int64_t)val;
	float frac = val - (float)int_part;

	char temp[32];
	int32_t temp_idx = 0;

	if (int_part == 0) {
		temp[temp_idx++] = '0';
	} else {
		while (int_part > 0) {
			temp[temp_idx++] = (char)('0' + (int_part % 10));
			int_part /= 10;
		}
	}

	while (temp_idx > 0) {
		*buf++ = temp[--temp_idx];
	}

	*buf++ = '.';

	for (int32_t i = 0; i < 6; ++i) {
		frac *= 10.0f;
		int32_t digit = (int32_t)frac;
		if (digit > 9) {
			digit = 9;
		}
		*buf++ = (char)('0' + digit);
		frac -= (float)digit;
	}

	*buf++ = '\n';
	*buf = '\0';
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
	if (argc < 2) {
		const char msg[] = "error: child requires filename\n";
		write(STDERR_FILENO, msg, sizeof(msg) - 1);
		exit(EXIT_FAILURE);
	}

	int32_t file = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0600);
	if (file == -1) {
		const char msg[] = "error: failed to open requested file\n";
		write(STDERR_FILENO, msg, sizeof(msg) - 1);
		write(STDOUT_FILENO, "E", 1);
		exit(EXIT_FAILURE);
	}

	char buf[4096];
	ssize_t bytes;

	while ((bytes = read_line(STDIN_FILENO, buf, sizeof(buf))) >= 0) {
		char *ptr = buf;
		char *endptr = NULL;
		float result = 0.0f;
		int32_t count = 0;
		bool div_by_zero = false;

		while (*ptr != '\0') {
			while (*ptr == ' ' || *ptr == '\t') {
				++ptr;
			}
			if (*ptr == '\0') {
				break;
			}

			float val = strtof(ptr, &endptr);
			if (ptr == endptr) {
				break;
			}

			if (count == 0) {
				result = val;
			} else {
				if (val == 0.0f) {
					div_by_zero = true;
					break;
				}
				result /= val;
			}
			++count;
			ptr = endptr;
		}

		if (div_by_zero) {
			write(STDOUT_FILENO, "E", 1);
			close(file);
			exit(EXIT_FAILURE);
		}

		if (count > 0) {

			char out_str[64];
			float_to_str(result, out_str);

			uint32_t len = 0;
			while (out_str[len] != '\0') {
				++len;
			}
			write_all(file, out_str, len);
		}

		write(STDOUT_FILENO, "K", 1);
	}

	close(file);
	return 0;
}
