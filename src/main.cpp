#include "audio_dsp.h"
#include "config.h"

static float clampf(float value, float minValue, float maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

void Biquad::reset() {
    z1 = 0.0f;
    z2 = 0.0f;
}

float Biquad::process(float input) {
    const float output = b0 * input + b1 * z1 + b2 * z2 - a1 * z1 - a2 * z2;
    z2 = z1;
    z1 = input;
    return output;
}

void Biquad::setLowPass(float sampleRate, float cutoffHz, float q) {
    const float wa = (2.0f * PI * cutoffHz) / sampleRate;
    const float sinw = sinf(wa);
    const float cosw = cosf(wa);
    const float alpha = sinw / (2.0f * q);
    const float b0_ = (1.0f - cosw) / 2.0f;
    const float b1_ = 1.0f - cosw;
    const float b2_ = (1.0f - cosw) / 2.0f;
    const float a0_ = 1.0f + alpha;
    const float a1_ = -2.0f * cosw;
    const float a2_ = 1.0f - alpha;
    b0 = b0_ / a0_;
    b1 = b1_ / a0_;
    b2 = b2_ / a0_;
    a1 = a1_ / a0_;
    a2 = a2_ / a0_;
    reset();
}

void Biquad::setHighPass(float sampleRate, float cutoffHz, float q) {
    const float wa = (2.0f * PI * cutoffHz) / sampleRate;
    const float sinw = sinf(wa);
    const float cosw = cosf(wa);
    const float alpha = sinw / (2.0f * q);
    const float b0_ = (1.0f + cosw) / 2.0f;
    const float b1_ = -(1.0f + cosw);
    const float b2_ = (1.0f + cosw) / 2.0f;
    const float a0_ = 1.0f + alpha;
    const float a1_ = -2.0f * cosw;
    const float a2_ = 1.0f - alpha;
    b0 = b0_ / a0_;
    b1 = b1_ / a0_;
    b2 = b2_ / a0_;
    a1 = a1_ / a0_;
    a2 = a2_ / a0_;
    reset();
}

void Biquad::setBandPass(float sampleRate, float lowHz, float highHz, float q) {
    const float center = sqrtf(lowHz * highHz);
    const float wa = (2.0f * PI * center) / sampleRate;
    const float sinw = sinf(wa);
    const float cosw = cosf(wa);
    const float alpha = sinw / (2.0f * q);
    const float beta = sqrtf(1.0f / q);
    (void)beta;
    const float b0_ = alpha;
    const float b1_ = 0.0f;
    const float b2_ = -alpha;
    const float a0_ = 1.0f + alpha;
    const float a1_ = -2.0f * cosw;
    const float a2_ = 1.0f - alpha;
    b0 = b0_ / a0_;
    b1 = b1_ / a0_;
    b2 = b2_ / a0_;
    a1 = a1_ / a0_;
    a2 = a2_ / a0_;
    reset();
}

void Biquad::setPeaking(float sampleRate, float centerHz, float gainDb, float q) {
    const float gain = powf(10.0f, gainDb / 40.0f);
    const float wa = (2.0f * PI * centerHz) / sampleRate;
    const float sinw = sinf(wa);
    const float cosw = cosf(wa);
    const float alpha = sinw / (2.0f * q);
    const float b0_ = 1.0f + (alpha * gain);
    const float b1_ = -2.0f * cosw;
    const float b2_ = 1.0f - (alpha * gain);
    const float a0_ = 1.0f + (alpha / gain);
    const float a1_ = -2.0f * cosw;
    const float a2_ = 1.0f - (alpha / gain);

    b0 = b0_ / a0_;
    b1 = b1_ / a0_;
    b2 = b2_ / a0_;
    a1 = a1_ / a0_;
    a2 = a2_ / a0_;
    reset();
}

AudioDSP::AudioDSP()
    : sampleRate_(AUDIO_SAMPLE_RATE),
      inputGainDb_(0.0f),
      outputGainDb_(0.0f),
      crossoverLowHz_(AUDIO_CROSSOVER_LOW_HZ),
      crossoverMidLowHz_(AUDIO_CROSSOVER_MID_LOW_HZ),
      crossoverMidHighHz_(AUDIO_CROSSOVER_MID_HIGH_HZ),
      crossoverHighHz_(AUDIO_CROSSOVER_HIGH_HZ),
      limiterThresholdDbFS_(AUDIO_LIMITER_THRESHOLD_DBFS),
      phaseInvertLeft_(false),
      phaseInvertRight_(false) {
    for (int i = 0; i < AUDIO_EQ_BANDS; ++i) {
        eqBandGainDb_[i] = 0.0f;
    }
}

void AudioDSP::begin(uint32_t sampleRate) {
    sampleRate_ = sampleRate;
    updateEqFilters();
    updateCrossoverFilters();
}

void AudioDSP::setInputGain(float gainDb) {
    inputGainDb_ = gainDb;
}

void AudioDSP::setOutputGain(float gainDb) {
    outputGainDb_ = gainDb;
}

void AudioDSP::setEqBand(uint8_t bandIndex, float gainDb) {
    if (bandIndex >= AUDIO_EQ_BANDS) return;
    eqBandGainDb_[bandIndex] = gainDb;
    updateEqFilters();
}

void AudioDSP::setCrossover(float lowHz, float midLowHz, float midHighHz, float highHz) {
    crossoverLowHz_ = lowHz;
    crossoverMidLowHz_ = midLowHz;
    crossoverMidHighHz_ = midHighHz;
    crossoverHighHz_ = highHz;
    updateCrossoverFilters();
}

void AudioDSP::setLimiterThreshold(float thresholdDbFS) {
    limiterThresholdDbFS_ = thresholdDbFS;
}

void AudioDSP::setPhaseInvert(bool leftInvert, bool rightInvert) {
    phaseInvertLeft_ = leftInvert;
    phaseInvertRight_ = rightInvert;
}

float AudioDSP::dbToLinear(float gainDb) {
    return powf(10.0f, gainDb / 20.0f);
}

void AudioDSP::updateEqFilters() {
    for (int band = 0; band < AUDIO_EQ_BANDS; ++band) {
        const float center = AUDIO_EQ_FREQUENCIES[band];
        for (int ch = 0; ch < AUDIO_OUTPUT_CHANNELS; ++ch) {
            eqFilters_[band][ch].setPeaking(sampleRate_, center, eqBandGainDb_[band], 0.707f);
        }
    }
}

void AudioDSP::updateCrossoverFilters() {
    const float q = 0.707f;
    for (int ch = 0; ch < AUDIO_OUTPUT_CHANNELS; ++ch) {
        lowPass_[ch].setLowPass(sampleRate_, crossoverLowHz_, q);
        bandPass_[ch].setBandPass(sampleRate_, crossoverMidLowHz_, crossoverMidHighHz_, q);
        highPass_[ch].setHighPass(sampleRate_, crossoverHighHz_, q);
    }
}

void AudioDSP::applyLimiter(float& left, float& right) {
    const float thresholdLinear = dbToLinear(limiterThresholdDbFS_);
    float peak = fabsf(left);
    if (fabsf(right) > peak) peak = fabsf(right);

    if (peak > thresholdLinear) {
        const float gain = thresholdLinear / peak;
        left *= gain;
        right *= gain;
    }
}

void AudioDSP::processFrame(float& left, float& right) {
    float l = left * dbToLinear(inputGainDb_);
    float r = right * dbToLinear(inputGainDb_);

    for (int band = 0; band < AUDIO_EQ_BANDS; ++band) {
        l = eqFilters_[band][0].process(l);
        r = eqFilters_[band][1].process(r);
    }

    const float lowL = lowPass_[0].process(l);
    const float lowR = lowPass_[1].process(r);
    const float midL = bandPass_[0].process(l);
    const float midR = bandPass_[1].process(r);
    const float highL = highPass_[0].process(l);
    const float highR = highPass_[1].process(r);

    float outL = lowL + midL + highL;
    float outR = lowR + midR + highR;

    applyLimiter(outL, outR);

    if (phaseInvertLeft_) outL = -outL;
    if (phaseInvertRight_) outR = -outR;

    outL *= dbToLinear(outputGainDb_);
    outR *= dbToLinear(outputGainDb_);

    left = clampf(outL, -1.0f, 1.0f);
    right = clampf(outR, -1.0f, 1.0f);
}
