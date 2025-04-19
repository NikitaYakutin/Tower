#include "WTowerSaveGame.h"

UWTowerSaveGame::UWTowerSaveGame()
{
    // Инициализация по умолчанию
    UserName = TEXT("Player");
    SaveDate = FDateTime::Now();
    
    // Разблокируем первый уровень по умолчанию
    FLevelData FirstLevelData;
    FirstLevelData.bUnlocked = true;
    LevelProgress.Add(TEXT("Level_1"), FirstLevelData);
}

float UWTowerSaveGame::GetBestCompletionTime(const FString& LevelName) const
{
    // Получаем лучшее время прохождения для указанного уровня
    if (LevelProgress.Contains(LevelName))
    {
        return LevelProgress[LevelName].BestCompletionTime;
    }
    return 0.0f;
}

void UWTowerSaveGame::SetBestCompletionTime(const FString& LevelName, float Time)
{
    // Создаем запись для уровня, если ее нет
    if (!LevelProgress.Contains(LevelName))
    {
        FLevelData NewLevelData;
        LevelProgress.Add(LevelName, NewLevelData);
    }
    
    // Обновляем лучшее время
    FLevelData& LevelData = LevelProgress[LevelName];
    LevelData.BestCompletionTime = Time;
}

int32 UWTowerSaveGame::GetBestScore(const FString& LevelName) const
{
    // Получаем лучший счет для указанного уровня
    if (LevelProgress.Contains(LevelName))
    {
        return LevelProgress[LevelName].BestScore;
    }
    return 0;
}

void UWTowerSaveGame::SetBestScore(const FString& LevelName, int32 Score)
{
    // Создаем запись для уровня, если ее нет
    if (!LevelProgress.Contains(LevelName))
    {
        FLevelData NewLevelData;
        LevelProgress.Add(LevelName, NewLevelData);
    }
    
    // Обновляем лучший счет
    FLevelData& LevelData = LevelProgress[LevelName];
    LevelData.BestScore = Score;
}

void UWTowerSaveGame::UnlockLevel(const FString& LevelName)
{
    // Создаем запись для уровня, если ее нет
    if (!LevelProgress.Contains(LevelName))
    {
        FLevelData NewLevelData;
        LevelProgress.Add(LevelName, NewLevelData);
    }
    
    // Разблокируем уровень
    FLevelData& LevelData = LevelProgress[LevelName];
    LevelData.bUnlocked = true;
}

bool UWTowerSaveGame::IsLevelUnlocked(const FString& LevelName) const
{
    // Проверяем, разблокирован ли указанный уровень
    if (LevelProgress.Contains(LevelName))
    {
        return LevelProgress[LevelName].bUnlocked;
    }
    return false;
}

TArray<FString> UWTowerSaveGame::GetUnlockedLevels() const
{
    // Возвращаем список разблокированных уровней
    TArray<FString> UnlockedLevels;
    
    for (const auto& Pair : LevelProgress)
    {
        if (Pair.Value.bUnlocked)
        {
            UnlockedLevels.Add(Pair.Key);
        }
    }
    
    return UnlockedLevels;
}