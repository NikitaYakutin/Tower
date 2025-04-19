#include "WTowerGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "SaveGame/WTowerSaveGame.h"
#include "Config/WTowerGameConfig.h"
#include "Audio/WAudioManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "JsonObjectConverter.h"
#include "WTowerGameState.h"

UWTowerGameInstance::UWTowerGameInstance()
{
    // Инициализация переменных по умолчанию
    CurrentSaveSlot = TEXT("DefaultSave");
    CurrentSaveGame = nullptr;
    GameConfig = nullptr;
    AudioManager = nullptr;
    CurrentLevelIndex = 0;
    
    // Установка путей для файлов настроек
    ConfigFolder = FPaths::ProjectSavedDir() + TEXT("Config/");
    ConfigFilePath = ConfigFolder + TEXT("GameConfig.json");
}

void UWTowerGameInstance::Init()
{
    Super::Init();
    
    // Создаем директорию для настроек, если она не существует
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*ConfigFolder))
    {
        PlatformFile.CreateDirectoryTree(*ConfigFolder);
    }
    
    // Инициализируем сохранение и настройки
    InitializeSaveGame();
    InitializeGameConfig();
    InitializeLevelSequence();
    
    // Создаем и инициализируем аудио менеджер
    AudioManager = NewObject<UWAudioManager>(this);
    if (AudioManager)
    {
        AudioManager->Initialize(this);
    }
    
    UE_LOG(LogTemp, Log, TEXT("WTowerGameInstance: Initialized"));
}

//----------------------------------------------------------------------------------------
// МЕТОДЫ УПРАВЛЕНИЯ СОХРАНЕНИЯМИ
//----------------------------------------------------------------------------------------

void UWTowerGameInstance::InitializeSaveGame()
{
    // Проверяем существующее сохранение
    if (UGameplayStatics::DoesSaveGameExist(CurrentSaveSlot, 0))
    {
        // Загружаем существующее сохранение
        CurrentSaveGame = Cast<UWTowerSaveGame>(UGameplayStatics::LoadGameFromSlot(CurrentSaveSlot, 0));
        UE_LOG(LogTemp, Log, TEXT("WTowerGameInstance: Loaded existing save game"));
    }
    
    // Если сохранения нет, создаем новое
    if (!CurrentSaveGame)
    {
        CurrentSaveGame = Cast<UWTowerSaveGame>(UGameplayStatics::CreateSaveGameObject(UWTowerSaveGame::StaticClass()));
        UE_LOG(LogTemp, Log, TEXT("WTowerGameInstance: Created new save game"));
    }
}

bool UWTowerGameInstance::SaveGame()
{
    if (CurrentSaveGame)
    {
        // Сохраняем текущий прогресс
        return UGameplayStatics::SaveGameToSlot(CurrentSaveGame, CurrentSaveSlot, 0);
    }
    return false;
}

bool UWTowerGameInstance::LoadGame()
{
    if (UGameplayStatics::DoesSaveGameExist(CurrentSaveSlot, 0))
    {
        // Загружаем сохраненный прогресс
        CurrentSaveGame = Cast<UWTowerSaveGame>(UGameplayStatics::LoadGameFromSlot(CurrentSaveSlot, 0));
        return CurrentSaveGame != nullptr;
    }
    return false;
}

//----------------------------------------------------------------------------------------
// МЕТОДЫ УПРАВЛЕНИЯ НАСТРОЙКАМИ
//----------------------------------------------------------------------------------------

void UWTowerGameInstance::InitializeGameConfig()
{
    // Создаем объект конфигурации
    GameConfig = NewObject<UWTowerGameConfig>(this);
    
    // Пытаемся загрузить настройки из файла
    LoadGameConfig();
    
    // Если не удалось загрузить, используем настройки по умолчанию
    if (!GameConfig->IsInitialized())
    {
        GameConfig->InitializeDefaultSettings();
        SaveGameConfig();
    }
}

void UWTowerGameInstance::SaveGameConfig()
{
    if (GameConfig)
    {
        // Сериализуем GameConfig в JSON
        FString JsonOutput;
        FJsonObjectConverter::UStructToJsonObjectString(GameConfig->GetClass(), GameConfig, JsonOutput, 0, 0);
        
        // Записываем JSON в файл
        FFileHelper::SaveStringToFile(JsonOutput, *ConfigFilePath);
    }
}

