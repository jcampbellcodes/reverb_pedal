#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

static DaisySeed hw;

ReverbSc   verb;
Oscillator osc;
AdEnv      env;
Metro      tick;

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    for(size_t i = 0; i < size; i += 2)
    {
        if(tick.Process())
        {
            env.Trigger();
        }

        float sig = env.Process() * osc.Process();
        verb.Process(sig, sig, &out[i], &out[i + 1]);
    }
}

int main(void)
{
    // initialize seed hardware and whitenoise daisysp module
    float sample_rate;
    hw.Configure();
    hw.Init();
    hw.SetAudioBlockSize(4);
    sample_rate = hw.AudioSampleRate();

    //setup reverb
    verb.Init(sample_rate);
    verb.SetFeedback(0.9f);
    verb.SetLpFreq(18000.0f);

    //setup metro
    tick.Init(1.f, sample_rate);

    //setup envelope
    env.Init(sample_rate);
    env.SetTime(ADENV_SEG_ATTACK, .1f);
    env.SetTime(ADENV_SEG_DECAY, .1f);
    env.SetMax(1.f);
    env.SetMin(0.f);
    env.SetCurve(0.f); //linear

    //setup oscillator
    osc.Init(sample_rate);
    osc.SetFreq(440.f);
    osc.SetWaveform(Oscillator::WAVE_TRI);

	// Declare a variable to store the state we want to set for the LED.
    bool led_state;
    led_state = true;

    // start callback
    hw.StartAudio(AudioCallback);
    for(;;)
    {
        // Set the onboard LED
        hw.SetLed(led_state);

        // Toggle the LED state for the next time around.
        led_state = !led_state;

        // Wait 500ms
        System::Delay(500);
    }
}
