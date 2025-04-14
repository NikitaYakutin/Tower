// Реализация класса платформы

#include "DoodlePlatform.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "BaruCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PowerUpComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "DrawDebugHelpers.h"

ADoodlePlatform::ADoodlePlatform()
{
    PrimaryActorTick.bCanEverTick = true;

    // Создаем корневой компонент
    PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
    RootComponent = PlatformMesh;

    if (PlatformMesh)
    {
        PlatformMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        PlatformMesh->SetCollisionResponseToAllChannels(ECR_Block);
    }

    // Создаем коллизию верхней части
    TopCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("TopCollision"));
    if (TopCollision && RootComponent)
    {
        TopCollision->SetupAttachment(RootComponent);
        TopCollision->SetBoxExtent(FVector(50.0f, 50.0f, 2.0f));
        TopCollision->SetRelativeLocation(FVector(0.0f, 0.0f, 10.0f));
        TopCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        TopCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
        TopCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    }

    // Создаем меш для усиления
    PowerUpMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PowerUpMesh"));
    if (PowerUpMesh && RootComponent)
    {
        PowerUpMesh->SetupAttachment(RootComponent);
        PowerUpMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 30.0f));
        PowerUpMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PowerUpMesh->SetVisibility(false);
    }

    // Значения по умолчанию
    PlatformType = EPlatformType::Normal;
    MovementRange = 200.0f;
    MovementSpeed = 100.0f;
    MoveHorizontal = true;
    BreakDelay = 0.5f;
    BounceMultiplier = 1.5f;
    MovementDirection = 1.0f;

    bHasPowerUp = false;
    PowerUpSpawnChance = 0.2f;
    PowerUpClass = nullptr;

    bShowDebugInfo = false;

    // Инициализация цветов
    PlatformColors.SetNum(4);
    PlatformColors[0] = FLinearColor(0.2f, 0.8f, 0.2f);
    PlatformColors[1] = FLinearColor(0.2f, 0.2f, 0.8f);
    PlatformColors[2] = FLinearColor(0.8f, 0.8f, 0.2f);
    PlatformColors[3] = FLinearColor(0.8f, 0.2f, 0.2f);
}

void ADoodlePlatform::BeginPlay()
{
    Super::BeginPlay();

    // Сохраняем начальную позицию для движущихся платформ
    InitialPosition = GetActorLocation();

    // Регистрируем обработчик события пересечения
    TopCollision->OnComponentBeginOverlap.AddDynamic(this, &ADoodlePlatform::OnPlayerLanded);

    // Проверяем и инициализируем массивы материалов
    InitializeArrays();

    // Обновляем внешний вид в зависимости от типа
    UpdateAppearance();

    // Инициализируем усиление, если необходимо
    if (bHasPowerUp && PowerUpClass)
    {
        SetupPowerUp();
    }

    // Выводим отладочную информацию
    if (bShowDebugInfo)
    {
        FString TypeName;
        switch (PlatformType)
        {
        case EPlatformType::Normal: TypeName = "Обычная"; break;
        case EPlatformType::Moving: TypeName = "Движущаяся"; break;
        case EPlatformType::Breakable: TypeName = "Разрушаемая"; break;
        case EPlatformType::Bouncy: TypeName = "Пружинная"; break;
        default: TypeName = "Неизвестный"; break;
        }
        UE_LOG(LogTemp, Display, TEXT("Платформа типа %s создана в позиции %s"), *TypeName, *GetActorLocation().ToString());
    }
}

void ADoodlePlatform::InitializeArrays()
{
    // Убеждаемся, что у нас есть 4 цвета в массиве
    if (PlatformColors.Num() < 4)
    {
        PlatformColors.SetNum(4);
        PlatformColors[0] = FLinearColor(0.2f, 0.8f, 0.2f); // Зеленый для Normal
        PlatformColors[1] = FLinearColor(0.2f, 0.2f, 0.8f); // Синий для Moving
        PlatformColors[2] = FLinearColor(0.8f, 0.8f, 0.2f); // Желтый для Breakable
        PlatformColors[3] = FLinearColor(0.8f, 0.2f, 0.2f); // Красный для Bouncy
    }

    // Если материалы не заданы, создаём пустой массив
    if (PlatformMaterials.Num() < 4)
    {
        PlatformMaterials.SetNum(4);
        UE_LOG(LogTemp, Warning, TEXT("Материалы платформы не установлены. Будут использованы цвета по умолчанию"));
    }
}

