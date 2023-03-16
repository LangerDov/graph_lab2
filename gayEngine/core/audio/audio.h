#pragma once
#include "str.h"
#include "list.h"

struct sound_t
{
	float* bitstream;
	size_t size;
	bool stereo;
	float volume = 1;
	size_t end_pos = 0xFFFFFFFF;
	size_t samplerate;
	void* callback_data = nullptr;
	void(*finish_callback)(void*) = nullptr;
	size_t pos = 0;
	float speed = 1;
};

#define OUTPUT_DEVICE 1
#define INPUT_DEVICE 0

typedef size_t device_type_t;

class AudioDevice
{
	friend class AudioMixer;
public:
	UString GetName();
	device_type_t GetType();
private:
	const void* data;
	const char* name;
	device_type_t type;
};

class AudioMixer
{
	friend class Core;
public:
	static bool SelectOutputDevice(AudioDevice);
	static bool Play(sound_t*);
	static bool Stop(sound_t*);
	static List<AudioDevice> *GetOutputDevices();
	static void SetVolume(float);
private:
	static bool Init();
	static bool Destroy();
};
