#include "WTowerGameConfig.h"
#include "GameFramework/GameUserSettings.h"
#include <Kismet/KismetSystemLibrary.h>

UWTowerGameConfig::UWTowerGameConfig()
{
    // По умолчанию объект не инициализирован
    bInitialized = false;
}

void UWTowerGameConfig::InitializeDefaultSettings()
{
    // Настройки графики по умолчанию
    GraphicsQuality = 1;  // Среднее качество
    ScreenResolution = TEXT("1920x1080");
    bFullscreen = true;
    bVSync = true;
    MaxFPS = 60;
    
    // Настройки звука по умолчанию
    MasterVolume = 1.0f;
    MusicVolume = 0.7f;
    SFXVolume = 1.0f;
    bMuteAudio = false;
    
    // Настройки управления по умолчанию
    MouseSensitivity = 1.0f;
    bInvertYAxis = false;
    
    // Базовые настройки клавиш
    KeyBindings.Add(TEXT("MoveForward"), TEXT("W"));
    KeyBindings.Add(TEXT("MoveBackward"), TEXT("S"));
    KeyBindings.Add(TEXT("MoveLeft"), TEXT("A"));
    KeyBindings.Add(TEXT("MoveRight"), TEXT("D"));
    KeyBindings.Add(TEXT("Jump"), TEXT("SpaceBar"));
    KeyBindings.Add(TEXT("Sprint"), TEXT("LeftShift"));
    
    // Отмечаем объект как инициализированный
    bInitialized = true;
}

void UWTowerGameConfig::ApplyGraphicsSettings()
{
    UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
    if (UserSettings)
    {
        // Разрешение экрана
        FString ResolutionStr = ScreenResolution;
        FIntPoint Resolution;
        if (ResolutionStr.Contains(TEXT("x")))
        {
            TArray<FString> Parts;
            ResolutionStr.ParseIntoArray(Parts, TEXT("x"), true);
            if (Parts.Num() >= 2)
            {
                Resolution.X = FCString::Atoi(*Parts[0]);
                Resolution.Y = FCString::Atoi(*Parts[1]);
                UserSettings->SetScreenResolution(Resolution);
            }
        }
        
        // Полноэкранный режим
        EWindowMode::Type WindowMode = bFullscreen ? EWindowMode::Fullscreen : EWindowMode::Windowed;
        UserSettings->SetFullscreenMode(WindowMode);
        
        // Вертикальная синхронизация
        UserSettings->SetVSyncEnabled(bVSync);
        
        // Частота кадров
        UserSettings->SetFrameRateLimit(MaxFPS);
        
        // Установка качества графики
        switch (GraphicsQuality)
        {
            case 0: // Низкое качество
                UserSettings->SetOverallScalabilityLevel(0);
                break;
            case 1: // Среднее качество
                UserSettings->SetOverallScalabilityLevel(2);
                break;
            case 2: // Высокое качество
                UserSettings->SetOverallScalabilityLevel(3);
                break;
            default:
                UserSettings->SetOverallScalabilityLevel(2);
                break;
        }
        
        // Применить изменения
        UserSettings->ApplySettings(false);
    }
}

TArray<FString> UWTowerGameConfig::GetAvailableResolutions() const
{
    TArray<FString> Result;

    // Получаем списка поддерживаемых разрешений экрана
    TArray<FIntPoint> SupportedResolutions;
    if (UKismetSystemLibrary::GetSupportedFullscreenResolutions(SupportedResolutions))
    {
        // Преобразуем каждое разрешение в строку формата "ШxВ"
        for (const FIntPoint& Resolution : SupportedResolutions)
        {
            Result.Add(FString::Printf(TEXT("%dx%d"), Resolution.X, Resolution.Y));
        }
    }

    // Если по какой-то причине не удалось получить разрешения, добавляем стандартные
    if (Result.Num() == 0)
    {
        Result.Add(TEXT("1280x720"));  // HD
        Result.Add(TEXT("1366x768"));  // HD+
        Result.Add(TEXT("1600x900"));  // HD+
        Result.Add(TEXT("1920x1080")); // Full HD
        Result.Add(TEXT("2560x1440")); // QHD
        Result.Add(TEXT("3840x2160")); // 4K UHD
    }

    return Result;
}