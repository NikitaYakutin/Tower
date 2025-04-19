#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WTowerGameConfig.generated.h"

/**
 * Класс для хранения конфигурации игры
 */
UCLASS(Blueprintable, BlueprintType)
class WTOWER_API UWTowerGameConfig : public UObject
{
    GENERATED_BODY()

public:
    UWTowerGameConfig();
    
    // Инициализировать настройки по умолчанию
    UFUNCTION(BlueprintCallable, Category = "Конфигурация")
    void InitializeDefaultSettings();
    
    // Проверить, инициализирован ли объект
    UFUNCTION(BlueprintCallable, Category = "Конфигурация")
    bool IsInitialized() const { return bInitialized; }
    
    // Установить флаг инициализации
    UFUNCTION(BlueprintCallable, Category = "Конфигурация")
    void SetInitialized(bool Value) { bInitialized = Value; }
    
    // Применить настройки графики
    UFUNCTION(BlueprintCallable, Category = "Графика")
    void ApplyGraphicsSettings();
    
    // Получить доступные разрешения экрана
    UFUNCTION(BlueprintCallable, Category = "Графика")
    TArray<FString> GetAvailableResolutions() const;
    
    //----------------------------------------------------------------------------------------
    // НАСТРОЙКИ ГРАФИКИ
    //----------------------------------------------------------------------------------------
    
    // Качество графики (0 - низкое, 1 - среднее, 2 - высокое)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Графика")
    int32 GraphicsQuality;
    
    // Разрешение экрана
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Графика")
    FString ScreenResolution;
    
    // Полноэкранный режим
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Графика")
    bool bFullscreen;
    
    // Вертикальная синхронизация
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Графика")
    bool bVSync;
    
    // Максимальное количество кадров в секунду
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Графика")
    int32 MaxFPS;
    
    //----------------------------------------------------------------------------------------
    // НАСТРОЙКИ ЗВУКА
    //----------------------------------------------------------------------------------------
    
    // Громкость главного звука (0.0 - 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Звук")
    float MasterVolume;
    
    // Громкость музыки (0.0 - 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Звук")
    float MusicVolume;
    
    // Громкость эффектов (0.0 - 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Звук")
    float SFXVolume;
    
    // Отключение звука
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Звук")
    bool bMuteAudio;
    
    //----------------------------------------------------------------------------------------
    // НАСТРОЙКИ УПРАВЛЕНИЯ
    //----------------------------------------------------------------------------------------
    
    // Чувствительность мыши
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Управление")
    float MouseSensitivity;
    
    // Инверсия оси Y
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Управление")
    bool bInvertYAxis;
    
    // Настройки кнопок (сериализуемый объект для хранения назначенных кнопок)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Управление")
    TMap<FString, FString> KeyBindings;

private:
    // Флаг инициализации
    UPROPERTY()
    bool bInitialized;
};