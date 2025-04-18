#include "WTowerHUD.h"
#include "WTowerHUDWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

AWTowerHUD::AWTowerHUD()
{
    // Конструктор
}

void AWTowerHUD::BeginPlay()
{
    Super::BeginPlay();

    // Проверка, установлен ли класс виджета
    if (!HUDWidgetClass)
    {
        // Загружаем класс виджета программно
        HUDWidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/Blueprints/Menu/WBP_GameHUD.WBP_GameHUD_C"));
    }

    if (HUDWidgetClass)
    {
        // Создаем и добавляем виджет
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            CurrentHUDWidget = CreateWidget<UWTowerHUDWidget>(PC, HUDWidgetClass);
            if (CurrentHUDWidget)
            {
                CurrentHUDWidget->AddToViewport(0);
            }
        }
    }
}

void AWTowerHUD::ShowHUD()
{
    if (CurrentHUDWidget)
    {
        CurrentHUDWidget->SetVisibility(ESlateVisibility::Visible);
    }
}

void AWTowerHUD::HideHUD()
{
    if (CurrentHUDWidget)
    {
        CurrentHUDWidget->SetVisibility(ESlateVisibility::Hidden);
    }
}

bool AWTowerHUD::IsHUDVisible() const
{
    if (CurrentHUDWidget)
    {
        return CurrentHUDWidget->GetVisibility() == ESlateVisibility::Visible;
    }
    return false;
}

void AWTowerHUD::ShowPowerUp(EPowerUpType PowerUpType, float Duration)
{
    if (CurrentHUDWidget)
    {
        CurrentHUDWidget->ShowPowerUp(PowerUpType, Duration);
    }
}

void AWTowerHUD::HidePowerUp(EPowerUpType PowerUpType)
{
    if (CurrentHUDWidget)
    {
        CurrentHUDWidget->HidePowerUp(PowerUpType);
    }
}