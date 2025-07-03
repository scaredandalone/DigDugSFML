#pragma once
#include <SFML/Audio.hpp>
#include <memory>
#include <string>

class SFX
{
public:
    enum class Type {
        SOUND,
        MUSIC
    };

private:
    Type type;

    // For sound effects
    std::unique_ptr<sf::SoundBuffer> soundBuffer;
    std::unique_ptr<sf::Sound> sound;

    // For music
    std::unique_ptr<sf::Music> music;

public:
    // Constructor
    SFX(const std::string& filename, Type audioType = Type::SOUND);

    // Playback control
    void play();
    void pause();
    void stop();

    // Status checking
    bool isPlaying() const;

    // Volume control
    void setVolume(float volume);
    float getVolume() const;

    // Loop control
    void setLoop(bool loop);
    bool getLoop() const;

    // Utility
    Type getType() const;
    bool isValid() const;
};