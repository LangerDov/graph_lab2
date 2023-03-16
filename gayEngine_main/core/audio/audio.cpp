#include "audio.h"
#include "debugging/log.h"
#include <portaudio/portaudio.h>
#include "thread/sync.h"

#include "list.h"
#include "fast_math.h"

#define BUFFER_SIZE 512
#ifdef _WIN32

static PaStream* stream;
static PaStreamParameters outputParameters;
#else
#endif
static Mutex main_stream_mutex;

List< sound_t*>* buffers;

float volume = 0;
#ifdef _WIN32

static int paCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData)
{
    float* out = (float*)outputBuffer;
    (void)timeInfo; /* Prevent unused variable warnings. */
    (void)statusFlags;
    (void)inputBuffer;
    main_stream_mutex.Enter();
    unsigned long frames_to_cp;
    sound_t* snd;
    float static buffer[BUFFER_SIZE*2];
    float static buffer_temp[BUFFER_SIZE * 2];
    int offset;
    memset(buffer, 0, sizeof(float) * BUFFER_SIZE*2);
    for (size_t i = 0; i < buffers->Size(); i++)
    {
        offset = 0;
        snd = buffers->El(i);
        if (snd->stereo)
        {
            if (snd->pos + BUFFER_SIZE * 2 > snd->size)
            {
                frames_to_cp = snd->pos + BUFFER_SIZE * 2- snd->size;
            }
            else frames_to_cp = BUFFER_SIZE*2;
            float j = 0;
            int real_offset = 0;
            for (; real_offset < BUFFER_SIZE *2; real_offset++)
            {
                int test = snd->pos + static_cast<int>(j);
                if (j > frames_to_cp or j == snd->end_pos or test < 0)
                {
                    j -= snd->speed;
                    break;
                }
                buffer[real_offset] += snd->bitstream[snd->pos+ static_cast<int>(j)];
                j += snd->speed;
            }
            if (j == snd->end_pos)
            {
                if (snd->finish_callback)
                    snd->finish_callback(snd->callback_data);
                buffers->Remove(i);
                i--;
            }
            if (frames_to_cp != framesPerBuffer * 2)
                buffers->El(i)->pos = 0;
            else
                snd->pos += frames_to_cp * j/ real_offset;//static_cast<int>(snd->speed);
        }
        else
        {

        }
    }
    memcpy(out, buffer, BUFFER_SIZE * 2 * sizeof(float));
    main_stream_mutex.Leave();
    return paContinue;
}
#else
#endif
bool AudioMixer::SelectOutputDevice(AudioDevice)
{
    return false;
}

bool AudioMixer::Play(sound_t* snd)
{
    main_stream_mutex.Enter();
    //buffers->Add(snd);
    main_stream_mutex.Leave();
    return false;
}

bool AudioMixer::Stop(sound_t*)
{
    main_stream_mutex.Enter();
    main_stream_mutex.Leave();
    return false;
}

List<AudioDevice> *AudioMixer::GetOutputDevices()
{
#ifdef _WIN32
    PaDeviceIndex device_count = Pa_GetDeviceCount();
    List<AudioDevice> *res = new List<AudioDevice>();
    for (size_t i = 0; i < device_count; i++)
    {
        if (Pa_GetDeviceInfo(i)->maxOutputChannels == 0)
            continue;
        AudioDevice device;
        device.name = Pa_GetDeviceInfo(i)->name;
        device.data = Pa_GetDeviceInfo(i);
        res->Add(device);
    }
    return res;
#else
    return 0;
#endif
}

void AudioMixer::SetVolume(float vol)
{
    volume = vol;
}

bool AudioMixer::Init()
{
#ifdef _WIN32

    buffers = new List< sound_t*>();
    PaError error = Pa_Initialize(); 
    if (error != paNoError)
    {
        Log(lErr,"PortAudio cannot be initialized. PortAudio error: %s", Pa_GetErrorText(error));
        return false;
    }
    Log(lInfo, "PortAudio version: %s ",Pa_GetVersionText());
    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) {
        Log(lErr, "No default output device.");
    }
    else
        Log(lInfo, "Selected default output device: %s Channels: %d SampleRate: %f", Pa_GetDeviceInfo(outputParameters.device)->name, Pa_GetDeviceInfo(outputParameters.device)->maxOutputChannels, Pa_GetDeviceInfo(outputParameters.device)->defaultSampleRate);
    outputParameters.channelCount = 2;       /* stereo output */
    outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    error = Pa_OpenStream(
        &stream,
        NULL,
        &outputParameters,
        Pa_GetDeviceInfo(outputParameters.device)->defaultSampleRate,
        BUFFER_SIZE,
        NULL,
        paCallback,
        NULL);
        if (error != paNoError)
        {
            Log(lErr, "PortAudio error: %s", Pa_GetErrorText(error));
            return false;
        }
        main_stream_mutex.Init(false);
        Pa_StartStream(stream);
    return true;
#else
    return 0;
#endif
}

bool AudioMixer::Destroy()
{
#ifdef _WIN32

    PaError error = Pa_Terminate();
    if (error != paNoError)
    {
        Log(lErr, "PortAudio cannot be terminated. PortAudio error: %s", Pa_GetErrorText(error));
        return false;
    }
	delete buffers;
#else
    return 0;
#endif
    return true;
}

UString AudioDevice::GetName()
{
    return name;
}

device_type_t AudioDevice::GetType()
{
    return type;
}
