#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

static DaisySeed hw;

ReverbSc   verb;
Oscillator osc;
AdEnv      env;
Metro      tick;

Parameter vfreq, vtime, vsend;
AnalogControl wetDryKnob;
AnalogControl reverbTimeKnob;
AnalogControl lpFreqKnob;

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
              AudioHandle::InterleavingOutputBuffer out,
              size_t                                size)
{
	vsend.Process();
    float dryl, dryr, wetl, wetr, sendl, sendr;
    //hw.ProcessDigitalControls();
    verb.SetFeedback(vtime.Process());
    verb.SetLpFreq(vfreq.Process());
    vsend.Process(); // Process Send to use later
    //bypass = hw.switches[DaisyPetal::SW_5].Pressed();
    // if(hw.switches[DaisyPetal::SW_1].RisingEdge())
    //     bypass = !bypass;
    for(size_t i = 0; i < size; i += 2)
    {
        dryl  = in[i];
        dryr  = in[i + 1];
        sendl = dryl * vsend.Value();
        sendr = dryr * vsend.Value();
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

    // Create an array of two AdcChannelConfig objects
    const int num_adc_channels = 3;
    AdcChannelConfig adc_config[num_adc_channels];
    adc_config[0].InitSingle(hw.GetPin(21));
    adc_config[1].InitSingle(hw.GetPin(20));
    adc_config[2].InitSingle(hw.GetPin(19));
    hw.adc.Init(adc_config, num_adc_channels);

    wetDryKnob.Init(hw.adc.GetPtr(0), sample_rate, false);
	reverbTimeKnob.Init(hw.adc.GetPtr(1), sample_rate, false);
    lpFreqKnob.Init(hw.adc.GetPtr(2), sample_rate, false);
	vtime.Init(reverbTimeKnob, 0.6f, 0.999f, Parameter::LOGARITHMIC);
    vsend.Init(wetDryKnob, 0.0f, 1.0f, Parameter::LINEAR);
	vfreq.Init(lpFreqKnob, 500.0f, 20000.0f, Parameter::LOGARITHMIC);

    //Start reading values
    hw.adc.Start();

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
