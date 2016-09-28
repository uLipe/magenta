// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <magenta/device/audio.h>
#include <mxio/io.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

#include <magenta/syscalls.h>
#include <magenta/types.h>

#include "wav.h"


#define DEV_AUDIO   "/dev/class/audio"

#define BUFFER_COUNT 2
#define BUFFER_SIZE 16384

#define BUFFER_EMPTY 0
#define BUFFER_BUSY 1
#define BUFFER_FULL 2

static mtx_t mutex = MTX_INIT;
static cnd_t empty_cond = CND_INIT;
static cnd_t full_cond = CND_INIT;

static bool done = false;

static uint8_t* buffers[BUFFER_COUNT];
static int buffer_states[BUFFER_COUNT];
static int buffer_sizes[BUFFER_COUNT];
static int empty_index = 0;
static int full_index = -1;
static bool file_done;

mx_time_t start_time = 0;
uint64_t sample_count = 0;

static int get_empty(void) {
    int index, other;

    mtx_lock(&mutex);

    while (empty_index == -1)
        cnd_wait(&empty_cond, &mutex);

    index = empty_index;
    other = (index == 0 ? 1 : 0);
    buffer_states[index] = BUFFER_BUSY;
    if (buffer_states[other] == BUFFER_EMPTY)
        empty_index = other;
    else
        empty_index = -1;

    mtx_unlock(&mutex);
    return index;
}

static void put_empty(int index) {
    mtx_lock(&mutex);

    buffer_states[index] = BUFFER_EMPTY;
    if (empty_index == -1) {
        empty_index = index;
        cnd_signal(&empty_cond);
    }

    mtx_unlock(&mutex);
}

static int get_full(void) {
    int index, other;

    mtx_lock(&mutex);

    if (file_done) {
        mtx_unlock(&mutex);
        return -1;
    }

    while (full_index == -1)
        cnd_wait(&full_cond, &mutex);

    index = full_index;
    other = (index == 0 ? 1 : 0);
    buffer_states[index] = BUFFER_BUSY;
    if (buffer_states[other] == BUFFER_FULL)
        full_index = other;
    else
        full_index = -1;

    mtx_unlock(&mutex);
    return index;
}

static void put_full(int index) {
    mtx_lock(&mutex);

    buffer_states[index] = BUFFER_FULL;
    if (full_index == -1) {
        full_index = index;
        cnd_signal(&full_cond);
    }

    mtx_unlock(&mutex);
}

static void set_done(void) {
    mtx_lock(&mutex);

    file_done = true;
    cnd_signal(&full_cond);
    mtx_unlock(&mutex);
}

static int file_read_thread(void* arg) {
    int fd = (int)(uintptr_t)arg;

    while (!done) {
        int index = get_empty();
        int count = read(fd, buffers[index], BUFFER_SIZE);
        if (count <= 0) {
            set_done();
            break;
        }
        buffer_sizes[index] = count;
        put_full(index);
    }

    return 0;
}
static void do_play(int src_fd, int dest_fd)
{
    thrd_t thread;
    thrd_create_with_name(&thread, file_read_thread, (void *)(uintptr_t)src_fd, "file_read_thread");
    thrd_detach(thread);

    while (!done) {
        int index = get_full();
        if (index < 0) break;
        uint8_t* buffer = buffers[index];
        int buffer_size = buffer_sizes[index];

        if (write(dest_fd, buffer, buffer_size) != buffer_size) {
            done = true;
        }

        put_empty(index);
    }
}

static int open_sink(void) {
    struct dirent* de;
    DIR* dir = opendir(DEV_AUDIO);
    if (!dir) {
        printf("Error opening %s\n", DEV_AUDIO);
        return -1;
    }

    while ((de = readdir(dir)) != NULL) {
       char devname[128];

        snprintf(devname, sizeof(devname), "%s/%s", DEV_AUDIO, de->d_name);
        int fd = open(devname, O_RDWR);
        if (fd < 0) {
            printf("Error opening %s\n", devname);
            continue;
        }

        int device_type;
        int ret = ioctl_audio_get_device_type(fd, &device_type);
        if (ret != sizeof(device_type)) {
            printf("ioctl_audio_get_device_type failed for %s\n", devname);
            goto next;
        }
        if (device_type != AUDIO_TYPE_SINK) {
            goto next;
        }

        // making assumption here that WAV file has 44100 sample rate
        uint32_t sample_rate = 44100;
        ret = ioctl_audio_set_sample_rate(fd, &sample_rate);
        if (ret != NO_ERROR) {
            printf("%s sample rate %d not supported\n", devname, sample_rate);
            goto next;
        }
        ioctl_audio_start(fd);

        closedir(dir);
        return fd;

next:
        close(fd);
    }

    closedir(dir);
    return -1;

}

static void play_file(const char* path, int dest_fd) {
    riff_wave_header riff_wave_header;
    chunk_header chunk_header;
    chunk_fmt chunk_fmt;
    int more_chunks = 1;

    int src_fd = open(path, O_RDONLY);
    if (src_fd < 0) {
        fprintf(stderr, "Unable to open file '%s'\n", path);
        return;
    }

    read(src_fd, &riff_wave_header, sizeof(riff_wave_header));
    if ((riff_wave_header.riff_id != ID_RIFF) ||
        (riff_wave_header.wave_id != ID_WAVE)) {
        fprintf(stderr, "Error: '%s' is not a riff/wave file\n", path);
        return;
    }

   for (int i = 0; i < BUFFER_COUNT; i++) {
        buffer_states[i] = BUFFER_EMPTY;
    }
    file_done = false;

    do {
        read(src_fd, &chunk_header, sizeof(chunk_header));

        switch (chunk_header.id) {
        case ID_FMT:
            read(src_fd, &chunk_fmt, sizeof(chunk_fmt));
            /* If the format header is larger, skip the rest */
            if (chunk_header.sz > sizeof(chunk_fmt))
                lseek(src_fd, chunk_header.sz - sizeof(chunk_fmt), SEEK_CUR);
            break;
        case ID_DATA:
            /* Stop looking for chunks */
            more_chunks = 0;
            break;
        default:
            /* Unknown chunk, skip bytes */
            lseek(src_fd, chunk_header.sz, SEEK_CUR);
        }
    } while (more_chunks);

    printf("playing %s\n", path);

    do_play(src_fd, dest_fd);
    close(src_fd);
}

static void play_files(const char* directory, int dest_fd) {
    struct dirent* de;
    DIR* dir = opendir(directory);
    if (!dir) {
        printf("Error opening %s\n", directory);
        return;
    }

    while ((de = readdir(dir)) != NULL) {
        char path[PATH_MAX];
        int namelen = strlen(de->d_name);
        if (namelen < 5 || strcasecmp(de->d_name + namelen - 4, ".wav") != 0) continue;

        snprintf(path, sizeof(path), "%s/%s", directory, de->d_name);
        play_file(path, dest_fd);
    }

    closedir(dir);

}

int main(int argc, char **argv) {
    int dest_fd = open_sink();
    if (dest_fd < 0) {
        printf("couldn't find a usable audio sink\n");
        return -1;
    }

    for (int i = 0; i < BUFFER_COUNT; i++) {
        buffers[i] = malloc(BUFFER_SIZE);
    }

    if (argc == 1) {
        play_files("/data", dest_fd);
    } else {
        for (int i = 1; i < argc && !done; i++) {
            play_file(argv[i], dest_fd);
        }
    }

    close(dest_fd);

    for (int i = 0; i < BUFFER_COUNT; i++) {
        free(buffers[i]);
    }

    return 0;
}
