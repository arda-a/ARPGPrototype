// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "MainCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

AWeapon::AWeapon()
{
    SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    SkeletalMesh->SetupAttachment(GetRootComponent());

    bWeaponParticles = false;

    WeaponState = EWeaponState::EWS_Pickup;
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
