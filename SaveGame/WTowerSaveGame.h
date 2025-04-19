#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "WTowerSaveGame.generated.h"

/**
 * Структура для хранения данных об уровне
 */
USTRUCT(BlueprintType)
struct FLevelData
{
    GENERATED_BODY()
    
    // Лучшее время прохождения
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Уровень")
    float BestCompletionTime;
    
    // Лучший счет
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Уровень")
    int32 BestScore;
    
    // Флаг разблокировки уровня
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Уровень")
    bool bUnlocked;
    
    FLevelData()
        : BestCompletionTime(0.0f)
        , BestScore(0)
        , bUnlocked(false)
    {
    }
};

/**
 * Класс для сохранения прогресса игры
 */
UCLASS()
class WTOWER_API UWTowerSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UWTowerSaveGame();
    
    // Получить лучшее время прохождения для уровня
    UFUNCTION(BlueprintCallable, Category = "Сохранение")
    float GetBestCompletionTime(const FString& LevelName) const;
    
    // Установить лучшее время прохождения для уровня
    UFUNCTION(BlueprintCallable, Category = "Сохранение")
    void SetBestCompletionTime(const FString& LevelName, float Time);
    
    // Получить лучший счет для уровня
    UFUNCTION(BlueprintCallable, Category = "Сохранение")
    int32 GetBestScore(const FString& LevelName) const;
    
    // Установить лучший счет для уровня
    UFUNCTION(BlueprintCallable, Category = "Сохранение")
    void SetBestScore(const FString& LevelName, int32 Score);
    
    // Разблокировать уровень
    UFUNCTION(BlueprintCallable, Category = "Сохранение")
    void UnlockLevel(const FString& LevelName);
    
    // Проверить, разблокирован ли уровень
    UFUNCTION(BlueprintCallable, Category = "Сохранение")
    bool IsLevelUnlocked(const FString& LevelName) const;
    
    // Получить все разблокированные уровни
    UFUNCTION(BlueprintCallable, Category = "Сохранение")
    TArray<FString> GetUnlockedLevels() const;

private:
    // Имя пользователя для сохранения
    UPROPERTY(VisibleAnywhere, Category = "SaveGame")
    FString UserName;
    
    // Дата сохранения
    UPROPERTY(VisibleAnywhere, Category = "SaveGame")
    FDateTime SaveDate;
    
    // Данные о прогрессе в уровнях
    UPROPERTY(VisibleAnywhere, Category = "SaveGame")
    TMap<FString, FLevelData> LevelProgress;
};