#include "WTowerHUDWidget.h"
#include "WTowerGameState.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"
#include <Components/VerticalBox.h>

void UWTowerHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Инициализируем цвета и иконки усилений, если они не установлены в редакторе
    if (PowerUpColors.Num() == 0)
    {
        PowerUpColors.Add(EPowerUpType::SpeedBoost, FLinearColor(0.0f, 0.5f, 1.0f)); // Голубой
        PowerUpColors.Add(EPowerUpType::ExtraJump, FLinearColor(0.0f, 1.0f, 0.0f));  // Зеленый
       
    }

    // Инициализируем отображаемые значения
    UpdateStats();
}

void UWTowerHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Обновляем статистику каждый кадр
    UpdateStats();

    // Обновляем таймеры усилений
    UpdatePowerUpTimers(InDeltaTime);
}

AWTowerGameState* UWTowerHUDWidget::GetWTowerGameState() const
{
    return Cast<AWTowerGameState>(UGameplayStatics::GetGameState(GetWorld()));
}

void UWTowerHUDWidget::UpdateStats()
{
    AWTowerGameState* GameState = GetWTowerGameState();
    if (!GameState)
        return;

    // Обновляем счет
    if (ScoreText)
    {
        ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Счет: %d"), GameState->GetScore())));
    }

    // Обновляем время
    if (TimeText)
    {
        TimeText->SetText(FText::FromString(FString::Printf(TEXT("Время: %s"), *FormatTime(GameState->GetGameTime()))));
    }

    // Обновляем текущую высоту
    if (HeightText)
    {
        HeightText->SetText(FText::FromString(FString::Printf(TEXT("Высота: %.1f м"), GameState->GetPlayerCurrentHeight() / 100.0f)));
    }

    // Обновляем максимальную высоту
    if (MaxHeightText)
    {
        MaxHeightText->SetText(FText::FromString(FString::Printf(TEXT("Макс. высота: %.1f м"), GameState->GetPlayerMaxHeight() / 100.0f)));
    }
}

FString UWTowerHUDWidget::FormatTime(float TimeInSeconds) const
{
    int32 Minutes = FMath::FloorToInt(TimeInSeconds / 60.0f);
    int32 Seconds = FMath::FloorToInt(TimeInSeconds) % 60;

    return FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
}

void UWTowerHUDWidget::ShowPowerUp(EPowerUpType PowerUpType, float Duration)
{
    // Проверяем, отображается ли уже это усиление
    if (ActivePowerUps.Contains(PowerUpType))
    {
        // Обновляем время существующего усиления
        FActivePowerUp& PowerUp = ActivePowerUps[PowerUpType];
        PowerUp.RemainingTime = Duration;
        PowerUp.TotalDuration = Duration;

        // Сбрасываем индикатор прогресса
        if (PowerUp.TimeBar)
        {
            PowerUp.TimeBar->SetPercent(1.0f);
        }
    }
    else
    {
        // Создаем новый элемент для отображения усиления
        CreatePowerUpElement(PowerUpType, Duration);
    }
}

void UWTowerHUDWidget::HidePowerUp(EPowerUpType PowerUpType)
{
    // Проверяем, есть ли такое усиление
    if (ActivePowerUps.Contains(PowerUpType))
    {
        FActivePowerUp PowerUp = ActivePowerUps[PowerUpType];

        // Удаляем визуальные элементы
        if (PowerUpsContainer)
        {
            if (PowerUp.Icon && PowerUp.Icon->GetParent())
            {
                PowerUpsContainer->RemoveChild(PowerUp.Icon->GetParent());
            }
        }

        // Удаляем из списка активных усилений
        ActivePowerUps.Remove(PowerUpType);
    }
}

void UWTowerHUDWidget::UpdatePowerUpTimers(float DeltaTime)
{
    // Временный список для хранения усилений, которые нужно удалить
    TArray<EPowerUpType> PowerUpsToRemove;

    // Обновляем таймеры всех активных усилений
    for (auto& Pair : ActivePowerUps)
    {
        EPowerUpType PowerUpType = Pair.Key;
        FActivePowerUp& PowerUp = Pair.Value;

        // Уменьшаем оставшееся время
        PowerUp.RemainingTime -= DeltaTime;

        // Обновляем индикатор прогресса
        if (PowerUp.TimeBar)
        {
            float Percent = FMath::Clamp(PowerUp.RemainingTime / PowerUp.TotalDuration, 0.0f, 1.0f);
            PowerUp.TimeBar->SetPercent(Percent);
        }

        // Если время истекло, добавляем в список для удаления
        if (PowerUp.RemainingTime <= 0.0f)
        {
            PowerUpsToRemove.Add(PowerUpType);
        }
    }

    // Удаляем усиления с истекшим временем
    for (EPowerUpType PowerUpType : PowerUpsToRemove)
    {
        HidePowerUp(PowerUpType);
    }
}

void UWTowerHUDWidget::CreatePowerUpElement(EPowerUpType PowerUpType, float Duration)
{
    if (!PowerUpsContainer || !WidgetTree)
        return;

    // Создаем контейнер для иконки и индикатора прогресса
    UBorder* Border = WidgetTree->ConstructWidget<UBorder>();
    if (!Border)
        return;

    // Настраиваем стиль рамки
    Border->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.7f));
    Border->SetPadding(FMargin(5.0f));
    PowerUpsContainer->AddChild(Border);

    // Создаем вертикальный контейнер для размещения иконки и прогресс-бара
    UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>();
    Border->SetContent(VBox);

    // Создаем иконку усиления
    UImage* Icon = WidgetTree->ConstructWidget<UImage>();
    VBox->AddChild(Icon);

    // Устанавливаем текстуру иконки, если доступна
    if (PowerUpIcons.Contains(PowerUpType))
    {
        Icon->SetBrushFromTexture(PowerUpIcons[PowerUpType], true);
    }

    // Устанавливаем цвет иконки для идентификации типа усиления
    if (PowerUpColors.Contains(PowerUpType))
    {
        Icon->SetColorAndOpacity(PowerUpColors[PowerUpType]);
    }

    // Создаем индикатор прогресса
    UProgressBar* TimeBar = WidgetTree->ConstructWidget<UProgressBar>();
    VBox->AddChild(TimeBar);

    // Настраиваем стиль индикатора
    if (PowerUpColors.Contains(PowerUpType))
    {
        TimeBar->SetFillColorAndOpacity(PowerUpColors[PowerUpType]);
    }
    TimeBar->SetPercent(1.0f);

    // Сохраняем информацию об усилении
    FActivePowerUp PowerUp;
    PowerUp.Icon = Icon;
    PowerUp.TimeBar = TimeBar;
    PowerUp.RemainingTime = Duration;
    PowerUp.TotalDuration = Duration;

    ActivePowerUps.Add(PowerUpType, PowerUp);
}