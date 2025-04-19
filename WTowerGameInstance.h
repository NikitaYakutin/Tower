#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SaveGame/WTowerSaveGame.h"
#include "Config/WTowerGameConfig.h"
#include "Audio/WAudioManager.h"
#include "WTowerGameInstance.generated.h"

/**
 * GameInstance для WTower, обеспечивающий сохранение прогресса между уровнями
 * и управление глобальными настройками игры
 */
UCLASS()
class WTOWER_API UWTowerGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    // Конструктор
    UWTowerGameInstance();
    
    // Инициализация GameInstance при запуске
    virtual void Init() override;

    //----------------------------------------------------------------------------------------
    // УПРАВЛЕНИЕ СОХРАНЕНИЯМИ
    //----------------------------------------------------------------------------------------

    // Создать новое сохранение или загрузить существующее
    UFUNCTION(BlueprintCallable, Category = "Сохранение|Загрузка")
    void InitializeSaveGame();
    
    // Сохранить текущий прогресс
    UFUNCTION(BlueprintCallable, Category = "Сохранение|Загрузка")
    bool SaveGame();
    
    // Загрузить сохраненный прогресс
    UFUNCTION(BlueprintCallable, Category = "Сохранение|Загрузка")
    bool LoadGame();
    
    // Получить объект сохранения
    UFUNCTION(BlueprintCallable, Category = "Сохранение|Загрузка")
    UWTowerSaveGame* GetSaveGame() const { return CurrentSaveGame; }

    //----------------------------------------------------------------------------------------
    // УПРАВЛЕНИЕ НАСТРОЙКАМИ
    //----------------------------------------------------------------------------------------
    
    // Получить конфигурацию игры
    UFUNCTION(BlueprintCallable, Category = "Настройки")
    UWTowerGameConfig* GetGameConfig() const { return GameConfig; }
    
    // Сохранить настройки игры
    UFUNCTION(BlueprintCallable, Category = "Настройки")
    void SaveGameConfig();
    
    // Загрузить настройки игры
    UFUNCTION(BlueprintCallable, Category = "Настройки")
    void LoadGameConfig();
    
    // Применить настройки
    UFUNCTION(BlueprintCallable, Category = "Настройки")
    void ApplySettings();

    //----------------------------------------------------------------------------------------
    // УПРАВЛЕНИЕ УРОВНЯМИ
    //----------------------------------------------------------------------------------------
    
    // Загрузить следующий уровень
    UFUNCTION(BlueprintCallable, Category = "Уровни")
    void LoadNextLevel();
    
    // Перезапустить текущий уровень
    UFUNCTION(BlueprintCallable, Category = "Уровни")
    void RestartCurrentLevel();
    
    // Открыть главное меню
    UFUNCTION(BlueprintCallable, Category = "Уровни")
    void OpenMainMenu();
    
    //----------------------------------------------------------------------------------------
    // АУДИО
    //----------------------------------------------------------------------------------------
    
    // Получить аудио менеджер
    UFUNCTION(BlueprintCallable, Category = "Аудио")
    UWAudioManager* GetAudioManager() const { return AudioManager; }
    
    //----------------------------------------------------------------------------------------
    // РЕКОРДЫ
    //----------------------------------------------------------------------------------------
    
    // Обновить лучшее время прохождения для уровня
    UFUNCTION(BlueprintCallable, Category = "Рекорды")
    void UpdateBestCompletionTime(const FString& LevelName, float NewTime);
    
    // Обновить максимальный счет для уровня
    UFUNCTION(BlueprintCallable, Category = "Рекорды")
    void UpdateBestScore(const FString& LevelName, int32 NewScore);

private:
    //----------------------------------------------------------------------------------------
    // ПРИВАТНЫЕ ПЕРЕМЕННЫЕ И МЕТОДЫ
    //----------------------------------------------------------------------------------------
    
    // Текущий слот сохранения
    UPROPERTY()
    FString CurrentSaveSlot;
    
    // Текущий объект сохранения
    UPROPERTY()
    UWTowerSaveGame* CurrentSaveGame;
    
    // Конфигурация игры
    UPROPERTY()
    UWTowerGameConfig* GameConfig;
    
    // Аудио менеджер
    UPROPERTY()
    UWAudioManager* AudioManager;
    
    // Путь к папке с настройками
    FString ConfigFolder;
    
    // Путь к файлу настроек
    FString ConfigFilePath;
    
    // Последовательность уровней в игре
    UPROPERTY()
    TArray<FString> LevelSequence;
    
    // Текущий индекс уровня
    int32 CurrentLevelIndex;
    
    // Инициализация настроек игры
    void InitializeGameConfig();
    
    // Инициализация последовательности уровней
    void InitializeLevelSequence();
};