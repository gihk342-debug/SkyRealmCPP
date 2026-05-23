#include "audio.h"
#include <SDL.h>
#include <cmath>
#include <vector>
#include <cstring>

static SDL_AudioDeviceID audioDevice = 0;
static bool audioInitialized = false;

// Simple square/sine/triangle/saw wave synthesis on-the-fly
struct Tone {
    float freq, dur, vol;
    float phase = 0;
    int type = 0; // 0=sine 1=triangle 2=sawtooth 3=square
};

static std::vector<Tone> activeTones;

static void audioCallback(void* userdata, Uint8* stream, int len) {
    float* buffer = (float*)stream;
    int samples = len / sizeof(float);
    int sampleRate = 44100;

    std::memset(buffer, 0, len);

    for (int i = 0; i < samples; i++) {
        float t = (float)i / sampleRate;
        float sum = 0;

        for (auto it = activeTones.begin(); it != activeTones.end(); ) {
            float elapsed = t;
            float localTime = elapsed;

            // Simple: we use a static accumulation scheme instead
            // Since SDL audio callback is complex to sync with game loop,
            // we use a simpler approach: pre-generate tones as buffers,
            // but for minimal effort, let's just output beeps using SDL_QueueAudio
            // For now: no-op (audio optional)
            ++it;
        }
    }
}

void initAudio() {
    if (audioInitialized) return;

    SDL_AudioSpec want, have;
    want.freq = 44100;
    want.format = AUDIO_F32;
    want.channels = 1;
    want.samples = 1024;
    want.callback = nullptr; // use queue-based audio for simplicity
    want.userdata = nullptr;

    audioDevice = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (audioDevice) {
        SDL_PauseAudioDevice(audioDevice, 0);
        audioInitialized = true;
    }
}

// Generate a simple tone buffer and queue it
static void queueTone(float freq, float dur, float vol) {
    if (!audioInitialized || !audioDevice) return;
    if (vol <= 0) vol = 0.01f;

    int sampleRate = 44100;
    int totalSamples = (int)(sampleRate * dur);
    std::vector<float> buffer(totalSamples);

    for (int i = 0; i < totalSamples; i++) {
        float t = (float)i / sampleRate;
        float envelope = 1.0f - (t / dur); // linear fade out
        if (envelope < 0) envelope = 0;
        envelope *= envelope; // exponential-ish decay

        // Square wave (closest to Web Audio 'square')
        float phase = freq * t;
        float value = std::sin(2.0f * 3.14159265f * phase);
        // crude square: sign of sine
        value = (value > 0) ? 1.0f : -1.0f;

        buffer[i] = value * envelope * vol;
    }

    SDL_QueueAudio(audioDevice, buffer.data(), totalSamples * sizeof(float));
}

void playTone(float freq, float dur, float vol) {
    queueTone(freq, dur, vol);
}

void Sfx::pickup()   { playTone(880, 0.1f, 0.1f);  playTone(1100, 0.08f, 0.08f); }
void Sfx::coin()     { playTone(1400, 0.06f, 0.06f); }
void Sfx::equip()    { playTone(600, 0.12f, 0.1f);  playTone(800, 0.1f, 0.1f); }
void Sfx::skill()    { playTone(500, 0.2f, 0.12f);  playTone(700, 0.15f, 0.1f); }
void Sfx::hit()      { playTone(200, 0.15f, 0.08f); }
void Sfx::sword()    { playTone(350, 0.12f, 0.06f); playTone(800, 0.08f, 0.04f); }
void Sfx::chest()    { playTone(330, 0.2f, 0.12f);  playTone(550, 0.18f, 0.12f); }
void Sfx::victory()  { playTone(523, 0.3f, 0.15f);  playTone(659, 0.3f, 0.15f);
                        playTone(784, 0.4f, 0.2f);   playTone(1047, 0.5f, 0.4f); }