// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MainCharacter.generated.h"

UENUM(BlueprintType)
enum class EMovementStatus : uint8
{
    EMS_Normal UMETA(DisplayName = "Normal"),
    EMS_Sprinting UMETA(DisplayName = "Sprinting"),

    EMS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EStaminaStatus : uint8
{
    ESS_Normal UMETA(DisplayName = "Normal"),
    ESS_BelowMinimum UMETA(DisplayName = "BelowMinimum"),
    ESS_Exhausted UMETA(DisplayName = "Exhausted"),
    ESS_ExhaustedRecovering UMETA(DisplayName = "ExhaustedRecovering"),

    ESS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class ARPGPROTOTYPE_API AMainCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    AMainCharacter();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    class UParticleSystem* HitParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    class USoundCue* HitSound;

    TArray<FVector> PickupLocations;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items")
        class AWeapon* EquippedWeapon;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items")
        class AItem* ActiveOverlappingItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enums")
        EMovementStatus MovementStatus;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
        EStaminaStatus StaminaStatus;

    FORCEINLINE void SetStaminaStatus(EStaminaStatus status) { StaminaStatus = status; };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
        float StaminaDrainRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
        float MinSprintStamina;

    float InterpSpeed;
    bool bInterpToEnemy;
    void SetInterpToEnemy(bool interp);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    class AEnemy* CombatTarget;

    FORCEINLINE void SetCombatTarget(AEnemy* target) { CombatTarget = target; }

    FRotator GetLookAtRotationYaw(FVector target);

    // Set movement status and running speed
    void SetMovementStatus(EMovementStatus status);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
        float RunningSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
        float SprintingSpeed;

    bool bShiftKeyDown;

    // Pressed down to enable sprinting
    void ShiftKeyDown();

    // Released to stop sprinting
    void ShiftKeyUp();

    /// <summary>
    /// Positioning the camera behind the player.
    /// </summary>
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
        class  USpringArmComponent* CameraBoom;

    /**Follow camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
        class UCameraComponent* FollowCamera;

    /** To scale turning functions for the camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
        float BaseTurnRate;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
        float BaseLookUpAtRate;

    /**
     *
     * Player stats
     *
     */

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
        float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
        float Health;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
        float MaxStamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
        float Stamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
        int Coins;

    void DecrementHealth(float amount);

    virtual float TakeDamage(float DamageAmount,
                             struct FDamageEvent const& DamageEvent,
                             class AController* EventInstigator,
                             AActor* DamageCauser) override;

    void IncrementCoins(int32 amount);

    void Die();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Called for forward/backward input
    void MoveForward(float value);

    // Called for side to side input
    void MoveRight(float value);

    /** Called via input to turn at a given rate
    * @param rate This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
    */
    void TurnAtRate(float rate);

    /** Called via input to look up/down at a given rate
    * @param rate This is a normalized rate, i.e. 1.0 means 100% of desired look up/down rate
    */
    void LookUpAtRate(float rate);

    bool bLMBDown;
    void LMBDown();
    void LMBUp();

    FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

    void SetEquippedWeapon(AWeapon* weaponToSet);
    FORCEINLINE void SetActiveOverlappingItem(AItem * item) { ActiveOverlappingItem = item; }

    UFUNCTION(BlueprintCallable)
    void ShowPickupLocations();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
    bool bAttacking;

    void Attack();

    UFUNCTION(BlueprintCallable)
    void AttackEnd();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
    class UAnimMontage* CombatMontage;

    UFUNCTION(BlueprintCallable)
    void PlaySwingSound();  
};