void ADoodlePlatform::UpdateAppearance()
{
    if (!PlatformMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("Ошибка: PlatformMesh не существует"));
        return;
    }

    // Индекс материала/цвета на основе типа платформы
    int32 TypeIndex = static_cast<int32>(PlatformType);

    // Проверяем, есть ли готовый материал для этого типа
    if (PlatformMaterials.IsValidIndex(TypeIndex) && PlatformMaterials[TypeIndex] != nullptr)
    {
        // Используем готовый материал
        PlatformMesh->SetMaterial(0, PlatformMaterials[TypeIndex]);
        UE_LOG(LogTemp, Display, TEXT("Установлен существующий материал для платформы типа %d"), TypeIndex);
    }
    else
    {
        // Материала нет, используем цвет
        if (PlatformColors.IsValidIndex(TypeIndex))
        {
            // Получаем текущий материал и создаем динамический экземпляр если его еще нет
            UMaterialInterface* BaseMaterial = PlatformMesh->GetMaterial(0);
            if (BaseMaterial)
            {
                UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(BaseMaterial);
                if (!DynMaterial)
                {
                    // Создаем новый динамический материал на основе текущего
                    DynMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
                    if (DynMaterial)
                    {
                        PlatformMesh->SetMaterial(0, DynMaterial);
                    }
                }

                // Устанавливаем цвет
                if (DynMaterial)
                {
                    // Пробуем разные названия параметров цвета
                    if (DynMaterial->IsValidLowLevelFast())
                    {
                        // Пробуем несколько распространенных имен параметров цвета
                        // Просто пытаемся установить цвет с разными именами, не проверяя результат
                        DynMaterial->SetVectorParameterValue(TEXT("Color"), PlatformColors[TypeIndex]);
                        DynMaterial->SetVectorParameterValue(TEXT("BaseColor"), PlatformColors[TypeIndex]);
                        DynMaterial->SetVectorParameterValue(TEXT("DiffuseColor"), PlatformColors[TypeIndex]);

                        UE_LOG(LogTemp, Display, TEXT("Установлен цвет для платформы типа %d"), TypeIndex);
                    }
                    else
                    {
                        UE_LOG(LogTemp, Error, TEXT("Динамический материал недействителен"));
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Не удалось создать динамический материал"));
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("На меше нет материала, невозможно обновить внешний вид"));

                // Пытаемся установить простой цвет напрямую
                SetPlatformColor(PlatformColors[TypeIndex]);
            }
        }
    }
}

void ADoodlePlatform::SetPlatformColor(const FLinearColor& Color)
{
    if (!PlatformMesh)
        return;

    // Создаем динамический материал
    UMaterialInstanceDynamic* DynMaterial = nullptr;

    // Используем существующий материал или пробуем найти базовый
    UMaterialInterface* BaseMaterial = PlatformMesh->GetMaterial(0);
    if (BaseMaterial)
    {
        DynMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
    }
    else
    {
        // Пробуем загрузить базовый материал
        UMaterialInterface* DefaultMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
        if (DefaultMaterial)
        {
            DynMaterial = UMaterialInstanceDynamic::Create(DefaultMaterial, this);
        }
    }

    // Если удалось создать динамический материал, устанавливаем цвет
    if (DynMaterial)
    {
        // Устанавливаем цвет (пробуем разные имена параметров)
        DynMaterial->SetVectorParameterValue(TEXT("Color"), Color);
        DynMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
        DynMaterial->SetVectorParameterValue(TEXT("DiffuseColor"), Color);

        // Применяем материал к мешу
        PlatformMesh->SetMaterial(0, DynMaterial);
    }
}
void ADoodlePlatform::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Обрабатываем движущиеся платформы
    if (PlatformType == EPlatformType::Moving)
    {
        FVector CurrentLocation = GetActorLocation();

        // Определяем ось движения
        FVector MovementAxis = MoveHorizontal ? FVector(0.0f, 1.0f, 0.0f) : FVector(1.0f, 0.0f, 0.0f);

        // Рассчитываем расстояние движения
        float Distance = FVector::Dist(CurrentLocation, InitialPosition);

        // Если мы достигли диапазона движения, меняем направление
        if (Distance >= MovementRange)
        {
            MovementDirection *= -1.0f;
        }

        // Применяем движение
        FVector NewLocation = CurrentLocation + MovementAxis * MovementSpeed * MovementDirection * DeltaTime;
        SetActorLocation(NewLocation);
    }
}

