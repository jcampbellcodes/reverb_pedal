#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

static DaisySeed hw;

ReverbSc   verb;
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

enum class ADCChannel
{
    WetDryKnob = 0,
    ReverbTimeKnob,
    LPFKnob,
    NUM_CHANNELS
};

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

    // Create an array of two AdcChannelConfig objects
    AdcChannelConfig adc_config[static_cast<size_t>(ADCChannel::NUM_CHANNELS)];
    adc_config[static_cast<size_t>(ADCChannel::WetDryKnob)].InitSingle(daisy::seed::A6); 
    adc_config[static_cast<size_t>(ADCChannel::ReverbTimeKnob)].InitSingle(daisy::seed::A5);
    adc_config[static_cast<size_t>(ADCChannel::LPFKnob)].InitSingle(daisy::seed::A4);
    hw.adc.Init(adc_config, static_cast<size_t>(ADCChannel::NUM_CHANNELS));

    wetDryKnob.Init(hw.adc.GetPtr(0), sample_rate, false);
	reverbTimeKnob.Init(hw.adc.GetPtr(1), sample_rate, false);
    lpFreqKnob.Init(hw.adc.GetPtr(2), sample_rate, false);
	vtime.Init(reverbTimeKnob, 0.6f, 0.999f, Parameter::LOGARITHMIC);
    vsend.Init(wetDryKnob, 0.0f, 1.0f, Parameter::LINEAR);
	vfreq.Init(lpFreqKnob, 500.0f, 20000.0f, Parameter::LOGARITHMIC);

    //Start reading values
    hw.adc.Start();

    // start callback
    hw.StartAudio(AudioCallback);

	// Declare a variable to store the state we want to set for the LED.
    bool led_state;
    led_state = true;
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
