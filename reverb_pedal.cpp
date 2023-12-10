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
bool isBypassed = true;

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
              AudioHandle::InterleavingOutputBuffer out,
              size_t                                size)
{
	vsend.Process();
    float dryl, dryr, wetl, wetr, sendl, sendr;
    verb.SetFeedback(vtime.Process());
    verb.SetLpFreq(vfreq.Process());
    vsend.Process(); // Process Send to use later
    for(size_t i = 0; i < size; i += 2)
    {
        dryl  = in[i];
        dryr  = in[i];
        sendl = dryl * vsend.Value();
        sendr = dryr * vsend.Value();
        verb.Process(sendl, sendr, &wetl, &wetr);
        if(isBypassed)
        {
            out[i]     = in[i];     // left
            out[i + 1] = in[i]; // right
        }
        else
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
    hw.SetAudioBlockSize(1);
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


    // Create our GPIO object
    Switch bypassSwitch;
    GPIO bypassLed;

    // Initialize the GPIO object for our button */
    bypassSwitch.Init(daisy::seed::D7, 1000);

    // Initialize the GPIO object for our LED
    bypassLed.Init(daisy::seed::D6, GPIO::Mode::OUTPUT);

    //Start reading values
    hw.adc.Start();

    // start callback
    hw.StartAudio(AudioCallback);

    bool lastBypass = isBypassed;
    hw.SetLed(isBypassed);
    bypassLed.Write(!isBypassed);
    while (1)
    {
        bypassSwitch.Debounce();
        // Handle bypass logic in the main loop
        if (bypassSwitch.RisingEdge())
        {
            // Toggle the bypass state
            isBypassed = !isBypassed;
            bypassLed.Write(!isBypassed);
            hw.SetLed(isBypassed);
        }
        // Your other non-audio-related code here
        hw.DelayMs(10);
    }
}