void ADoodlePlatform::OnPlayerLanded(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Проверяем, что приземлившийся актор - это персонаж игрока
    ABaruCharacter* Player = Cast<ABaruCharacter>(OtherActor);
    if (Player)
    {
        UE_LOG(LogTemp, Display, TEXT("Игрок приземлился на платформу типа %d"), static_cast<int32>(PlatformType));

        // Обрабатываем разные типы платформ
        switch (PlatformType)
        {
            // В функции OnPlayerLanded - изменить обработку для Breakable
        case EPlatformType::Breakable:
            // Визуальная индикация начала разрушения
            if (PlatformMesh)
            {
                UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(PlatformMesh->GetMaterial(0));
                if (!DynMaterial)
                {
                    DynMaterial = UMaterialInstanceDynamic::Create(PlatformMesh->GetMaterial(0), this);
                    PlatformMesh->SetMaterial(0, DynMaterial);
                }

                if (DynMaterial)
                {
                    // Меняем цвет на красный для индикации
                    DynMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.2f, 0.2f));
                }

                // Добавляем покачивание платформы
// Добавляем покачивание платформы
                FTimerHandle ShakeTimerHandle;

                // Для UE4 версий ниже 4.22
                // auto WeakThis = MakeWeakObjectPtr<AActor>(this); 

                // Для UE4 4.22 и выше / UE5
                auto WeakThis = TWeakObjectPtr<AActor>(this);

                if (GetWorld())
                {
                    GetWorld()->GetTimerManager().SetTimer(ShakeTimerHandle,
                        [WeakThis]()
                        {
                            if (WeakThis.IsValid())
                            {
                                float ShakeAmount = 2.0f;
                                FVector NewLocation = WeakThis->GetActorLocation() + FVector(
                                    FMath::RandRange(-ShakeAmount, ShakeAmount),
                                    FMath::RandRange(-ShakeAmount, ShakeAmount),
                                    0);
                                WeakThis->SetActorLocation(NewLocation);
                            }
                        },
                        0.05f, true);
                }
            }

            // Планируем разрушение платформы
            if (GetWorld())
            {
                UE_LOG(LogTemp, Display, TEXT("Breakable platform: Starting destruction sequence in %f seconds"), BreakDelay);
                FTimerHandle TimerHandle;
                GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ADoodlePlatform::BreakPlatform, BreakDelay, false);
            }
            break;

            // В функции OnPlayerLanded - улучшить обработку для Bouncy
        case EPlatformType::Bouncy:
            // Визуальный эффект сжатия и растяжения
            if (PlatformMesh)
            {
                // Сжимаем платформу
                FVector OriginalScale = PlatformMesh->GetRelativeScale3D();
                FVector CompressedScale = FVector(OriginalScale.X * 1.2f, OriginalScale.Y * 1.2f, OriginalScale.Z * 0.5f);
                PlatformMesh->SetRelativeScale3D(CompressedScale);

                // Возвращаем к исходному размеру с задержкой
                FTimerHandle ScaleTimerHandle;
                GetWorld()->GetTimerManager().SetTimer(ScaleTimerHandle, [this, OriginalScale]()
                    {
                        PlatformMesh->SetRelativeScale3D(OriginalScale);
                    }, 0.15f, false);
            }

            // Применяем дополнительный отскок
            if (Player->GetCharacterMovement())
            {
                UE_LOG(LogTemp, Display, TEXT("Bouncy platform: Applying bounce factor %f"), BounceMultiplier);

                // Сохраняем текущую скорость падения и увеличиваем её
                float OriginalJumpVelocity = Player->GetCharacterMovement()->JumpZVelocity;
                Player->GetCharacterMovement()->JumpZVelocity = OriginalJumpVelocity * BounceMultiplier;

                // Прикладываем импульс напрямую для более надежного эффекта
                Player->GetCharacterMovement()->Velocity = FVector(
                    Player->GetCharacterMovement()->Velocity.X,
                    Player->GetCharacterMovement()->Velocity.Y,
                    Player->GetCharacterMovement()->JumpZVelocity
                );
                Player->Jump();

                // Возвращаем исходную скорость прыжка
                FTimerHandle ResetTimerHandle;
                GetWorld()->GetTimerManager().SetTimer(ResetTimerHandle, [Player, OriginalJumpVelocity]()
                    {
                        if (Player && Player->GetCharacterMovement())
                        {
                            Player->GetCharacterMovement()->JumpZVelocity = OriginalJumpVelocity;
                        }
                    }, 0.5f, false);
            }
            break;

        default:
            // Обычные и движущиеся платформы обрабатываются стандартным приземлением персонажа
            break;
        }

        // Проверка и активация усиления
        UPowerUpComponent* PowerUp = FindComponentByClass<UPowerUpComponent>();
        if (PowerUp && PowerUpMesh && PowerUpMesh->IsVisible())
        {
            UE_LOG(LogTemp, Display, TEXT("Активация усиления при приземлении игрока"));
            ActivatePowerUp(Player);
        }
    }
}



