// Copyright Epic Games, Inc. All Rights Reserved.

#include "PowerUpComponent.h"
#include "BaruCharacter.h"
#include "GameManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

TMap<EPowerUpType, FLinearColor> UPowerUpComponent::PowerUpColors;
bool UPowerUpComponent::bColorsInitialized = false;

UPowerUpComponent::UPowerUpComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    // Значения по умолчанию
    PowerUpType = EPowerUpType::None;
    Duration = 5.0f;
    Strength = 1.0f;

    // Инициализация сохраняемых значений
    OriginalGravity = 0.0f;
    OriginalJumpVelocity = 0.0f;
    OriginalMaxSpeed = 0.0f;
}

void UPowerUpComponent::InitializePowerUpColors()
{
    if (!bColorsInitialized)
    {
        PowerUpColors.Add(EPowerUpType::ExtraJump, FLinearColor(0.0f, 1.0f, 0.0f));
        PowerUpColors.Add(EPowerUpType::SpeedBoost, FLinearColor(0.0f, 0.8f, 1.0f));
        PowerUpColors.Add(EPowerUpType::Shield, FLinearColor(1.0f, 0.8f, 0.0f));
        PowerUpColors.Add(EPowerUpType::Magnet, FLinearColor(0.8f, 0.2f, 1.0f));
        PowerUpColors.Add(EPowerUpType::Rocket, FLinearColor(1.0f, 0.4f, 0.0f));
        PowerUpColors.Add(EPowerUpType::SlowFall, FLinearColor(0.6f, 0.8f, 1.0f));
        PowerUpColors.Add(EPowerUpType::ScoreBonus, FLinearColor(1.0f, 0.5f, 0.5f)); // Розовый цвет
        PowerUpColors.Add(EPowerUpType::Victory, FLinearColor(1.0f, 1.0f, 0.0f)); // Золотой цвет
        bColorsInitialized = true;
    }
}

void UPowerUpComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializePowerUpColors();

    // Устанавливаем цвет свечения
    if (const FLinearColor* Color = PowerUpColors.Find(PowerUpType))
    {
        GlowColor = *Color;
    }
    else
    {
        GlowColor = FLinearColor(1.0f, 1.0f, 1.0f);
    }
}

void UPowerUpComponent::ApplyPowerUp(AActor* Target)
{
    if (!ensure(Target))
    {
        UE_LOG(LogTemp, Warning, TEXT("PowerUpComponent: Invalid target for power-up"));
        return;
    }

    ABaruCharacter* Character = Cast<ABaruCharacter>(Target);
    if (!ensure(Character))
    {
        UE_LOG(LogTemp, Warning, TEXT("PowerUpComponent: Target is not a BaruCharacter"));
        return;
    }

    UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
    if (!ensure(MovementComp))
    {
        UE_LOG(LogTemp, Warning, TEXT("PowerUpComponent: No movement component found"));
        return;
    }

    // Создаем визуальный эффект
    if (Character->GetMesh())
    {
        UGameplayStatics::SpawnEmitterAttached(
            nullptr,
            Character->GetMesh(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            FVector(1.0f),
            EAttachLocation::SnapToTarget,
            true
        );
    }

    // Применяем эффект усиления
    switch (PowerUpType)
    {
    case EPowerUpType::ExtraJump:
        Character->ActivateJumpBoost(1.0f + Strength, Duration);
        break;

    case EPowerUpType::SpeedBoost:
        Character->ActivateSpeedBoost(1.0f + Strength, Duration);
        break;

    case EPowerUpType::Shield:
        Character->ActivateShield(Duration);
        break;

    case EPowerUpType::Magnet:
        Character->ActivateMagnet(Duration);
        break;

    case EPowerUpType::Rocket:
        Character->ActivateRocket();
        break;

    case EPowerUpType::SlowFall:
        Character->ActivateSlowFall(1.0f - FMath::Clamp(Strength, 0.1f, 0.9f), Duration);
        break;
    case EPowerUpType::ScoreBonus:
    {
        // Добавляем фигурные скобки для создания локальной области видимости
        int32 ScoreToAdd = FMath::FloorToInt(Strength * 5.0f);
        if (AGameManager* GameManager = AGameManager::GetInstance(Character))
        {
            GameManager->UpdateScore(ScoreToAdd);
        }
        break;
    }

    case EPowerUpType::Victory:
    {
        // Также добавляем фигурные скобки для согласованности
        if (AGameManager* GameManager = AGameManager::GetInstance(Character))
        {
            GameManager->PlayerWon();
        }
        break;
    }
    default:
        UE_LOG(LogTemp, Warning, TEXT("PowerUpComponent: Unknown power-up type"));
        break;
    }

    UE_LOG(LogTemp, Display, TEXT("PowerUpComponent: Applied %s power-up to %s"),
        *UEnum::GetValueAsString(PowerUpType), *Target->GetName());
}

void UPowerUpComponent::RemovePowerUp(AActor* Target)
{
    // Функционал перенесен в соответствующие методы персонажа
    // Каждый тип усиления сам отвечает за удаление своего эффекта
    // через методы DeactivateXXX
}
// PowerUpComponent.cpp - реализация новых методов
// Изменение в методе GetColorForPowerUpType:
FLinearColor UPowerUpComponent::GetColorForPowerUpType(EPowerUpType Type)
{
    UPowerUpComponent::InitializePowerUpColors(); // Указываем класс явно для статического метода

    const FLinearColor* Color = PowerUpColors.Find(Type);
    if (Color)
    {
        return *Color;
    }

    return FLinearColor(1.0f, 1.0f, 1.0f); // Белый цвет по умолчанию
}

void UPowerUpComponent::UpdateVisualEffects(UStaticMeshComponent* TargetMesh)
{
    if (!TargetMesh)
        return;

    // Создаем динамический материал
    UMaterialInstanceDynamic* DynamicMaterial = nullptr;

    if (TargetMesh->GetMaterial(0))
    {
        DynamicMaterial = UMaterialInstanceDynamic::Create(TargetMesh->GetMaterial(0), this);
    }
    else
    {
        // Пробуем загрузить базовый материал с эмиссией
        UMaterial* DefaultMat = LoadObject<UMaterial>(nullptr, TEXT("/Engine/EngineMaterials/DefaultLitEmissiveMaterial.DefaultLitEmissiveMaterial"));
        if (DefaultMat)
        {
            DynamicMaterial = UMaterialInstanceDynamic::Create(DefaultMat, this);
        }
    }

    if (DynamicMaterial)
    {
        // Вызываем статический метод класса, а не метод экземпляра
        FLinearColor Color = UPowerUpComponent::GetColorForPowerUpType(PowerUpType);

        // Применяем цвет к материалу
        DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
        DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), Color * 5.0f);
        DynamicMaterial->SetScalarParameterValue(TEXT("Emissive"), 5.0f);
        TargetMesh->SetMaterial(0, DynamicMaterial);
    }
}

