#include <iostream>
#include <cstring>
#include "portaudio.h"

#define SAMPLE_RATE 48000
#define FRAMES_PER_BUFFER 512

static void checkErr(PaError err)
{
    if (err != paNoError)
    {
        std::cout << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        exit(EXIT_FAILURE);
    }
}

static inline float max(float a, float b) { return a > b ? a : b; }

static int patestCallback(
    const void *inputBuffer,
    void *outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo *timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData)
{
    float *in = (float *)inputBuffer;
    (void)outputBuffer;

    int displaySize = 100;
    std::cout << "\r";

    float vol_r = 0, vol_l = 0;

    // the input buffer carries 512 frames per channel
    for (unsigned int i = 0; i < framesPerBuffer * 2; i += 2)
    {
        vol_l = max(vol_l, std::abs(in[i]));
        vol_r = max(vol_r, std::abs(in[i + 1]));
    }

    float display_vol_l = vol_l * static_cast<float>(displaySize);
    float display_vol_r = vol_r * static_cast<float>(displaySize);

    // In either way, we have to go to the end of our displaySize
    // First one calculates the total persentagef first, them it prints.
    // Second one creates a treshold for every bar in the displaySize.
    for (int i = 0; i < displaySize; i++)
    {
        if (display_vol_l >= i && display_vol_r >= i)
            std::cout << "█";
        else if (display_vol_l >= i)
            std::cout << "▀";
        else if (display_vol_r >= i)
            std::cout << "▄";
        else
            std::cout << " ";
    }

    // Second Method for Display
    // ---
    // for (int i = 0; i < displaySize; i++)
    // {
    //     float barProportion = i / static_cast<float>(displaySize);
    //     if (barProportion <= vol_l && barProportion <= vol_r)
    //         std::cout << "█";
    //     else if (barProportion <= vol_l)
    //         std::cout << "▀";
    //     else if (barProportion <= vol_r)
    //         std::cout << "▄";
    //     else
    //     {
    //         std::cout << " ";
    //     }
    // }

    std::cout << std::flush;
    return 0;
}

int main()
{
    PaError err;
    err = Pa_Initialize();

    int numDevices = Pa_GetDeviceCount();
    std::cout << "Number of devices: " << numDevices << std::endl;

    if (numDevices < 0)
    {
        std::cout << "Error getting the device count" << std::endl;
        exit(EXIT_FAILURE);
    }
    else if (numDevices == 0)
    {
        std::cout << "No devices found." << std::endl;
        exit(EXIT_SUCCESS);
    }

    const PaDeviceInfo *deviceInfo;

    for (int i = 0; i < numDevices; i++)
    {
        deviceInfo = Pa_GetDeviceInfo(i);
        if (deviceInfo->maxInputChannels > 0 && deviceInfo->maxOutputChannels > 0)
        {
            std::cout << "Device " << i << ":" << std::endl;
            std::cout << "Name: " << deviceInfo->name << std::endl;
            std::cout << "Max Input Channels: " << deviceInfo->maxInputChannels << std::endl;
            std::cout << "Max Output Channels: " << deviceInfo->maxOutputChannels << std::endl;
            std::cout << "Default Sample Rate: " << deviceInfo->defaultSampleRate << std::endl;
            std::cout << std::endl;
        }
    }

    int device = 4;

    std::cout << "Please choose the device you wanted: ";
    std::cin >> device;

    PaStreamParameters inputParameters;
    PaStreamParameters outputParameters;

    memset(&inputParameters, 0, sizeof(inputParameters));
    inputParameters.channelCount = 2;
    inputParameters.device = device;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(device)->defaultLowInputLatency;

    memset(&outputParameters, 0, sizeof(outputParameters));
    outputParameters.channelCount = 2;
    outputParameters.device = device;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(device)->defaultLowOutputLatency;

    PaStream *stream;

    err = Pa_OpenStream(
        &stream,
        &inputParameters,
        &outputParameters,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paNoFlag,
        patestCallback,
        NULL);


    checkErr(err);

    err = Pa_StartStream(stream);
    checkErr(err);

    Pa_Sleep(10 * 1000);

    err = Pa_StopStream(stream);
    checkErr(err);

    err = Pa_CloseStream(stream);
    checkErr(err);

    err = Pa_Terminate();
    std::cout << std::endl;
    checkErr(err);
    return EXIT_SUCCESS;
}