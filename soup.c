#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#define BITS_PER_SAMPLE 16


typedef struct {
    uint8_t  riff_header[4];
    uint32_t wav_size;
    uint8_t  wave_header[4];
    uint8_t  fmt_header[4];
    uint32_t fmt_chunk_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint8_t  data_header[4];
    uint32_t data_bytes;
} WAVHeader;

void writeWAVHeader(FILE* file, int sampleRate, int bitsPerSample, int channels, int duration) {
    WAVHeader header;
    int bytesPerSample = bitsPerSample / 8;
    int dataSize = sampleRate * channels * bytesPerSample * duration;

    header.riff_header[0] = 'R';
    header.riff_header[1] = 'I';
    header.riff_header[2] = 'F';
    header.riff_header[3] = 'F';
    header.wav_size = dataSize + sizeof(WAVHeader) - 8;
    header.wave_header[0] = 'W';
    header.wave_header[1] = 'A';
    header.wave_header[2] = 'V';
    header.wave_header[3] = 'E';

    header.fmt_header[0] = 'f';
    header.fmt_header[1] = 'm';
    header.fmt_header[2] = 't';
    header.fmt_header[3] = ' ';
    header.fmt_chunk_size = 16;
    header.audio_format = 1;
    header.num_channels = channels;
    header.sample_rate = sampleRate;
    header.byte_rate = sampleRate * channels * bytesPerSample;
    header.block_align = channels * bytesPerSample;
    header.bits_per_sample = bitsPerSample;

    header.data_header[0] = 'd';
    header.data_header[1] = 'a';
    header.data_header[2] = 't';
    header.data_header[3] = 'a';
    header.data_bytes = dataSize;

    fwrite(&header, sizeof(header), 1, file);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    char input[1000];
    double frequencies[100];
    int numFrequencies = 0;
    int stereo = 0;
    int SAMPLE_RATE = 44100;
    int CHANNELS = 2;
    int DURATION = 5;
    int squareSampleL;
    int squareL;
    int squareR;
    int type=0;
    double tanhFactor;

    fprintf(stderr, ".---. .--. .   . .,--.\n"
    "`---.(    )|   | |    )\n"
    "`---' `--' `---`-|`--'\n"
    "~~~~~~~~~~~~~~~~ | ~~~~\n"
    "  by msx * v0.1  | \n"
    "                 '\n");

    printf("freqs: ");
    fgets(input, sizeof(input), stdin);

    char *token = strtok(input, " ");
    while (token != NULL) {
        frequencies[numFrequencies++] = atof(token);
        token = strtok(NULL, " ");
    }

    printf("saw/square/sine (0/1/2): ");
    scanf("%d", &type);

    if (type==2)
    {
    printf("sine tanh factor (>0.5): ");
    scanf("%lf", &tanhFactor);
    }

    printf("stereo (0/1): ");
    scanf("%d", &stereo);

    printf("sample rate: ");
    scanf("%d", &SAMPLE_RATE);

    printf("length: ");
    scanf("%d", &DURATION);

    char filename[64] = "soup_";
    for (int i = 0; i < numFrequencies; ++i) {
        char freq_str[20];
        sprintf(freq_str, "%.0f", frequencies[i]);
        strcat(filename, freq_str);
        if (i < numFrequencies - 1)
            strcat(filename, "_");
    }
    strcat(filename, ".wav");

    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("something went wrong.\n");
        return 1;
    }

    if (type>=2) {type=2;}
    else if (type<=0) {type=0;}

    if (tanhFactor<=0.5) {tanhFactor=0.5;}

    if (SAMPLE_RATE<=0) {SAMPLE_RATE=44100;}
    else if (SAMPLE_RATE>=96000) {SAMPLE_RATE=96000;}

    if (DURATION<=0) {DURATION=5;}
    else if (DURATION>=60) {DURATION=60;}

    if (stereo>=1) {CHANNELS=2; stereo=1;}
    else {CHANNELS=1; stereo=0;}

    writeWAVHeader(file, SAMPLE_RATE, BITS_PER_SAMPLE, CHANNELS, DURATION);

    const double amplitude = 32767.0 / numFrequencies;
    double angleL[numFrequencies], angleR[numFrequencies];
    double increment[numFrequencies];
    double phaseOffsetL[numFrequencies], phaseOffsetR[numFrequencies];

    for (int i = 0; i < numFrequencies; ++i) {
        phaseOffsetL[i] = (double)rand() / RAND_MAX;
        if (stereo) {
            phaseOffsetR[i] = (double)rand() / RAND_MAX;
            }
    }

    for (int i = 0; i < numFrequencies; ++i) {
        angleL[i] = phaseOffsetL[i];
        if (stereo) {
            angleR[i] = phaseOffsetR[i];
            }
        increment[i] = frequencies[i] / SAMPLE_RATE;
    }

    for (int i = 0; i < SAMPLE_RATE * DURATION; ++i) {
        int16_t sampleL = 0, sampleR = 0;
        for (int j = 0; j < numFrequencies; ++j) {
            if (type==1) {
                if (angleL[j] > 0.5) {squareL=1;} else {squareL=0;}
                sampleL += (int16_t)(amplitude * (2.0 * squareL - 1.0));}
            else if (type==2) {
                sampleL += (int16_t)(amplitude * (tanh(sin(2.0 * M_PI*(angleL[j]))*tanhFactor)));}
            else {sampleL += (int16_t)(amplitude * (2.0 * angleL[j] - 1.0));}
            if (stereo) {
                if (type==1) {
                    if (angleR[j] > 0.5) {squareR=1;} else {squareR=0;}
                    sampleR += (int16_t)(amplitude * (2.0 * squareR - 1.0));}
            else if (type==2) {
                sampleR += (int16_t)(amplitude * (tanh(sin(2.0 * M_PI*(angleR[j]))*tanhFactor)));}
            else {sampleR += (int16_t)(amplitude * (2.0 * angleR[j] - 1.0));}
                    }
            angleL[j] += increment[j];
            if (stereo) {
                angleR[j] += increment[j];
                }
            if (angleL[j] >= 1.0) {
                angleL[j] -= 1.0;
            }
            if (stereo) {
                if (angleR[j] >= 1.0) {
                    angleR[j] -= 1.0;
                }
            }
        }
        if (stereo) {
            fwrite(&sampleL, sizeof(sampleL), 1, file);
            fwrite(&sampleR, sizeof(sampleR), 1, file);
        } else {
            fwrite(&sampleL, sizeof(sampleL), 1, file);
        }
    }

    fclose(file);
    printf("i did it!\n");
    return 0;
}
