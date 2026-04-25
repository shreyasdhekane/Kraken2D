#pragma once
#include <SDL3/SDL.h>
#include <cmath>
#include <vector>
#include <cstdio>

class Audio {
public:
    static Audio& get() { static Audio a; return a; }

    bool init() {
        // Explicitly initialize audio subsystem
        if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
            SDL_Log("Audio init failed: %s", SDL_GetError());
            return false;
        }

        SDL_AudioSpec want{};
        want.format   = SDL_AUDIO_S16;   // S16 is more universally supported than F32
        want.channels = 1;
        want.freq     = 44100;

        m_stream = SDL_OpenAudioDeviceStream(
            SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
            &want, nullptr, nullptr);

        if (!m_stream) {
            SDL_Log("OpenAudioDeviceStream failed: %s", SDL_GetError());
            m_ok = false;
            return false;
        }

        SDL_ResumeAudioStreamDevice(m_stream);
        m_ok = true;
        SDL_Log("Audio OK: S16 44100Hz mono");
        return true;
    }

    // Generate and queue a tone
    // freq = Hz, duration = seconds, vol = 0..1, shape: 0=sine 1=square 2=noise
    void play(float freq, float duration, float vol = 0.4f, int shape = 0) {
        if (!m_ok || !m_stream) return;

        // Don't let queue get too large (prevents lag buildup)
        if (SDL_GetAudioStreamQueued(m_stream) > (int)(44100 * 2 * sizeof(Sint16)))
            return;

        int samples = (int)(44100.0f * duration);
        std::vector<Sint16> buf(samples);

        for (int i = 0; i < samples; i++) {
            float t   = (float)i / 44100.0f;
            // Envelope: fast attack, exponential decay
            float env = std::exp(-t * 5.0f / duration);
            float wave = 0;
            switch (shape) {
                case 0: wave = std::sin(2.0f * 3.14159f * freq * t); break;
                case 1: wave = (std::fmod(freq * t, 1.0f) < 0.5f) ? 1.f : -1.f; break;
                case 2: wave = ((rand() % 32767) / 16383.5f) - 1.0f; break; // noise
            }
            buf[i] = (Sint16)(wave * env * vol * 32767.0f);
        }

        SDL_PutAudioStreamData(m_stream,
            buf.data(), (int)(buf.size() * sizeof(Sint16)));
    }

    // ── Presets ────────────────────────────────────────────────
    void sfxShoot()    { play(800, 0.06f, 0.3f, 1); play(500, 0.05f, 0.2f, 1); }
    void sfxExplode()  { play(200, 0.08f, 0.5f, 2); play(100, 0.18f, 0.4f, 2); }
    void sfxHit()      { play(180, 0.10f, 0.5f, 1); }
    void sfxPaddle()   { play(520, 0.07f, 0.4f, 0); }
    void sfxWall()     { play(300, 0.05f, 0.3f, 0); }
    void sfxCoin()     { play(660, 0.06f, 0.3f, 0); play(880, 0.08f, 0.35f, 0); }
    void sfxScore()    { play(440, 0.06f, 0.3f, 0); play(550, 0.06f, 0.3f, 0);
                         play(660, 0.10f, 0.35f, 0); }
    void sfxLineClear(){ play(330, 0.05f, 0.3f, 0); play(440, 0.05f, 0.3f, 0);
                         play(550, 0.05f, 0.3f, 0); play(660, 0.12f, 0.4f, 0); }
    void sfxDie()      { play(300, 0.08f, 0.5f, 1); play(220, 0.10f, 0.5f, 1);
                         play(150, 0.18f, 0.5f, 1); }
    void sfxLevelUp()  { play(440, 0.07f, 0.3f, 0); play(550, 0.07f, 0.3f, 0);
                         play(660, 0.07f, 0.3f, 0); play(880, 0.14f, 0.4f, 0); }

    ~Audio() {
        if (m_stream) SDL_DestroyAudioStream(m_stream);
    }

private:
    Audio() = default;
    SDL_AudioStream* m_stream = nullptr;
    bool             m_ok     = false;
};
