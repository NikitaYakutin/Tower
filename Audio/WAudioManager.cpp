#include "WAudioManager.h"
#include "../WTowerGameInstance.h"
#include "../Config/WTowerGameConfig.h"
#include "Kismet/GameplayStatics.h"

UWAudioManager::UWAudioManager()
{
    // Инициализация по умолчанию
    GameInstance = nullptr;
    BackgroundMusicComponent = nullptr;
    CurrentMasterVolume = 1.0f;
    CurrentMusicVolume = 1.0f;
    CurrentSFXVolume = 1.0f;
    SavedMasterVolume = 1.0f;
    bIsMuted = false;
}

void UWAudioManager::Initialize(UWTowerGameInstance* InGameInstance)
{
    GameInstance = InGameInstance;
    ApplySoundSettings();
    
    UE_LOG(LogTemp, Log, TEXT("WAudioManager: Initialized"));
}

void UWAudioManager::ApplySoundSettings()
{
    if (GameInstance && GameInstance->GetGameConfig())
    {
        UWTowerGameConfig* Config = GameInstance->GetGameConfig();
        
        // Получаем значения громкости из конфига
        CurrentMasterVolume = Config->MasterVolume;
        CurrentMusicVolume = Config->MusicVolume;
        CurrentSFXVolume = Config->SFXVolume;
        
        // Если есть активная музыка, обновляем её громкость
        if (BackgroundMusicComponent && BackgroundMusicComponent->IsPlaying())
        {
            BackgroundMusicComponent->SetVolumeMultiplier(CurrentMasterVolume * CurrentMusicVolume);
        }
        
        // Применяем настройку отключения звука
        if (Config->bMuteAudio && !bIsMuted)
        {
            MuteAllSounds();
        }
        else if (!Config->bMuteAudio && bIsMuted)
        {
            UnmuteAllSounds();
        }
    }
}

void UWAudioManager::SetMasterVolume(float Volume)
{
    // Ограничиваем громкость диапазоном 0-1
    Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
    CurrentMasterVolume = Volume;
    
    // Обновляем громкость фоновой музыки
    if (BackgroundMusicComponent)
    {
        BackgroundMusicComponent->SetVolumeMultiplier(CurrentMasterVolume * CurrentMusicVolume);
    }
    
    // Обновляем настройку в конфиге
    if (GameInstance && GameInstance->GetGameConfig())
    {
        GameInstance->GetGameConfig()->MasterVolume = Volume;
    }
    
    UE_LOG(LogTemp, Log, TEXT("WAudioManager: Master volume set to %.2f"), Volume);
}

void UWAudioManager::SetMusicVolume(float Volume)
{
    // Ограничиваем громкость диапазоном 0-1
    Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
    CurrentMusicVolume = Volume;
    
    // Обновляем громкость фоновой музыки
    if (BackgroundMusicComponent)
    {
        BackgroundMusicComponent->SetVolumeMultiplier(CurrentMasterVolume * CurrentMusicVolume);
    }
    
    // Обновляем настройку в конфиге
    if (GameInstance && GameInstance->GetGameConfig())
    {
        GameInstance->GetGameConfig()->MusicVolume = Volume;
    }
    
    UE_LOG(LogTemp, Log, TEXT("WAudioManager: Music volume set to %.2f"), Volume);
}

void UWAudioManager::SetSFXVolume(float Volume)
{
    // Ограничиваем громкость диапазоном 0-1
    Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
    CurrentSFXVolume = Volume;
    
    // Обновляем настройку в конфиге
    if (GameInstance && GameInstance->GetGameConfig())
    {
        GameInstance->GetGameConfig()->SFXVolume = Volume;
    }
    
    UE_LOG(LogTemp, Log, TEXT("WAudioManager: SFX volume set to %.2f"), Volume);
}

void UWAudioManager::MuteAllSounds()
{
    if (!bIsMuted)
    {
        // Сохраняем текущую громкость
        SavedMasterVolume = CurrentMasterVolume;
        
        // Отключаем звук
        SetMasterVolume(0.0f);
        
        // Устанавливаем флаг отключенного звука
        bIsMuted = true;
        
        // Обновляем настройку в конфиге
        if (GameInstance && GameInstance->GetGameConfig())
        {
            GameInstance->GetGameConfig()->bMuteAudio = true;
        }
        
        UE_LOG(LogTemp, Log, TEXT("WAudioManager: All sounds muted"));
    }
}

void UWAudioManager::UnmuteAllSounds()
{
    if (bIsMuted)
    {
        // Восстанавливаем громкость
        SetMasterVolume(SavedMasterVolume);
        
        // Снимаем флаг отключенного звука
        bIsMuted = false;
        
        // Обновляем настройку в конфиге
        if (GameInstance && GameInstance->GetGameConfig())
        {
            GameInstance->GetGameConfig()->bMuteAudio = false;
        }
        
        UE_LOG(LogTemp, Log, TEXT("WAudioManager: All sounds unmuted"));
    }
}

void UWAudioManager::PlaySoundEffect(USoundBase* Sound, FVector Location)
{
    if (Sound && !bIsMuted)
    {
        // Воспроизводим звук с учетом настроек громкости
        float FinalVolume = CurrentMasterVolume * CurrentSFXVolume;
        
        if (Location.IsZero())
        {
            // Обычный 2D звук
            UGameplayStatics::PlaySound2D(GameInstance, Sound, FinalVolume);
        }
        else
        {
            // 3D звук с указанием позиции
            UGameplayStatics::PlaySoundAtLocation(GameInstance, Sound, Location, FRotator::ZeroRotator, FinalVolume);
        }
    }
}

void UWAudioManager::PlayBackgroundMusic(USoundBase* Music)
{
    if (Music)
    {
        // Останавливаем текущую фоновую музыку, если она играет
        StopBackgroundMusic();

        // Создаем новый компонент для воспроизведения фоновой музыки
        // Параметр зацикливания теперь не требуется, т.к. он настроен в самом звуковом файле
        BackgroundMusicComponent = UGameplayStatics::CreateSound2D(GameInstance, Music);

        if (BackgroundMusicComponent)
        {

            BackgroundMusicComponent->bAutoDestroy = false;

            // Устанавливаем громкость
            float FinalVolume = bIsMuted ? 0.0f : (CurrentMasterVolume * CurrentMusicVolume);
            BackgroundMusicComponent->SetVolumeMultiplier(FinalVolume);

            // Запускаем воспроизведение
            BackgroundMusicComponent->Play();

            UE_LOG(LogTemp, Log, TEXT("WAudioManager: Started playing background music"));
        }
    }
}

void UWAudioManager::StopBackgroundMusic()
{
    if (BackgroundMusicComponent && BackgroundMusicComponent->IsPlaying())
    {
        // Останавливаем воспроизведение фоновой музыки
        BackgroundMusicComponent->Stop();
        BackgroundMusicComponent = nullptr;
        
        UE_LOG(LogTemp, Log, TEXT("WAudioManager: Stopped background music"));
    }
}