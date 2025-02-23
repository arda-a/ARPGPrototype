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
#include "MainPlayerController.h"
#include "ARPGSaveGame.h"

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

    bHasCombatTarget = false;

    bMovingForward = false;
    bMovingRight = false;
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
    Super::BeginPlay();

    MainPlayerController = Cast<AMainPlayerController>(GetController());
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (MovementStatus == EMovementStatus::EMS_Dead)
        return;

    float deltaStamina = StaminaDrainRate * DeltaTime;

    switch (StaminaStatus) {
    case EStaminaStatus::ESS_Normal:
        if (bShiftKeyDown && (bMovingForward || bMovingRight)) {
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
        if (bShiftKeyDown && (bMovingForward || bMovingRight)) {
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
        SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering);
        Stamina += deltaStamina;

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

    if (CombatTarget) {
        CombatTargetLocation = CombatTarget->GetActorLocation();
        if (MainPlayerController) {
            MainPlayerController->EnemyLocation = CombatTargetLocation;
        }
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

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMainCharacter::Jump);
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
    bMovingForward = false;

    if (Controller != nullptr && value != 0.f && !bAttacking && Alive())
    {
        //Find out which way is forward
        const FRotator rotation = Controller->GetControlRotation();
        const FRotator yawRotation(0.f, rotation.Yaw, 0.f);

        const FVector direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(direction, value);

        bMovingForward = true;
    }
}

void AMainCharacter::MoveRight(float value)
{
    bMovingRight = false;

    if (Controller != nullptr && value != 0.f && Alive())
    {
        //Find out which way is forward
        const FRotator rotation = Controller->GetControlRotation();
        const FRotator yawRotation(0.f, rotation.Yaw, 0.f);

        const FVector direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(direction, value);

        bMovingRight = true;
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

    if (MovementStatus == EMovementStatus::EMS_Dead) {
        return;
    }

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
    if (Health - DamageAmount <= 0.f)
    {
        Health -= DamageAmount;
        Die();
        if (DamageCauser) {
            AEnemy* enemy = Cast<AEnemy>(DamageCauser);
            if (enemy) {
                enemy->bHasValidTarget = false;
            }
        }
    }
    else
    {
        Health -= DamageAmount;
    }
    return DamageAmount;
}

void AMainCharacter::IncrementCoins(int32 amount)
{
    Coins += amount;
}

void AMainCharacter::IncrementHealth(float amount) {
    if (Health + amount >= MaxHealth) {
        Health = MaxHealth;
    }
    else {
        Health += amount;
    }
}

void AMainCharacter::Die()
{
    if (MovementStatus == EMovementStatus::EMS_Dead)
        return;

    UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
    if (animInstance && CombatMontage) {
        animInstance->Montage_Play(CombatMontage, 1.f);
        animInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
    }
    SetMovementStatus(EMovementStatus::EMS_Dead);
}

bool AMainCharacter::Alive() {
    return MovementStatus != EMovementStatus::EMS_Dead;
}

void AMainCharacter::Jump() {
    if (Alive()) {
        Super::Jump();
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
    if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead) {
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

void AMainCharacter::DeathEnd() {
    GetMesh()->bPauseAnims = true;
    GetMesh()->bNoSkeletonUpdate = true;
}

void AMainCharacter::SetInterpToEnemy(bool interp) {
    bInterpToEnemy = interp;
}

void AMainCharacter::UpdateCombatTarget() {
    TArray<AActor*> OverlappingActors;
    GetOverlappingActors(OverlappingActors, EnemyFilter);

    if (OverlappingActors.Num() == 0) {
        if (MainPlayerController) {
            MainPlayerController->RemoveEnemyHealthBar();
        }
        return;
    }

    AEnemy* closestEnemy = Cast<AEnemy>(OverlappingActors[0]);
    if (closestEnemy) {
        FVector location = GetActorLocation();
        float minDistance = (closestEnemy->GetActorLocation() - location).Size();

        for (auto actor : OverlappingActors) {
            AEnemy* enemy = Cast<AEnemy>(actor);
            if (enemy) {
                float distance = (enemy->GetActorLocation() - location).Size();
                if (distance < minDistance) {
                    minDistance = distance;
                    closestEnemy = enemy;
                }
            }
        }

        if (MainPlayerController) {
            MainPlayerController->DisplayEnemyHealthBar();
        }

        SetCombatTarget(closestEnemy);
        SetHasCombatTarget(true);
    }
}

void AMainCharacter::SwitchLevel(FName levelName) {
    UWorld* world = GetWorld();
    if (world) {
        FString currentLevel = world->GetMapName();

        FName currentLevelName(*currentLevel);
        if (currentLevelName != levelName) {
            UGameplayStatics::OpenLevel(world, levelName);
        }
    }
}

void AMainCharacter::SaveGame() {
    UARPGSaveGame* saveGameInstance = Cast<UARPGSaveGame>(UGameplayStatics::CreateSaveGameObject(UARPGSaveGame::StaticClass()));

    saveGameInstance->CharacterStats.Health = Health;
    saveGameInstance->CharacterStats.MaxHealth = MaxHealth;
    saveGameInstance->CharacterStats.Stamina = Stamina;
    saveGameInstance->CharacterStats.MaxStamina = MaxStamina;
    saveGameInstance->CharacterStats.Coins = Coins;
    saveGameInstance->CharacterStats.Location = GetActorLocation();
    saveGameInstance->CharacterStats.Rotation = GetActorRotation();

    UGameplayStatics::SaveGameToSlot(saveGameInstance, saveGameInstance->PlayerName, saveGameInstance->UserIndex);
}

void AMainCharacter::LoadGame(bool setPosition) {
    UARPGSaveGame* loadGameInstance = Cast<UARPGSaveGame>(UGameplayStatics::CreateSaveGameObject(UARPGSaveGame::StaticClass()));

    loadGameInstance = Cast<UARPGSaveGame>(UGameplayStatics::LoadGameFromSlot(loadGameInstance->PlayerName, loadGameInstance->UserIndex));

    Health = loadGameInstance->CharacterStats.Health;
    MaxHealth = loadGameInstance->CharacterStats.MaxHealth;
    Stamina = loadGameInstance->CharacterStats.Stamina;
    MaxStamina = loadGameInstance->CharacterStats.MaxStamina;
    Coins = loadGameInstance->CharacterStats.Coins;

    if (setPosition) {
        SetActorLocation(loadGameInstance->CharacterStats.Location);
        SetActorRotation(loadGameInstance->CharacterStats.Rotation);
    }

}
