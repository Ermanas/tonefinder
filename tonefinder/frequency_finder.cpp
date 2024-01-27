#include <iostream>
#include <cstring>
#include "portaudio.h"
#include "fftw3.h"

#define SAMPLE_RATE 48000
#define FRAMES_PER_BUFFER 16000
#define NUM_OF_CHANNELS 2
#define FREQ_START 0
#define FREQ_END 20000

class AudioDevice
{
private:
    PaError err;
    PaStream *stream;
    void checkErr();
    PaDeviceIndex device;
    PaStreamParameters inputParameters;
    PaStreamParameters outputParameters;
    static int streamCallBack(
        const void *inputBuffer,
        void *outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo *timeInfo,
        PaStreamCallbackFlags statusFlags,
        void *userData);
    typedef struct
    {
        double *inData;
        double *outData;
        fftw_plan myFFTPlan;
        int startIndex;
        int frequencySize;
    } streamCallbackData;
    static streamCallbackData *frequencyData;

public:
    AudioDevice();
    ~AudioDevice();
    void listDevices();
    void chooseDevice(PaDeviceIndex chosen_device);
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

void AudioDevice::chooseDevice(PaDeviceIndex chosen_device)
{
    this->device = chosen_device;

    // TO DO
    // Device availability control

    memset(&inputParameters, 0, sizeof(inputParameters));
    inputParameters.channelCount = 2;
    inputParameters.device = device;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(device)->defaultLowInputLatency;
    checkErr();

    memset(&outputParameters, 0, sizeof(outputParameters));
    outputParameters.channelCount = 2;
    outputParameters.device = device;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(device)->defaultLowOutputLatency;
    checkErr();
}

AudioDevice::streamCallbackData *AudioDevice::frequencyData = nullptr;

void AudioDevice::startStream(unsigned int duration_in_s)
{
    // Allocate the data to be used for the FFT process
    frequencyData = (streamCallbackData *)malloc(sizeof(streamCallbackData)); 
    frequencyData->inData = (double *)fftw_malloc(sizeof(double) * FRAMES_PER_BUFFER);
    frequencyData->outData = (double *)fftw_malloc(sizeof(double) * FRAMES_PER_BUFFER);
    if (frequencyData->inData == NULL || frequencyData->outData == NULL)
    {
        std::cout << "Could not allocate data for FFT process" << std::endl;
        exit(EXIT_FAILURE);
    }

    frequencyData->myFFTPlan = fftw_plan_r2r_1d(
        FRAMES_PER_BUFFER,
        frequencyData->inData,
        frequencyData->outData,
        FFTW_R2HC,
        FFTW_ESTIMATE);

    double sampleRatio = FRAMES_PER_BUFFER / SAMPLE_RATE;
    frequencyData->startIndex = sampleRatio * FREQ_START;
    frequencyData->frequencySize = std::min(sampleRatio * FREQ_END, FRAMES_PER_BUFFER / 2.0) - frequencyData->startIndex;

    err = Pa_OpenStream(
        &stream,
        &inputParameters,
        &outputParameters,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paNoFlag,
        this->streamCallBack,
        frequencyData);

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

    // FFT Process CleanUp
    fftw_destroy_plan(frequencyData->myFFTPlan);
    fftw_free(frequencyData->outData);
    fftw_free(frequencyData->inData);
    free(frequencyData);
}

int AudioDevice::streamCallBack(
    const void *inputBuffer,
    void *outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo *timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData)
{
    (void)outputBuffer;
    (void)timeInfo;
    (void)statusFlags;

    float *inBuff = (float *)inputBuffer;
    streamCallbackData *callbackData = (streamCallbackData *)userData;

    for (size_t i = 0; i < framesPerBuffer; i++)
    {
        callbackData->inData[i] = inBuff[i * NUM_OF_CHANNELS];
    }

    fftw_execute(callbackData->myFFTPlan);
    //fftw_execute_r2r(callbackData->myFFTPlan, callbackData->inData, callbackData->outData);

    double maxAmplitude = 0;
    int maxIndex = 0;

    // Find the index of the maximum amplitude in the positive frequency range
    for (size_t i = 0; i < framesPerBuffer / 2; i++)
    {
        double amplitude = std::abs(callbackData->outData[i]);
        if (amplitude > maxAmplitude)
        {
            maxAmplitude = amplitude;
            maxIndex = i;
        }
    }

    // Calculate the dominant frequency in Hz
    double dominantFrequency = static_cast<double>(maxIndex) * SAMPLE_RATE / FRAMES_PER_BUFFER;
    std::cout << "Dominant Frequency: " << dominantFrequency << " Hz" << std::endl;

    std::cout << std::flush;
    return paContinue;
}

int main()
{
    AudioDevice myAudioDevice1;
    std::cout << Pa_GetVersionText() << std::endl;
    std::cout << "Default Input Device: " << Pa_GetDefaultInputDevice() << std::endl;
    std::cout << "Default Output Device: " << Pa_GetDefaultOutputDevice() << "\n"
              << std::endl;
    myAudioDevice1.listDevices();

    PaDeviceIndex my_device;
    std::cout << "Choose a device to select: ";
    std::cin >> my_device;

    myAudioDevice1.chooseDevice(my_device);
    myAudioDevice1.startStream(15);
    myAudioDevice1.stopStream();

    return EXIT_SUCCESS;
}
