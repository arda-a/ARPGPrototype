// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Enemy.h"
#include "Components/SkeletalMeshComponent.h"
#include "MainCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/SkeletalMeshSocket.h"

AWeapon::AWeapon()
{
    SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    SkeletalMesh->SetupAttachment(GetRootComponent());

    CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
    CombatCollision->SetupAttachment(GetRootComponent());

    bWeaponParticles = false;

    WeaponState = EWeaponState::EWS_Pickup;

    Damage = 25.f;
}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();

    CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnCombatOverlapBegin);
    CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnCombatOverlapEnd);

    CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
}

void AWeapon::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
    if (WeaponState == EWeaponState::EWS_Pickup && OtherActor) {
        AMainCharacter* main = Cast<AMainCharacter>(OtherActor);
        if (main) {
            main->SetActiveOverlappingItem(this);
        }
    }
}

void AWeapon::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
    if (OtherActor) {
        AMainCharacter* main = Cast<AMainCharacter>(OtherActor);
        if (main) {
            main->SetActiveOverlappingItem(nullptr);
        }
    }
}

void AWeapon::Equip(AMainCharacter* character)
{
    if (character) {
        SetInstigator(character->GetController());

        SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
        SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

        SkeletalMesh->SetSimulatePhysics(false);

        const USkeletalMeshSocket* RightHandSocket = character->GetMesh()->GetSocketByName("RightHandSocket");
        if (RightHandSocket) {
            RightHandSocket->AttachActor(this, character->GetMesh());
            bRotate = false;

            character->SetEquippedWeapon(this);
            character->SetActiveOverlappingItem(nullptr);
        }

        if (OnEquipSound) {
            UGameplayStatics::PlaySound2D(this, OnEquipSound);
        }
        if (!bWeaponParticles) {
            IdleParticlesComponent->Deactivate();
        }
    }
}

void AWeapon::OnCombatOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
    if (OtherActor) {
        AEnemy* enemy = Cast<AEnemy>(OtherActor);
        if (enemy) {
            if (enemy->HitParticles) {
                const USkeletalMeshSocket* wepSocket = SkeletalMesh->GetSocketByName("WeaponSocket");
                if (wepSocket) {
                    FVector socketLocation = wepSocket->GetSocketLocation(SkeletalMesh);
                    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), enemy->HitParticles, socketLocation, FRotator(0.f), false);
                }
            }

            if (enemy->HitSound) {
                UGameplayStatics::PlaySound2D(this, enemy->HitSound);
            }

            if (DamageTypeClass) {
                UGameplayStatics::ApplyDamage(enemy, Damage, WeaponInstigator, this, DamageTypeClass);
            }
        }
    }
}

void AWeapon::OnCombatOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {

}

void AWeapon::ActivateCollision() {
    CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AWeapon::DeactivateCollision() {
    CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

