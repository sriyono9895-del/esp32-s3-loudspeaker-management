#pragma once

#include <stdint.h>
#include <Arduino.h>

struct Biquad {
    float b0 = 1.0f;
    float b1 = 0.0f;
    float b2 = 0.0f;
    float a1 = 0.0f;
    float a2 = 0.0f;
    float z1 = 0.0f;
    float z2 = 0.0f;

    void reset();
    float process(float input);
    void setLowPass(float sampleRate, float cutoffHz, float q);
    void setHighPass(float sampleRate, float cutoffHz, float q);
    void setBandPass(float sampleRate, float lowHz, float highHz, float q);
    void setPeaking(float sampleRate, float centerHz, float gainDb, float q);
};

class AudioDSP {
public:
    AudioDSP();

    void begin(uint32_t sampleRate = AUDIO_SAMPLE_RATE);
    void setInputGain(float gainDb);
    void setOutputGain(float gainDb);
    void setEqBand(uint8_t bandIndex, float gainDb);
    void setCrossover(float lowHz, float midLowHz, float midHighHz, float highHz);
    void setLimiterThreshold(float thresholdDbFS);
    void setPhaseInvert(bool leftInvert, bool rightInvert);
    void processFrame(float& left, float& right);

private:
    void updateEqFilters();
    void updateCrossoverFilters();
    void applyLimiter(float& left, float& right);
    static float dbToLinear(float gainDb);

    uint32_t sampleRate_;
    float inputGainDb_;
    float outputGainDb_;
    float eqBandGainDb_[AUDIO_EQ_BANDS];
    float crossoverLowHz_;
    float crossoverMidLowHz_;
    float crossoverMidHighHz_;
    float crossoverHighHz_;
    float limiterThresholdDbFS_;
    bool phaseInvertLeft_;
    bool phaseInvertRight_;

    Biquad eqFilters_[AUDIO_EQ_BANDS][AUDIO_OUTPUT_CHANNELS];
    Biquad lowPass_[AUDIO_OUTPUT_CHANNELS];
    Biquad bandPass_[AUDIO_OUTPUT_CHANNELS];
    Biquad highPass_[AUDIO_OUTPUT_CHANNELS];
};