void ADoodlePlatform::BreakPlatform()
{
    UE_LOG(LogTemp, Display, TEXT("Платформа разрушается"));

    // Визуальные эффекты можно добавить в Blueprint

    // Отключаем коллизию
    PlatformMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    TopCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Делаем платформу невидимой
    PlatformMesh->SetVisibility(false);

    // Уничтожаем с задержкой, чтобы эффекты могли проиграться
    SetLifeSpan(2.0f);
}
// Остальные методы остаются теми же, но добавляем проверки на nullptr...

void ADoodlePlatform::SetupPowerUp()
{
    if (!PowerUpClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("PowerUpClass не установлен в настройках платформы"));
        return;
    }

    if (FMath::FRand() > PowerUpSpawnChance)
    {
        return;
    }

    if (!PowerUpMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("PowerUpMesh не найден"));
        return;
    }

    UPowerUpComponent* PowerUp = NewObject<UPowerUpComponent>(this, PowerUpClass);
    if (!PowerUp)
    {
        UE_LOG(LogTemp, Error, TEXT("Не удалось создать PowerUpComponent"));
        return;
    }

    PowerUp->RegisterComponent();

    PowerUpMesh->SetVisibility(true);

    if (UMaterialInterface* Material = PowerUpMesh->GetMaterial(0))
    {
        UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(Material, this);
        if (DynMaterial)
        {
            DynMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), PowerUp->GlowColor * 5.0f);
            PowerUpMesh->SetMaterial(0, DynMaterial);
        }
    }

    GetWorld()->GetTimerManager().SetTimer(
        PowerUpAnimTimerHandle,
        this,
        &ADoodlePlatform::AnimatePowerUp,
        0.016f,
        true
    );
}

void ADoodlePlatform::AnimatePowerUp()
{
    if (PowerUpMesh && PowerUpMesh->IsVisible())
    {
        // Вращение
        PowerUpMesh->AddRelativeRotation(FRotator(0.0f, 1.0f, 0.0f));

        // Плавающее движение вверх-вниз
        float Time = GetWorld()->GetTimeSeconds();
        float NewZ = 40.0f + FMath::Sin(Time * 2.0f) * 10.0f;

        FVector CurrentLocation = PowerUpMesh->GetRelativeLocation();
        CurrentLocation.Z = NewZ;
        PowerUpMesh->SetRelativeLocation(CurrentLocation);
    }
}
void ADoodlePlatform::ActivatePowerUp(AActor* Activator)
{
    // Находим компонент усиления
    UPowerUpComponent* PowerUp = FindComponentByClass<UPowerUpComponent>();
    if (!ensure(PowerUp))
    {
        UE_LOG(LogTemp, Warning, TEXT("PowerUpComponent не найден при активации"));
        return;
    }

    // Применяем усиление через обновленный интерфейс
    PowerUp->ApplyPowerUp(Activator);

    // Очищаем визуальное представление
    if (PowerUpMesh)
    {
        PowerUpMesh->SetVisibility(false);
        GetWorld()->GetTimerManager().ClearTimer(PowerUpAnimTimerHandle);
    }

    // Визуальный эффект активации
    if (ensure(GetWorld()))
    {
        DrawDebugSphere(
            GetWorld(),
            PowerUpMesh->GetComponentLocation(),
            30.0f,
            12,
            FColor::Yellow,
            false,
            0.5f
        );
    }
}
