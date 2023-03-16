#include "mainenvironment.h"
#include "core.h"
#include "debugging/log.h"
#include "thirdparty/libsnd/sndfile.h"
#include "audio/audio.h"

sound_t snd;

sound_t minecraft;


struct sound_test
{
    SNDFILE* file;
    SF_INFO      info;
};

sound_test test;
sound_test test1;

bool on_try_close(void* data)
{
    Core::ShutDown();
    return true;
}

bool MainEnv::Create()
{
    if (!mywin.Create("mainwindow"))
    {
        Core::ShutDown();
    }
   // mywin.AppendChild(&tr);
   // mywin.AppendChild(&img);
    callback_t<bool> cb;
    cb.func = on_try_close;
    mywin.SetOnTryCloseCallback(cb);
    mywin.Open();
    //test.file = sf_open("sound/rockstar.wav", SFM_READ, &test.info);
    //snd.bitstream = (float*)malloc(sizeof(float) * test.info.channels * test.info.frames);
    //snd.size = test.info.frames;
    //snd.stereo = true;
    //snd.speed = 1.f;
    //snd.pos = 0;
    //snd.end_pos = 100000;
    //sf_read_float(test.file, snd.bitstream, test.info.channels * test.info.frames);
    //sf_close(test.file);
    test.file = sf_open("sound/shkola.wav", SFM_READ, &test.info);
    minecraft.bitstream = (float*)malloc(sizeof(float) * test.info.channels * test.info.frames);
    minecraft.size = test.info.frames*2;
    minecraft.stereo = true;
    minecraft.volume = 1;
    minecraft.speed = 1.f;
    minecraft.pos = 0;
    sf_read_float(test.file, minecraft.bitstream, test.info.channels * test.info.frames);
    sf_close(test.file);
    AudioMixer::Play(&minecraft);
    //AudioMixer::Play(&snd);
    return true;
}

bool MainEnv::Destroy()
{
    mywin.Destroy();
    return true;
}

UString MainEnv::GetName()
{
    return "Main";
}

bool MainEnv::AcceptData(int id, BaseEnvironment* sender, void* data)
{
    return false;
}

bool MainEnv::RequestControl()
{
    return false;
}

void MainEnv::SleepLoop()
{
}

void MainEnv::MainLoop()
{
    Sleep(1);
}
