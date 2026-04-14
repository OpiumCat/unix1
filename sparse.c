#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
    int block_size = 4096;
    int opt;

    while ((opt = getopt(argc, argv, "b:")) != -1) {
        if (opt == 'b') {
            block_size = atoi(optarg);
            if (block_size <= 0) {
                fprintf(stderr, "Ошибка: размер блока должен быть положительным числом\n");
                exit(1);
            }
        } else {
            fprintf(stderr, "Использование: %s [-b размер] [входной_файл] выходной_файл\n", argv[0]);
            exit(1);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Ошибка: не указан выходной файл\n");
        exit(1);
    }

    char *input_name = NULL;
    char *output_name = NULL;
    if (argc - optind == 1) {
        output_name = argv[optind];
    } else if (argc - optind == 2) {
        input_name = argv[optind];
        output_name = argv[optind + 1];
    } else {
        fprintf(stderr, "Ошибка: слишком много аргументов\n");
        exit(1);
    }

    int in_fd;
    if (input_name != NULL) {
        in_fd = open(input_name, O_RDONLY);
        if (in_fd == -1) {
            perror("open input file");
            exit(1);
        }
    } else {
        in_fd = 0; // stdin
    }

    int out_fd = open(output_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd == -1) {
        perror("open output file");
        exit(1);
    }

    char *buf = (char *)malloc(block_size);
    if (buf == NULL) {
        perror("malloc");
        exit(1);
    }

    long long total_read = 0;
    ssize_t n;

    while ((n = read(in_fd, buf, block_size)) > 0) {
        total_read += n;

        int all_zero = 1;
        for (int i = 0; i < n; i++) {
            if (buf[i] != 0) {
                all_zero = 0;
                break;
            }
        }

        if (all_zero) {
            off_t seek_res = lseek(out_fd, n, SEEK_CUR);
            if (seek_res == (off_t)-1) {
                perror("lseek");
                exit(1);
            }
        } else {
            ssize_t written = write(out_fd, buf, n);
            if (written != n) {
                perror("write");
                exit(1);
            }
        }
    }

    if (n == -1) {
        perror("read");
        exit(1);
    }

    if (ftruncate(out_fd, (off_t)total_read) == -1) {
        perror("ftruncate");
        exit(1);
    }

    free(buf);
    if (close(out_fd) == -1) {
        perror("close output");
        exit(1);
    }
    if (input_name != NULL) {
        if (close(in_fd) == -1) {
            perror("close input");
            exit(1);
        }
    }

    return 0;
}