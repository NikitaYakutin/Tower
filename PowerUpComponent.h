// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PowerUpComponent.generated.h"

// Типы усилений
UENUM(BlueprintType)
enum class EPowerUpType : uint8
{
    None UMETA(DisplayName = "None"),
    ExtraJump UMETA(DisplayName = "Extra Jump"),
    SpeedBoost UMETA(DisplayName = "Speed Boost"),
    Shield UMETA(DisplayName = "Shield"),
    Magnet UMETA(DisplayName = "Magnet"),
    Rocket UMETA(DisplayName = "Rocket"),
    SlowFall UMETA(DisplayName = "Slow Fall"),
    ScoreBonus UMETA(DisplayName = "Score Bonus"),  // Новый тип усиления
    Victory UMETA(DisplayName = "Victory Item") // Предмет для победы
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TOWER_API UPowerUpComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPowerUpComponent();

    // Статические методы для работы с цветами
    static void InitializePowerUpColors();
    static FLinearColor GetColorForPowerUpType(EPowerUpType Type);

    // Метод для настройки визуальных эффектов
    void UpdateVisualEffects(UStaticMeshComponent* TargetMesh);

    // Основные параметры усиления
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up")
    EPowerUpType PowerUpType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up")
    float Strength;

    // Цвет свечения для визуального эффекта
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor GlowColor;

    // Методы активации/деактивации
    UFUNCTION(BlueprintCallable, Category = "Power Up")
    void ApplyPowerUp(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Power Up")
    void RemovePowerUp(AActor* Target);

protected:
    virtual void BeginPlay() override;

private:
    // Сохраняемые значения для восстановления
    UPROPERTY()
    float OriginalGravity;

    UPROPERTY()
    float OriginalJumpVelocity;

    UPROPERTY()
    float OriginalMaxSpeed;

    // Карта цветов усилений
    static TMap<EPowerUpType, FLinearColor> PowerUpColors;
    static bool bColorsInitialized;
};