void UWTowerGameInstance::LoadGameConfig()
{
    FString JsonString;

    // Проверяем существование файла настроек
    if (FFileHelper::LoadFileToString(JsonString, *ConfigFilePath))
    {
        // Сначала создаем JSON объект
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

        if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
        {
            // Затем преобразуем JSON объект в структуру используя JsonObjectToUStruct
            FJsonObjectConverter::JsonObjectToUStruct(
                JsonObject.ToSharedRef(),
                GameConfig->GetClass(),
                GameConfig,
                0, 0);
            GameConfig->SetInitialized(true);
        }
    }
}

void UWTowerGameInstance::ApplySettings()
{
    // Применяем настройки графики
    if (GameConfig)
    {
        GameConfig->ApplyGraphicsSettings();
    }
    
    // Применяем настройки звука
    if (AudioManager)
    {
        AudioManager->ApplySoundSettings();
    }
}

//----------------------------------------------------------------------------------------
// МЕТОДЫ УПРАВЛЕНИЯ УРОВНЯМИ
//----------------------------------------------------------------------------------------

void UWTowerGameInstance::InitializeLevelSequence()
{
    // Заполняем массив уровней (названия должны совпадать с названиями файлов карт)
    LevelSequence.Add(TEXT("MainMenu"));
    LevelSequence.Add(TEXT("Level_1"));
    LevelSequence.Add(TEXT("Level_2"));
    LevelSequence.Add(TEXT("Level_3"));
    // Добавьте другие уровни по мере необходимости
}

void UWTowerGameInstance::LoadNextLevel()
{
    // Увеличиваем индекс текущего уровня
    CurrentLevelIndex++;
    
    // Проверяем, не вышли ли за пределы массива
    if (CurrentLevelIndex >= LevelSequence.Num())
    {
        // Если это последний уровень, возвращаемся в главное меню
        CurrentLevelIndex = 0;
    }
    
    // Загружаем уровень
    UGameplayStatics::OpenLevel(GetWorld(), FName(*LevelSequence[CurrentLevelIndex]));
}

void UWTowerGameInstance::RestartCurrentLevel()
{
    // Перезапускаем текущий уровень
    FString CurrentLevel = LevelSequence[CurrentLevelIndex];
    UGameplayStatics::OpenLevel(GetWorld(), FName(*CurrentLevel));
}

void UWTowerGameInstance::OpenMainMenu()
{
    // Устанавливаем индекс на главное меню и открываем его
    CurrentLevelIndex = 0;
    UGameplayStatics::OpenLevel(GetWorld(), FName(*LevelSequence[CurrentLevelIndex]));
}

//----------------------------------------------------------------------------------------
// МЕТОДЫ ОБРАБОТКИ РЕКОРДОВ
//----------------------------------------------------------------------------------------

void UWTowerGameInstance::UpdateBestCompletionTime(const FString& LevelName, float NewTime)
{
    if (CurrentSaveGame)
    {
        // Обновляем лучшее время прохождения, если оно лучше предыдущего
        float CurrentBestTime = CurrentSaveGame->GetBestCompletionTime(LevelName);
        if (CurrentBestTime <= 0.0f || NewTime < CurrentBestTime)
        {
            CurrentSaveGame->SetBestCompletionTime(LevelName, NewTime);
            SaveGame();
            
            UE_LOG(LogTemp, Log, TEXT("WTowerGameInstance: Updated best time for %s: %.2f seconds"), *LevelName, NewTime);
        }
    }
}

void UWTowerGameInstance::UpdateBestScore(const FString& LevelName, int32 NewScore)
{
    if (CurrentSaveGame)
    {
        // Обновляем лучший счет, если он выше предыдущего
        int32 CurrentBestScore = CurrentSaveGame->GetBestScore(LevelName);
        if (NewScore > CurrentBestScore)
        {
            CurrentSaveGame->SetBestScore(LevelName, NewScore);
            SaveGame();
            
            UE_LOG(LogTemp, Log, TEXT("WTowerGameInstance: Updated best score for %s: %d"), *LevelName, NewScore);
        }
    }
}