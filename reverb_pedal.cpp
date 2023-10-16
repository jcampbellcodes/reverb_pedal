#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

static DaisySeed hw;

ReverbSc   verb;
Oscillator osc;
AdEnv      env;
Metro      tick;

// static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
//                           AudioHandle::InterleavingOutputBuffer out,
//                           size_t                                size)
// {
//     for(size_t i = 0; i < size; i += 2)
//     {
// 		float dry = 0.0f, send = 0.0f, wetl = 0.0f, wetr = 0.0f; // Effects Vars
// 		float *out_left = &out[i];
//         float *out_right = &out[i + 1];
//         // if(tick.Process())
//         // {
//         //     env.Trigger();
//         // }

//         // float sig = in[i]; //env.Process() * osc.Process();
//         // verb.Process(sig, sig, &out[i], &out[i + 1]);

//         // get dry sample from the state of the voices
//         dry  = in[i] * 0.5f; 
//         // run an attenuated dry signal through the reverb
//         send = dry * 0.45f;
//         verb.Process(send, send, &wetl, &wetr);
//         // sum the dry oscillator and processed reverb signal
//         out_left[i]  = dry + wetl;
//         out_right[i] = dry + wetr;
//     }
// }


// void AudioCallback(AudioHandle::InputBuffer  in,
//                    AudioHandle::OutputBuffer out,
//                    size_t                    size)
// {
// 	// Assign Output Buffers
//     float *out_left = out[0];
//     float *out_right = out[1];
//     float dry = 0.0f, send = 0.0f, wetl = 0.0f, wetr = 0.0f; // Effects Vars
// 	for(size_t i = 0; i < size; i += 2)
//     {
// 		// float dry = 0.0f, send = 0.0f, wetl = 0.0f, wetr = 0.0f; // Effects Vars
// 		// float *out_left = out[i];
//         // float *out_right = out[i + 1];
//         // if(tick.Process())
//         // {
//         //     env.Trigger();
//         // }

//         // float sig = in[i]; //env.Process() * osc.Process();
//         // verb.Process(sig, sig, &out[i], &out[i + 1]);

//         // get dry sample from the state of the voices
//         dry  = (*in[i]) * 0.5f; 
//         // run an attenuated dry signal through the reverb
//         send = dry * 0.45f;
//         verb.Process(send, send, &wetl, &wetr);
//         // sum the dry oscillator and processed reverb signal
//         out_left[i]  = dry + wetl;
//         out_right[i] = dry + wetr;
//     }
// }


void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
              AudioHandle::InterleavingOutputBuffer out,
              size_t                                size)
{
    float dryl, dryr, wetl, wetr, sendl, sendr;
    //hw.ProcessDigitalControls();
    // verb.SetFeedback(vtime.Process());
    // verb.SetLpFreq(vfreq.Process());
    // vsend.Process(); // Process Send to use later
    //bypass = hw.switches[DaisyPetal::SW_5].Pressed();
    // if(hw.switches[DaisyPetal::SW_1].RisingEdge())
    //     bypass = !bypass;
    for(size_t i = 0; i < size; i += 2)
    {
        dryl  = in[i];
        dryr  = in[i + 1];
        sendl = dryl * 0.45; //vsend.Value();
        sendr = dryr * 0.45; //vsend.Value();
        verb.Process(sendl, sendr, &wetl, &wetr);
        // if(bypass)
        // {
        //     out[i]     = in[i];     // left
        //     out[i + 1] = in[i + 1]; // right
        // }
        // else
        {
            out[i]     = dryl + wetl;
            out[i + 1] = dryr + wetr;
        }
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
