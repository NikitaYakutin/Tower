#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/AudioComponent.h"
#include "WAudioManager.generated.h"

UCLASS(Blueprintable, BlueprintType)
class WTOWER_API UWAudioManager : public UObject
{
    GENERATED_BODY()
    
public:
    UWAudioManager();
    
    // Инициализация аудио менеджера
    void Initialize(class UWTowerGameInstance* InGameInstance);
    
    // Применение настроек из GameConfig
    UFUNCTION(BlueprintCallable, Category = "Аудио")
    void ApplySoundSettings();
    
    // Методы для управления громкостью
    UFUNCTION(BlueprintCallable, Category = "Аудио")
    void SetMasterVolume(float Volume);
    
    UFUNCTION(BlueprintCallable, Category = "Аудио")
    void SetMusicVolume(float Volume);
    
    UFUNCTION(BlueprintCallable, Category = "Аудио")
    void SetSFXVolume(float Volume);
    
    // Включение/выключение звука
    UFUNCTION(BlueprintCallable, Category = "Аудио")
    void MuteAllSounds();
    
    UFUNCTION(BlueprintCallable, Category = "Аудио")
    void UnmuteAllSounds();
    
    // Воспроизведение звуков
    UFUNCTION(BlueprintCallable, Category = "Аудио")
    void PlaySoundEffect(USoundBase* Sound, FVector Location = FVector::ZeroVector);
    
    UFUNCTION(BlueprintCallable, Category = "Аудио")
    void PlayBackgroundMusic(USoundBase* Music);
    
    UFUNCTION(BlueprintCallable, Category = "Аудио")
    void StopBackgroundMusic();

private:
    // Ссылка на GameInstance
    UPROPERTY()
    class UWTowerGameInstance* GameInstance;
    
    // Компонент для фоновой музыки
    UPROPERTY()
    UAudioComponent* BackgroundMusicComponent;
    
    // Текущие значения громкости
    float CurrentMasterVolume;
    float CurrentMusicVolume;
    float CurrentSFXVolume;
    
    // Сохраненное значение громкости до отключения
    float SavedMasterVolume;
    
    // Флаг отключенного звука
    bool bIsMuted;
};