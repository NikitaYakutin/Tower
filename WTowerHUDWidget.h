#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "BasePowerUp.h" // Добавляем доступ к типам усилений
#include "WTowerHUDWidget.generated.h"

class AWTowerGameState;
class UProgressBar;
class UHorizontalBox;
class UImage;

/**
 * Виджет для отображения игровой статистики и активных усилений
 */
UCLASS()
class WTOWER_API UWTowerHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Получить GameState
    AWTowerGameState* GetWTowerGameState() const;

    // Обновить все отображаемые значения
    void UpdateStats();

    // Форматировать время в MM:SS
    FString FormatTime(float TimeInSeconds) const;

    // Методы для управления усилениями
    void ShowPowerUp(EPowerUpType PowerUpType, float Duration);
    void HidePowerUp(EPowerUpType PowerUpType);
    void UpdatePowerUpTimers(float DeltaTime);

protected:
    // Текстовые блоки для отображения статистики
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ScoreText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TimeText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* HeightText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* MaxHeightText;

    // Контейнер для усилений
    UPROPERTY(meta = (BindWidget))
    UHorizontalBox* PowerUpsContainer;

    // Иконки для каждого типа усиления
    UPROPERTY(EditDefaultsOnly, Category = "Power-Ups")
    TMap<EPowerUpType, UTexture2D*> PowerUpIcons;

    // Цвета для каждого типа усиления
    UPROPERTY(EditDefaultsOnly, Category = "Power-Ups")
    TMap<EPowerUpType, FLinearColor> PowerUpColors;

private:
    // Структура для хранения информации об активных усилениях
    struct FActivePowerUp
    {
        UImage* Icon;
        UProgressBar* TimeBar;
        float RemainingTime;
        float TotalDuration;
    };

    // Активные усиления
    TMap<EPowerUpType, FActivePowerUp> ActivePowerUps;

    // Создает новый элемент для усиления
    void CreatePowerUpElement(EPowerUpType PowerUpType, float Duration);
};