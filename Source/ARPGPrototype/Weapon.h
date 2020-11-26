// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
    EWS_Pickup   UMETA(DisplayName = "Pickup"),
    EWS_Equipped UMETA(DisplayName = "Equipped"),

    EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

/**
 *
 */
UCLASS()
class ARPGPROTOTYPE_API AWeapon : public AItem
{
    GENERATED_BODY()

public:

    AWeapon();

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item")
    EWeaponState WeaponState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Particles")
    bool bWeaponParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Sound")
    class USoundCue* OnEquipSound;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SkeletalMesh")
    class USkeletalMeshComponent* SkeletalMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item | Combat")
    class UBoxComponent* CombatCollision;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item | Combat")
    float Damage;

protected:

    virtual void BeginPlay() override;

public:

    virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
    
    virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

    void Equip(class AMainCharacter* character);

    FORCEINLINE void SetWeaponState(EWeaponState state) { WeaponState = state; }
    FORCEINLINE EWeaponState GetWeaponState() { return WeaponState; }

    UFUNCTION()
    void OnCombatOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnCombatOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION(BlueprintCallable)
    void ActivateCollision();

    UFUNCTION(BlueprintCallable)
    void DeactivateCollision();
};
