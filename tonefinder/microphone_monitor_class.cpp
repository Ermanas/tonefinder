#include <iostream>
#include <cstring>
#include "portaudio.h"

#define SAMPLE_RATE 48000.0
#define FRAMES_PER_BUFFER 512
#define NUM_OF_CHANNELS 2

class AudioDevice
{
private:
    PaError err;
    PaStream *stream;
    void checkErr();
    unsigned int device;
    PaStreamParameters inputParameters;
    PaStreamParameters outputParameters;
    static int streamCallBack(
        const void *inputBuffer,
        void *outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo *timeInfo,
        PaStreamCallbackFlags statusFlags,
        void *userData);

public:
    AudioDevice();
    ~AudioDevice();
    void listDevices();
    void chooseDevice(unsigned int chosen_device);
    void startStream(unsigned int duration_in_s);
    void stopStream();
};

AudioDevice::AudioDevice()
{
    err = Pa_Initialize();
}

AudioDevice::~AudioDevice()
{
    err = Pa_Terminate();
    checkErr();
}

void AudioDevice::checkErr()
{
    if (err != paNoError)
    {
        std::cout << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        exit(EXIT_FAILURE);
    }
}

void AudioDevice::listDevices()
{
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
}

void AudioDevice::chooseDevice(unsigned int chosen_device)
{
    this->device = chosen_device;

    memset(&inputParameters, 0, sizeof(inputParameters));
    inputParameters.channelCount = NUM_OF_CHANNELS;
    inputParameters.device = device;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(device)->defaultLowInputLatency;
    checkErr();
}

void AudioDevice::startStream(unsigned int duration_in_s)
{
    err = Pa_OpenStream(
        &stream,
        &inputParameters,
        NULL,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paNoFlag,
        this->streamCallBack,
        NULL);

    checkErr();

    err = Pa_StartStream(stream);
    checkErr();

    Pa_Sleep(duration_in_s * 1000);
}

void AudioDevice::stopStream()
{
    err = Pa_StopStream(stream);
    checkErr();

    err = Pa_CloseStream(stream);
    checkErr();
    std::cout << std::endl;
}

int AudioDevice::streamCallBack(
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
        vol_l = std::max(vol_l, std::abs(in[i]));
        vol_r = std::max(vol_r, std::abs(in[i + 1]));
    }

    float display_vol_l = vol_l * static_cast<float>(displaySize);
    float display_vol_r = vol_r * static_cast<float>(displaySize);

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

    std::cout << std::flush;
    return 0;
}

int main()
{
    AudioDevice myAudioDevice1;
    myAudioDevice1.listDevices();

    int my_device;
    std::cout << "Choose a device to select: ";
    std::cin >> my_device;

    myAudioDevice1.chooseDevice(my_device);
    myAudioDevice1.startStream(10);
    myAudioDevice1.stopStream();

    return EXIT_SUCCESS;
}
