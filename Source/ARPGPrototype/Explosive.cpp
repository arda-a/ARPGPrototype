// Fill out your copyright notice in the Description page of Project Settings.

#include "Explosive.h"
#include "MainCharacter.h"
#include "Enemy.h"
#include "Kismet/GameplayStatics.h"

AExplosive::AExplosive()
{
    Damage = 15.f;
}

void AExplosive::OnOverlapBegin(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
    if (OtherActor)
    {
        AMainCharacter *main = Cast<AMainCharacter>(OtherActor);
        AEnemy* enemy = Cast<AEnemy>(OtherActor);
        if (main || enemy)
        {
            Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
            
            UGameplayStatics::ApplyDamage(OtherActor, Damage, nullptr, this, DamageTypeClass);
            
            Destroy();
        }
    }
}

void AExplosive::OnOverlapEnd(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex)
{
    Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
    UE_LOG(LogTemp, Warning, TEXT("Explosive:::OnOverlapEnd"));
}
