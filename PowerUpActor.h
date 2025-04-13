// PowerUpActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerUpComponent.h"
#include "PowerUpActor.generated.h"

UCLASS()
class TOWER_API APowerUpActor : public AActor
{
    GENERATED_BODY()

public:
    APowerUpActor();

    // Основные компоненты
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPowerUpComponent* PowerUpComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* CollisionComponent;

    // Настройки усиления
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power-Up")
    EPowerUpType PowerUpType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power-Up")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power-Up")
    float Strength;

    // Визуальные настройки
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float RotationSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float HoverAmplitude;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float HoverFrequency;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Обработка столкновений
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    // Обновление визуального стиля на основе типа усиления
    void UpdateVisuals();

    // Хранение начальной позиции для эффекта парения
    FVector InitialLocation;
    float TimeElapsed;
};
