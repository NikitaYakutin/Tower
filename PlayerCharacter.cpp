#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "WTowerGameState.h"
#include "WTowerHUD.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

//----------------------------------------------------------------------------------------
// КОНСТРУКТОР И ИНИЦИАЛИЗАЦИЯ
//----------------------------------------------------------------------------------------

APlayerCharacter::APlayerCharacter()
{
    // Включаем Tick
    PrimaryActorTick.bCanEverTick = true;
    // Настраиваем капсулу столкновения
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCapsuleComponent()->SetUseCCD(true); // Включаем непрерывное определение столкновений

    // Настраиваем компонент меша
    USkeletalMeshComponent* MeshComponent = GetMesh();
    MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -96.0f));
    MeshComponent->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    // Настраиваем компонент движения персонажа
    UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    MovementComponent->bOrientRotationToMovement = false; // Мы будем управлять поворотом самостоятельно
    MovementComponent->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    MovementComponent->GravityScale = 1.8f;
    MovementComponent->JumpZVelocity = 1000.0f;
    MovementComponent->AirControl = 0.8f;
    MovementComponent->MaxWalkSpeed = 600.0f;
    MovementComponent->MaxAcceleration = 2048.0f;
    MovementComponent->BrakingDecelerationFalling = 2048.0f;
    MovementComponent->bUseControllerDesiredRotation = false;

    // Создаем стрелу камеры
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = false;
    CameraBoom->bInheritPitch = true;
    CameraBoom->bInheritYaw = true;
    CameraBoom->bInheritRoll = false;

    // Настройки стабилизации камеры
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 10.0f;
    CameraBoom->CameraLagMaxDistance = 50.0f;
    CameraBoom->bEnableCameraRotationLag = true;
    CameraBoom->CameraRotationLagSpeed = 10.0f;
    CameraBoom->CameraLagMaxTimeStep = 0.05f;
    CameraBoom->bDoCollisionTest = true;

    // Создаем камеру слежения
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // Значения по умолчанию
    JumpPower = 850.0f;
    CameraSensitivity = 2.0f;
    MinPitchAngle = -80.0f;
    MaxPitchAngle = 80.0f;
    CameraRotation = FRotator::ZeroRotator;

    // Настройки масштабирования
    MinZoomDistance = 200.0f;
    MaxZoomDistance = 800.0f;
    ZoomStep = 50.0f;

    // Не используем поворот контроллера, мы будем управлять им самостоятельно
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
    //----------------------------------------------------------------------------------------
    // Усиления
    //----------------------------------------------------------------------------------------

}

//----------------------------------------------------------------------------------------
// МЕТОДЫ ЖИЗНЕННОГО ЦИКЛА
//----------------------------------------------------------------------------------------
void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Инициализируем первый прыжок с задержкой
    GetWorldTimerManager().SetTimer(
        JumpTimerHandle,
        this,
        &APlayerCharacter::PerformJump,
        0.5f,
        false
    );

    // Инициализируем поворот камеры
    if (Controller)
    {
        Controller->SetControlRotation(FRotator(0.0f, GetActorRotation().Yaw, 0.0f));
    }

    // Настраиваем контроллеры для корректной работы орбитальной камеры
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        PC->bAutoManageActiveCameraTarget = false;
    }

    // Настраиваем CameraBoom для использования контроллера
    if (CameraBoom)
    {
        CameraBoom->bUsePawnControlRotation = true;
    }

}

// Добавьте вызов UpdateHeight в метод Tick персонажа:
void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Поворачиваем персонажа в соответствии с камерой
    if (Controller)
    {
        FRotator ControlRotation = Controller->GetControlRotation();
        SetActorRotation(FRotator(0.0f, ControlRotation.Yaw, 0.0f));
    }

    // Обновляем высоту персонажа в GameState каждый кадр
    UpdateHeight();
  
}
//----------------------------------------------------------------------------------------
// НАСТРОЙКА ВВОДА
//----------------------------------------------------------------------------------------

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Привязки движения
    PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);

    // Привязки управления камерой через контроллер напрямую
    PlayerInputComponent->BindAxis("Turn", this, &APlayerCharacter::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp", this, &APlayerCharacter::AddControllerPitchInput);

    // Привязка масштабирования
    PlayerInputComponent->BindAxis("CameraZoom", this, &APlayerCharacter::ZoomCamera);
}

//----------------------------------------------------------------------------------------
// ПЕРЕМЕЩЕНИЕ ПЕРСОНАЖА
//----------------------------------------------------------------------------------------


void APlayerCharacter::MoveForward(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        // Получаем направление "вперед" из контроллера
        const FRotator ControlRotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, ControlRotation.Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

        // Применяем движение
        AddMovementInput(Direction, Value);
        LastMovementInput.X = Value;
    }
    else
    {
        LastMovementInput.X = 0.0f;
    }
}

