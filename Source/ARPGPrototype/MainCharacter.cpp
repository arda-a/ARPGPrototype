// Fill out your copyright notice in the Description page of Project Settings.

#include "MainCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AMainCharacter::AMainCharacter()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(GetRootComponent());
    CameraBoom->TargetArmLength = 600.f;        //Camera follows at this distance
    CameraBoom->bUsePawnControlRotation = true; //Rotate arm based on the controller

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

    // Attach the camera to the end of the boom and let the boom adjust to match
    // the controller orientation
    FollowCamera->bUsePawnControlRotation = false;

    // Set turn rates for input
    BaseTurnRate = 65.f;
    BaseLookUpAtRate = 65.f;

    // We don't want to rotate the character along with the rotation
    // Let that just affect the camera.
    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true;         //Character moves towards the direction of the input
    GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); //Character moves at this rotation rate
    GetCharacterMovement()->JumpZVelocity = 650.f;
    GetCharacterMovement()->AirControl = 0.2f;

    MaxHealth = 100.f;
    Health = 65.f;
    MaxStamina = 150.f;
    Stamina = 120.f;
    Coins = 0;

    RunningSpeed = 400.f;
    SprintingSpeed = 750.f;

    bShiftKeyDown = false;

    // Initialize enums
    MovementStatus = EMovementStatus::EMS_Normal;
    StaminaStatus = EStaminaStatus::ESS_Normal;

    StaminaDrainRate = 25.f;
    MinSprintStamina = 50.f;
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    float deltaStamina = StaminaDrainRate * DeltaTime;

    switch (StaminaStatus) {
    case EStaminaStatus::ESS_Normal:
        if (bShiftKeyDown) {
            if (Stamina - deltaStamina <= MinSprintStamina) {
                SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum);
                Stamina -= deltaStamina;
            }
            else {
                Stamina -= deltaStamina;
            }

            SetMovementStatus(EMovementStatus::EMS_Sprinting);
        }
        else { //Shift key up
            if (Stamina + deltaStamina >= MaxStamina) {
                Stamina = MaxStamina;
            }
            else {
                Stamina += deltaStamina;
            }

            SetMovementStatus(EMovementStatus::EMS_Normal);
        }
        break;
    case EStaminaStatus::ESS_BelowMinimum:
        if (bShiftKeyDown) {
            if (Stamina - deltaStamina <= 0.f) {
                Stamina = 0.f;
                SetStaminaStatus(EStaminaStatus::ESS_Exhausted);
                SetMovementStatus(EMovementStatus::EMS_Normal);
            }
            else {
                Stamina -= deltaStamina;
                SetMovementStatus(EMovementStatus::EMS_Sprinting);
            }
        }
        else { // shift key up
            if (Stamina + deltaStamina >= MinSprintStamina) {
                SetStaminaStatus(EStaminaStatus::ESS_Normal);
                Stamina += deltaStamina;
            }
            else {
                Stamina += deltaStamina;
            }

            SetMovementStatus(EMovementStatus::EMS_Normal);
        }
        break;
    case EStaminaStatus::ESS_Exhausted:
        if (bShiftKeyDown) {
            Stamina = 0.f;
        }
        else { // shift key up
            SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering);
            Stamina += deltaStamina;
        }

        SetMovementStatus(EMovementStatus::EMS_Normal);
        break;
    case EStaminaStatus::ESS_ExhaustedRecovering:
        if (Stamina + deltaStamina >= MinSprintStamina) {
            SetStaminaStatus(EStaminaStatus::ESS_Normal);
            Stamina += deltaStamina;
        }
        else {
            Stamina += deltaStamina;
        }

        SetMovementStatus(EMovementStatus::EMS_Normal);
        break;
    default:break;
    }
}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    check(PlayerInputComponent);

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

    PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMainCharacter::ShiftKeyDown);
    PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMainCharacter::ShiftKeyUp);

    PlayerInputComponent->BindAxis("MoveForward", this, &AMainCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AMainCharacter::MoveRight);

    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
    PlayerInputComponent->BindAxis("TurnRate", this, &AMainCharacter::TurnAtRate);
    PlayerInputComponent->BindAxis("LookUpRate", this, &AMainCharacter::LookUpAtRate);
}

void AMainCharacter::MoveForward(float value)
{
    if (Controller != nullptr && value != 0.f)
    {
        //Find out which way is forward
        const FRotator rotation = Controller->GetControlRotation();
        const FRotator yawRotation(0.f, rotation.Yaw, 0.f);

        const FVector direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(direction, value);
    }
}

void AMainCharacter::MoveRight(float value)
{
    if (Controller != nullptr && value != 0.f)
    {
        //Find out which way is forward
        const FRotator rotation = Controller->GetControlRotation();
        const FRotator yawRotation(0.f, rotation.Yaw, 0.f);

        const FVector direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(direction, value);
    }
}

void AMainCharacter::TurnAtRate(float rate)
{
    AddControllerYawInput(rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMainCharacter::LookUpAtRate(float rate)
{
    AddControllerPitchInput(rate * BaseLookUpAtRate * GetWorld()->GetDeltaSeconds());
}

void AMainCharacter::SetMovementStatus(EMovementStatus status)
{
    MovementStatus = status;
    if (MovementStatus == EMovementStatus::EMS_Sprinting) {
        GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
    }
    else {
        GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
    }
}

void AMainCharacter::ShiftKeyDown()
{
    bShiftKeyDown = true;
}

void AMainCharacter::ShiftKeyUp()
{
    bShiftKeyDown = false;
}

void AMainCharacter::DecrementHealth(float amount)
{
    if (Health - amount <= 0.f)
    {
        Die();
    }
    else
    {
        Health -= amount;
    }
}

void AMainCharacter::IncrementCoins(int32 amount)
{
    Coins += amount;
}

void AMainCharacter::Die()
{
    //TODO
}
