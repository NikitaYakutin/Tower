// DoodlePlatform.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DoodlePlatform.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UPowerUpComponent;

UENUM(BlueprintType)
enum class EPlatformType : uint8
{
    Normal UMETA(DisplayName = "Normal"),
    Moving UMETA(DisplayName = "Moving"),
    Breakable UMETA(DisplayName = "Breakable"),
    Bouncy UMETA(DisplayName = "Bouncy")
};

UCLASS(Blueprintable)
class TOWER_API ADoodlePlatform : public AActor
{
    GENERATED_BODY()

public:
    ADoodlePlatform();

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
    UStaticMeshComponent* PlatformMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform")
    UBoxComponent* TopCollision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowerUp")
    UStaticMeshComponent* PowerUpMesh;

    // Platform Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform")
    EPlatformType PlatformType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform|Movement")
    float MovementRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform|Movement")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform|Movement")
    bool MoveHorizontal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform|Break")
    float BreakDelay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform|Bounce")
    float BounceMultiplier;

    // PowerUp Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp")
    bool bHasPowerUp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp")
    float PowerUpSpawnChance;

    UPROPERTY(EditDefaultsOnly, Category = "PowerUp")
    TSubclassOf<UPowerUpComponent> PowerUpClass;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void OnPlayerLanded(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

private:
    FVector InitialPosition;
    float MovementDirection;
    FTimerHandle PowerUpAnimTimerHandle;

    UPROPERTY()
    TArray<FLinearColor> PlatformColors;

    UPROPERTY()
    TArray<UMaterialInterface*> PlatformMaterials;

    void InitializeArrays();
    void UpdateAppearance();
    void SetPlatformColor(const FLinearColor& Color);
    void SetupPowerUp();
    void AnimatePowerUp();
    void ActivatePowerUp(AActor* Activator);
    void BreakPlatform();

    UPROPERTY()
    bool bShowDebugInfo;
};
