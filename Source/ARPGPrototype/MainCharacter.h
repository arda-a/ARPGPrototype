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

    TArray<FVector> PickupLocations;

    UFUNCTION(BlueprintCallable)
    void ShowPickupLocations();

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
    EMovementStatus MovementStatus;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
    EStaminaStatus StaminaStatus;

    FORCEINLINE void SetStaminaStatus(EStaminaStatus status) { StaminaStatus = status; };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float StaminaDrainRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MinSprintStamina;

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

    FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
