// PowerUpActor.cpp
#include "PowerUpActor.h"
#include "Components/SphereComponent.h"
#include "BaruCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundBase.h"

APowerUpActor::APowerUpActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Создаем компоненты
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    RootComponent = CollisionComponent;
    CollisionComponent->SetSphereRadius(50.0f);
    CollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    PowerUpComponent = CreateDefaultSubobject<UPowerUpComponent>(TEXT("PowerUpComponent"));

    // Значения по умолчанию
    PowerUpType = EPowerUpType::ExtraJump;
    Duration = 5.0f;
    Strength = 1.0f;

    // Визуальные эффекты
    RotationSpeed = 90.0f;
    HoverAmplitude = 10.0f;
    HoverFrequency = 2.0f;
    TimeElapsed = 0.0f;
}

void APowerUpActor::BeginPlay()
{
    Super::BeginPlay();

    // Сохраняем начальное положение для эффекта парения
    InitialLocation = GetActorLocation();

    // Подписываемся на событие пересечения
    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &APowerUpActor::OnOverlapBegin);

    // Настраиваем PowerUpComponent из свойств этого актора
    PowerUpComponent->PowerUpType = PowerUpType;
    PowerUpComponent->Duration = Duration;
    PowerUpComponent->Strength = Strength;

    // Обновляем визуальный стиль
    UpdateVisuals();
}

void APowerUpActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Вращение
    FRotator NewRotation = GetActorRotation();
    NewRotation.Yaw += RotationSpeed * DeltaTime;
    SetActorRotation(NewRotation);

    // Эффект парения
    TimeElapsed += DeltaTime;
    float HoverOffset = HoverAmplitude * FMath::Sin(TimeElapsed * HoverFrequency);

    FVector NewLocation = InitialLocation;
    NewLocation.Z += HoverOffset;
    SetActorLocation(NewLocation);
}

void APowerUpActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // Проверяем, что пересечение произошло с игроком
    ABaruCharacter* Character = Cast<ABaruCharacter>(OtherActor);
    if (Character)
    {
        // Применяем усиление
        PowerUpComponent->ApplyPowerUp(Character);

        // Проигрываем звук подбора (можно добавить в UPowerUpComponent)
        UGameplayStatics::PlaySound2D(this, nullptr); // Здесь нужно задать звук в BP

        // Создаем эффект подбора
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            nullptr, // Здесь нужно задать систему частиц в BP
            GetActorLocation(),
            FRotator::ZeroRotator,
            true
        );

        // Уничтожаем объект усиления
        Destroy();
    }
}

void APowerUpActor::UpdateVisuals()
{
    UMaterialInstanceDynamic* DynamicMaterial = nullptr;

    // Устанавливаем базовую сферическую геометрию если не установлена
    if (!MeshComponent->GetStaticMesh())
    {
        UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
        if (SphereMesh)
        {
            MeshComponent->SetStaticMesh(SphereMesh);
        }
    }

    // Создаем динамический материал
    if (MeshComponent->GetMaterial(0))
    {
        DynamicMaterial = UMaterialInstanceDynamic::Create(MeshComponent->GetMaterial(0), this);
    }
    else
    {
        // Пробуем создать базовый эмиссионный материал
        UMaterial* DefaultMat = LoadObject<UMaterial>(nullptr, TEXT("/Engine/EngineMaterials/DefaultLitEmissiveMaterial.DefaultLitEmissiveMaterial"));
        if (DefaultMat)
        {
            DynamicMaterial = UMaterialInstanceDynamic::Create(DefaultMat, this);
        }
    }

    // Устанавливаем цвет на основе типа усиления
    if (DynamicMaterial)
    {
        // Вместо прямого доступа к PowerUpColors используем метод GetColorForPowerUpType
        FLinearColor Color = UPowerUpComponent::GetColorForPowerUpType(PowerUpType);

        // Применяем цвет к материалу
        DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
        DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), Color * 5.0f);
        DynamicMaterial->SetScalarParameterValue(TEXT("Emissive"), 5.0f);
        MeshComponent->SetMaterial(0, DynamicMaterial);
    }
}
