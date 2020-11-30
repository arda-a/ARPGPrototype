// Fill out your copyright notice in the Description page of Project Settings.

#include "MainCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Sound/SoundCue.h"
#include "Enemy.h"

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
    bLMBDown = false;

    // Initialize enums
    MovementStatus = EMovementStatus::EMS_Normal;
    StaminaStatus = EStaminaStatus::ESS_Normal;

    StaminaDrainRate = 25.f;
    MinSprintStamina = 50.f;

    InterpSpeed = 15.f;
    bInterpToEnemy = false;
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

    if (bInterpToEnemy && CombatTarget) {
        FRotator lookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
        FRotator interpRotation = FMath::RInterpTo(GetActorRotation(), lookAtYaw, DeltaTime, InterpSpeed);

        SetActorRotation(interpRotation);
    }
}

FRotator AMainCharacter::GetLookAtRotationYaw(FVector target) {
    FRotator lookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), target);
    FRotator lookAtRotationYaw(0.f, lookAtRotation.Yaw, 0.f);
    return lookAtRotationYaw;
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

    PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMainCharacter::LMBDown);
    PlayerInputComponent->BindAction("LMB", IE_Released, this, &AMainCharacter::LMBUp);

    PlayerInputComponent->BindAxis("MoveForward", this, &AMainCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AMainCharacter::MoveRight);

    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
    PlayerInputComponent->BindAxis("TurnRate", this, &AMainCharacter::TurnAtRate);
    PlayerInputComponent->BindAxis("LookUpRate", this, &AMainCharacter::LookUpAtRate);
}

void AMainCharacter::MoveForward(float value)
{
    if (Controller != nullptr && value != 0.f && !bAttacking)
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

void AMainCharacter::LMBDown() {
    bLMBDown = true;

    if (ActiveOverlappingItem) {
        AWeapon* wep = Cast<AWeapon>(ActiveOverlappingItem);
        if (wep) {
            wep->Equip(this);
            SetActiveOverlappingItem(nullptr);
        }
    }
    else if (EquippedWeapon) {
        Attack();
    }
}

void AMainCharacter::LMBUp() {
    bLMBDown = false;
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

float AMainCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    DecrementHealth(DamageAmount);
    return DamageAmount;
}

void AMainCharacter::IncrementCoins(int32 amount)
{
    Coins += amount;
}

void AMainCharacter::Die()
{
    UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
    if (animInstance && CombatMontage) {
        animInstance->Montage_Play(CombatMontage, 1.f);
        animInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
    }
}

void AMainCharacter::ShowPickupLocations()
{
    for (int32 i = 0; i < PickupLocations.Num(); i++)
    {
        UKismetSystemLibrary::DrawDebugSphere(this, PickupLocations[i], 25.f, 8, FLinearColor::Blue, 10.f, 0.75f);
    }

}

void AMainCharacter::SetEquippedWeapon(AWeapon* weaponToSet) {
    if (EquippedWeapon) {
        EquippedWeapon->Destroy();
    }

    EquippedWeapon = weaponToSet;
}

void AMainCharacter::Attack() {
    if (!bAttacking) {
        bAttacking = true;
        SetInterpToEnemy(true);

        UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
        if (animInstance && CombatMontage) {
            int32 section = FMath::RandRange(0, 1);
            switch (section)
            {
            case 0:
                animInstance->Montage_Play(CombatMontage, 1.75f);
                animInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage);
                break;
            case 1:
                animInstance->Montage_Play(CombatMontage, 1.75f);
                animInstance->Montage_JumpToSection(FName("Attack_2"), CombatMontage);
                break;
            default: break;
            }
        }
    }
}

void AMainCharacter::AttackEnd() {
    bAttacking = false;
    SetInterpToEnemy(false);
    if (bLMBDown) {
        Attack();
    }
}

void AMainCharacter::PlaySwingSound() {
    if (EquippedWeapon->SwingSound) {
        UGameplayStatics::PlaySound2D(this, EquippedWeapon->SwingSound);
    }
}

void AMainCharacter::SetInterpToEnemy(bool interp) {
    bInterpToEnemy = interp;
}

