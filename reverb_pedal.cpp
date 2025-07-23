#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisysp;
using namespace daisy;

// Uncomment the following line to enable peripherals (knobs, switches, etc.)
// #define USE_PERIPHERALS

static DaisySeed hw;

ReverbSc verb;

#ifdef USE_PERIPHERALS
Parameter vfreq, vtime, vsend;
AnalogControl wetDryKnob;
AnalogControl reverbTimeKnob;
AnalogControl lpFreqKnob;
Switch bypassSwitch;
dsy_gpio bypassLed;
bool isBypassed = false;  // initial bypass state
#else
float vtime_value = 0.8f;  // Hardcoded reverb time
float vsend_value = 0.5f;  // Hardcoded wet/dry mix
float vfreq_value = 10000.0f; // Hardcoded low-pass filter frequency
bool isBypassed = false;  // Hardcoded bypass state
#endif

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
#ifdef USE_PERIPHERALS
    vsend.Process();
    float dryl, dryr, wetl, wetr, sendl, sendr;
    verb.SetFeedback(vtime.Process());
    verb.SetLpFreq(vfreq.Process());
#else
    verb.SetFeedback(vtime_value);
    verb.SetLpFreq(vfreq_value);
#endif

    for(size_t i = 0; i < size; i++)
    {
        float dryl = in[0][i];
        float dryr = in[1][i];
#ifdef USE_PERIPHERALS
        float sendl = dryl * vsend.Value();
        float sendr = dryr * vsend.Value();
#else
        float sendl = dryl * vsend_value;
        float sendr = dryr * vsend_value;
#endif
        float wetl, wetr;
        verb.Process(sendl, sendr, &wetl, &wetr);
#ifdef USE_PERIPHERALS
        if(isBypassed)
#else
        if(isBypassed)
#endif
        {
            out[0][i] = dryl; // left
            out[1][i] = dryr; // right
        }
        else
        {
            out[0][i] = dryl + wetl;
            out[1][i] = dryr + wetr;
        }
    }
}

int main(void)
{
    // Initialize seed hardware
    hw.Configure();
    hw.Init();
    hw.SetAudioBlockSize(48); // Set block size to 48 samples
    float sample_rate = hw.AudioSampleRate();

    // Setup reverb
    verb.Init(sample_rate);

#ifdef USE_PERIPHERALS
    // Configure ADC channels
    AdcChannelConfig adc_config[3];
    adc_config[0].InitSingle(seed::A6);
    adc_config[1].InitSingle(seed::A5);
    adc_config[2].InitSingle(seed::A4);
    hw.adc.Init(adc_config, 3);

    // Initialize knobs
    wetDryKnob.Init(hw.adc.GetPtr(0), sample_rate, false);
    reverbTimeKnob.Init(hw.adc.GetPtr(1), sample_rate, false);
    lpFreqKnob.Init(hw.adc.GetPtr(2), sample_rate, false);
    vtime.Init(reverbTimeKnob, 0.6f, 0.999f, Parameter::LOGARITHMIC);
    vsend.Init(wetDryKnob, 0.0f, 1.0f, Parameter::LINEAR);
    vfreq.Init(lpFreqKnob, 500.0f, 20000.0f, Parameter::LOGARITHMIC);

    // Initialize bypass switch and LED
    bypassSwitch.Init(seed::D7, 1000);
    bypassLed.pin = seed::D6;
    bypassLed.mode = DSY_GPIO_MODE_OUTPUT_PP;
    bypassLed.pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&bypassLed);

    // Start ADC and audio
    hw.adc.Start();
#else
    // Hardcoded bypass state
    isBypassed = false;
#endif

    hw.StartAudio(AudioCallback);

    // Main loop
    while(1)
    {
#ifdef USE_PERIPHERALS
        bypassSwitch.Debounce();
        // Handle bypass logic in the main loop
        if(bypassSwitch.RisingEdge())
        {
            // Toggle the bypass state
            isBypassed = !isBypassed;
            dsy_gpio_write(&bypassLed, !isBypassed);
            hw.SetLed(isBypassed);
        }
        hw.DelayMs(10);
#else
        // No peripherals, just delay to keep the loop running
        hw.DelayMs(10);
#endif
    }
}
