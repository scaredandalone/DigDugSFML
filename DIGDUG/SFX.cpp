#include "SFX.h"


SFX::SFX(const std::string& filename, Type audioType) : type(audioType)
{
    if (type == Type::SOUND) {
        soundBuffer = std::make_unique<sf::SoundBuffer>();
        if (soundBuffer->loadFromFile(filename)) {
            sound = std::make_unique<sf::Sound>(*soundBuffer);
            sound->setBuffer(*soundBuffer);
        }
    }
    else {
        music = std::make_unique<sf::Music>();
        music->openFromFile(filename);
    }
}

// Playback control
void SFX::play()
{
    if (type == Type::SOUND && sound) {
        sound->play();
    }
    else if (type == Type::MUSIC && music) {
        music->play();
    }
}

void SFX::pause()
{
    if (type == Type::SOUND && sound) {
        sound->pause();
    }
    else if (type == Type::MUSIC && music) {
        music->pause();
    }
}

void SFX::stop()
{
    if (type == Type::SOUND && sound) {
        sound->stop();
    }
    else if (type == Type::MUSIC && music) {
        music->stop();
    }
}

// Status checking
bool SFX::isPlaying() const
{
    if (type == Type::SOUND && sound) {
        return sound->getStatus() == sf::Sound::Status::Playing;
    }
    else if (type == Type::MUSIC && music) {
        return music->getStatus() == sf::Music::Status::Playing;
    }
    return false;
}

// Volume control
void SFX::setVolume(float volume)
{
    if (type == Type::SOUND && sound) {
        sound->setVolume(volume);
    }
    else if (type == Type::MUSIC && music) {
        music->setVolume(volume);
    }
}

float SFX::getVolume() const
{
    if (type == Type::SOUND && sound) {
        return sound->getVolume();
    }
    else if (type == Type::MUSIC && music) {
        return music->getVolume();
    }
    return 0.0f;
}

// Loop control
void SFX::setLoop(bool loop)
{
    if (type == Type::SOUND && sound) {
        sound->setLooping(loop);
    }
    else if (type == Type::MUSIC && music) {
        music->setLooping(loop);
    }
}

bool SFX::getLoop() const
{
    if (type == Type::SOUND && sound) {
        return sound->isLooping();
    }
    else if (type == Type::MUSIC && music) {
        return music->isLooping();
    }
    return false;
}

// Utility
SFX::Type SFX::getType() const
{
    return type;
}

bool SFX::isValid() const
{
    return (type == Type::SOUND && sound && soundBuffer) ||
        (type == Type::MUSIC && music);
}