void APlayerCharacter::MoveRight(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        // Получаем направление "вправо" из контроллера
        const FRotator ControlRotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, ControlRotation.Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // Применяем движение
        AddMovementInput(Direction, Value);
        LastMovementInput.Y = Value;
    }
    else
    {
        LastMovementInput.Y = 0.0f;
    }
}


//----------------------------------------------------------------------------------------
// УПРАВЛЕНИЕ КАМЕРОЙ
//----------------------------------------------------------------------------------------

void APlayerCharacter::MouseTurn(float Rate)
{
    // Всегда активная орбитальная камера (не требуется зажатие ПКМ)
    if (Rate != 0.0f)
    {
        // Плавное вращение по Yaw с учетом чувствительности
        float DeltaYaw = Rate * CameraSensitivity;
        DeltaYaw = FMath::Clamp(DeltaYaw, -5.0f, 5.0f);
        CameraRotation.Yaw += DeltaYaw;

        // Немедленно применяем поворот к персонажу
        SetActorRotation(FRotator(0.0f, CameraRotation.Yaw, 0.0f));
    }
}

void APlayerCharacter::MouseLookUp(float Rate)
{
    // Всегда активная орбитальная камера (не требуется зажатие ПКМ)
    if (Rate != 0.0f)
    {
        // Плавное вращение по Pitch с учетом чувствительности и ограничений
        float DeltaPitch = Rate * CameraSensitivity;
        DeltaPitch = FMath::Clamp(DeltaPitch, -5.0f, 5.0f);

        // Применяем ограничения Pitch
        CameraRotation.Pitch = FMath::Clamp(
            CameraRotation.Pitch + DeltaPitch,
            MinPitchAngle,
            MaxPitchAngle
        );
    }
}

void APlayerCharacter::ZoomCamera(float AxisValue)
{
    if (CameraBoom && AxisValue != 0.0f)
    {
        // Вычисляем новую длину камеры (отрицательное значение для колеса мыши - приближение)
        float NewTargetArmLength = CameraBoom->TargetArmLength - (ZoomStep * AxisValue);

        // Ограничиваем минимальными и максимальными значениями
        NewTargetArmLength = FMath::Clamp(NewTargetArmLength, MinZoomDistance, MaxZoomDistance);

        // Устанавливаем новую длину
        CameraBoom->TargetArmLength = NewTargetArmLength;
    }
}
// Добавляем новый метод для получения направлений относительно камеры
FVector APlayerCharacter::GetDirectionFromCamera(EAxis::Type Axis) const
{
    // Используем только Yaw-компоненту вращения камеры
    const FRotator YawRotation(0, CameraRotation.Yaw, 0);

    // Получаем направление по указанной оси из матрицы вращения
    return FRotationMatrix(YawRotation).GetUnitAxis(Axis);
}
void APlayerCharacter::AddControllerYawInput(float Value)
{
    if (Value != 0.0f)
    {
        // Применяем чувствительность
        float ScaledValue = Value * CameraSensitivity;
        Super::AddControllerYawInput(ScaledValue);
    }
}

// Настраиваем чувствительность наклона камеры с ограничениями
void APlayerCharacter::AddControllerPitchInput(float Value)
{
    if (Value == 0.0f || !Controller)
        return;

    // Применяем чувствительность
    float ScaledValue = Value * CameraSensitivity;

    // Получаем текущий поворот камеры
    FRotator CurrentRotation = Controller->GetControlRotation();

    // Нормализуем угол Pitch в диапазон от -90 до +90
    float CurrentPitch = CurrentRotation.Pitch;
    if (CurrentPitch > 180.0f)
    {
        CurrentPitch = CurrentPitch - 360.0f;
    }

    // Создаем фактор масштабирования, который уменьшает скорость движения
    // по мере приближения к пределам, но никогда не блокирует полностью
    float ScaleFactor = 1.0f;

    // Если движемся вниз и близки к нижнему пределу
    if (ScaledValue < 0.0f && CurrentPitch < MinPitchAngle + 5.0f)
    {
        // Постепенно уменьшаем масштаб при приближении к пределу
        ScaleFactor = FMath::Max(0.1f, (CurrentPitch - MinPitchAngle + 5.0f) / 5.0f);
    }
    // Если движемся вверх и близки к верхнему пределу
    else if (ScaledValue > 0.0f && CurrentPitch > MaxPitchAngle - 5.0f)
    {
        // Постепенно уменьшаем масштаб при приближении к пределу
        ScaleFactor = FMath::Max(0.1f, (MaxPitchAngle - CurrentPitch + 5.0f) / 5.0f);
    }

    // Применяем масштабирование и поворот
    Super::AddControllerPitchInput(ScaledValue * ScaleFactor);
}
//----------------------------------------------------------------------------------------
// СИСТЕМА ПРЫЖКОВ
//----------------------------------------------------------------------------------------

void APlayerCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);

    // Воспроизводим звук приземления
    PlayCharacterSound(LandSound);

    // Планируем следующий прыжок с небольшой задержкой
    // Задержка в 0.1 секунды необходима для того, чтобы:
    // 1. Дать персонажу стабилизироваться после приземления
    // 2. Избежать "залипания" в земле из-за слишком раннего прыжка
    // 3. Создать естественную паузу между прыжками
    GetWorldTimerManager().SetTimer(
        JumpTimerHandle,
        this,
        &APlayerCharacter::PerformJump,
        0.1f,
        false
    );
}

void APlayerCharacter::PerformJump()
{
    // Проверяем, находимся ли мы на земле
    if (GetCharacterMovement()->IsMovingOnGround())
    {
        // Очищаем горизонтальную скорость перед прыжком
        // для более контролируемой траектории
        FVector Velocity = GetCharacterMovement()->Velocity;
        Velocity.X = 0.0f;
        Velocity.Y = 0.0f;
        GetCharacterMovement()->Velocity = Velocity;

        // Воспроизводим звук прыжка
        PlayCharacterSound(JumpSound);

        // Вызываем базовую функцию Jump
        Jump();

        // Устанавливаем только вертикальную скорость для
        // стабильной траектории прыжка
        GetCharacterMovement()->Velocity = FVector(0.0f, 0.0f, JumpPower);
    }
}

//----------------------------------------------------------------------------------------
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
//----------------------------------------------------------------------------------------

void APlayerCharacter::PlayCharacterSound(USoundBase* Sound)
{
    if (Sound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            Sound,
            GetActorLocation()
        );
    }
}

//----------------------------------------------------------------------------------------
// МЕТОДЫ ИГРОВОЙ СТАТИСТИКИ
//----------------------------------------------------------------------------------------

void APlayerCharacter::AddScore(int32 Points)
{
    // Получаем GameState и используем его методы
    if (AWTowerGameState* GameState = Cast<AWTowerGameState>(GetWorld()->GetGameState()))
    {
        GameState->AddScore(Points);
    }
}

int32 APlayerCharacter::GetScore() const
{
    // Получаем счет из GameState
    if (AWTowerGameState* GameState = Cast<AWTowerGameState>(GetWorld()->GetGameState()))
    {
        return GameState->GetScore();
    }
    return 0;
}

float APlayerCharacter::GetGameTime() const
{
    // Получаем время игры из GameState
    if (AWTowerGameState* GameState = Cast<AWTowerGameState>(GetWorld()->GetGameState()))
    {
        return GameState->GetGameTime();
    }
    return 0.0f;
}

float APlayerCharacter::GetMaxHeight() const
{
    // Получаем максимальную высоту из GameState
    if (AWTowerGameState* GameState = Cast<AWTowerGameState>(GetWorld()->GetGameState()))
    {
        return GameState->GetPlayerMaxHeight();
    }
    return 0.0f;
}

void APlayerCharacter::CompleteGame()
{
    // Устанавливаем флаг завершения игры в GameState
    if (AWTowerGameState* GameState = Cast<AWTowerGameState>(GetWorld()->GetGameState()))
    {
        GameState->SetGameCompleted();
    }
}

void APlayerCharacter::UpdateHeight()
{
    // Обновляем текущую высоту персонажа в GameState
    if (AWTowerGameState* GameState = Cast<AWTowerGameState>(GetWorld()->GetGameState()))
    {
        GameState->UpdatePlayerHeight(GetActorLocation().Z);
    }
}


//----------------------------------------------------------------------------------------
// МЕТОДЫ УСИЛЕНИЙ
//----------------------------------------------------------------------------------------
// Добавьте в существующий файл PlayerCharacter.cpp

void APlayerCharacter::DisplayActivePowerUp(EPowerUpType PowerUpType, float Duration)
{
    // Сохраняем информацию об активном усилении в персонаже
    ActivePowerUps.Add(PowerUpType, true);

    // Отображаем усиление на HUD
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC)
    {
        AWTowerHUD* HUD = Cast<AWTowerHUD>(PC->GetHUD());
        if (HUD)
        {
            HUD->ShowPowerUp(PowerUpType, Duration);
        }
    }

    // Если задана длительность, настраиваем таймер для отключения визуального эффекта
    if (Duration > 0.0f)
    {
        FTimerHandle& TimerHandle = ActivePowerUpTimers.FindOrAdd(PowerUpType);

        GetWorldTimerManager().ClearTimer(TimerHandle);
        GetWorldTimerManager().SetTimer(
            TimerHandle,
            FTimerDelegate::CreateLambda([this, PowerUpType]() {
                // По истечении таймера отмечаем усиление как неактивное
                ActivePowerUps.Add(PowerUpType, false);

                // Скрываем индикатор усиления на HUD
                APlayerController* PC = Cast<APlayerController>(GetController());
                if (PC)
                {
                    AWTowerHUD* HUD = Cast<AWTowerHUD>(PC->GetHUD());
                    if (HUD)
                    {
                        HUD->HidePowerUp(PowerUpType);
                    }
                }

                // Уведомляем UI о деактивации усиления
                NotifyPowerUpDeactivated(PowerUpType);
                }),
            Duration,
            false
        );
    }
}