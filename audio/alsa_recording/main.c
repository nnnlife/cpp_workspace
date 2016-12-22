#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <signal.h>

int filedesc = -1;


typedef struct
{
    char RIFF_marker[4];
    uint32_t data_size;
    char filetype_header[4];
    char format_marker[4];
    uint32_t data_header_length;
    uint16_t format_type;
    uint16_t number_of_channels;
    uint32_t sample_rate;
    uint32_t bytes_per_second;
    uint16_t bytes_per_frame;
    uint16_t bits_per_sample;
} WaveHeader;

WaveHeader *hdr;

WaveHeader *generic_wav_header(uint32_t sample_rate, uint16_t bit_depth, uint16_t channels)
{
    WaveHeader *hdr;
    hdr = malloc(sizeof(*hdr));
    if (!hdr) return NULL;

    memcpy(hdr->RIFF_marker, "RIFF", 4);
    memcpy(hdr->filetype_header, "WAVE", 4);
    memcpy(hdr->format_marker, "fmt ", 4);
    hdr->data_header_length = 16;
    hdr->format_type = 1;
    hdr->number_of_channels = channels;
    hdr->sample_rate = sample_rate;
    hdr->data_size = 0;
    hdr->bytes_per_second = sample_rate * channels * bit_depth / 8;
    hdr->bytes_per_frame = channels * bit_depth / 8;
    hdr->bits_per_sample = bit_depth;

    return hdr;
}

int write_wav_header(int fd, WaveHeader *hdr)
{
    if (!hdr)
        return -1;

    uint32_t empty_size = 0;

    write(fd, hdr->RIFF_marker, 4);
    write(fd, &empty_size, 4); //file_size
    write(fd, hdr->filetype_header, 4);
    write(fd, hdr->format_marker, 4);
    write(fd, &hdr->data_header_length, 4);
    write(fd, &hdr->format_type, 2);
    write(fd, &hdr->number_of_channels, 2);
    write(fd, &hdr->sample_rate, 4);
    write(fd, &hdr->bytes_per_second, 4);
    write(fd, &hdr->bytes_per_frame, 2);
    write(fd, &hdr->bits_per_sample, 2);
    write(fd, "data", 4);

    //uint32_t data_size = hdr->file_size - 36;
    write(fd, &empty_size, 4);

    return 0;
}

void my_function(int sig) {
    lseek(filedesc, 4, SEEK_SET);
    uint32_t file_size = hdr->data_size + 36;
    write(filedesc, &file_size, 4);
    lseek(filedesc, 40, SEEK_SET);
    write(filedesc, &hdr->data_size, 4);

    fsync(filedesc);
    close(filedesc);
    fprintf(stderr, "FILE SIZE : %d B, DATA SIZE : %d B\n", file_size, hdr->data_size);
    exit(0);
}

main (int argc, char *argv[])
{
    int i;
    int err;
    char *buffer;
    int buf_size = 0;
    int buffer_frames = 128;
    unsigned int rate = 16000;
    snd_pcm_t *capture_handle;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;

    signal(SIGINT, my_function);

    if (argc < 4) {
        fprintf(stderr, "Usage: %s <device> <wavfile> <sampling rate>\n");
        exit(EXIT_FAILURE);
    }

    rate = atoi(argv[4]);
    hdr = generic_wav_header(rate, 16, 2);
    filedesc = open(argv[2], O_WRONLY | O_CREAT, 0644);
    if (filedesc < 0) {
        fprintf(stderr, "Cannoe open file: %s\n", argv[2]);
        exit(EXIT_FAILURE);
    }
    if ((err = write_wav_header(filedesc, hdr)) < 0) {
        fprintf (stderr, "cannot write wav header %s\n", argv[2]);
        close(filedesc);
        exit (1);
    }

    if ((err = snd_pcm_open (&capture_handle, argv[1], SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        fprintf (stderr, "cannot open audio device %s (%s)\n", 
        argv[1],
        snd_strerror (err));
        exit (1);
    }

    fprintf(stdout, "audio interface opened\n");

    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
        snd_strerror (err));
        exit (1);
    }

    fprintf(stdout, "hw_params allocated\n");

    if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
        snd_strerror (err));
        exit (1);
    }

    fprintf(stdout, "hw_params initialized\n");

    if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf (stderr, "cannot set access type (%s)\n",
        snd_strerror (err));
        exit (1);
    }

    fprintf(stdout, "hw_params access setted\n");

    if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, format)) < 0) {
        fprintf (stderr, "cannot set sample format (%s)\n",
        snd_strerror (err));
        exit (1);
    }

    fprintf(stdout, "hw_params format setted\n");

    if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
        fprintf (stderr, "cannot set sample rate (%s)\n",
        snd_strerror (err));
        exit (1);
    }

    fprintf(stdout, "hw_params rate setted\n");

    if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, hdr->number_of_channels)) < 0) {
        fprintf (stderr, "cannot set channel count (%s)\n",
        snd_strerror (err));
        exit (1);
    }

    fprintf(stdout, "hw_params channels setted\n");

    if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot set parameters (%s)\n",
        snd_strerror (err));
        exit (1);
    }

    fprintf(stdout, "hw_params setted\n");

    snd_pcm_hw_params_free (hw_params);

    fprintf(stdout, "hw_params freed\n");

    if ((err = snd_pcm_prepare (capture_handle)) < 0) {
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
               snd_strerror (err));
        exit (1);
    }

    fprintf(stdout, "audio interface prepared\n");


    fprintf(stdout, "buffer allocated\n");

    buf_size = buffer_frames * snd_pcm_format_width(format) / 8 * hdr->number_of_channels;
    
    while (1) {
        int ret = 0;
        buffer = malloc(buf_size);
        if ((err = snd_pcm_readi (capture_handle, buffer, buffer_frames)) != buffer_frames) {
            fprintf (stderr, "read from audio interface failed (%s)\n", err, snd_strerror (err));
            exit (1);
        }
        ret = write(filedesc, buffer, buf_size);
        if (ret != buf_size) {
            fprintf(stderr, "did not write all data in buffer\n");
        }
        hdr->data_size += ret;
        free(buffer);
    }

    fprintf(stdout, "buffer freed\n");

    snd_pcm_close (capture_handle);
    fprintf(stdout, "audio interface closed\n");

    exit (0);
}
