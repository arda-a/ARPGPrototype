// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAnimInstance.h"
#include "Enemy.h"

void UEnemyAnimInstance::NativeInitializeAnimation() {
    if (Pawn == nullptr) {
        Pawn = TryGetPawnOwner();
        if (Pawn) {
            Enemy = Cast<AEnemy>(Pawn);
        }
    }
}

void UEnemyAnimInstance::UpdateAnimationProperties() {
    if (Pawn == nullptr) {
        Pawn = TryGetPawnOwner();
        if (Pawn) {
            FVector speed = Pawn->GetVelocity();
            FVector lateralSpeed = FVector(speed.X, speed.Y, 0.f);
            MovementSpeed = lateralSpeed.Size();

            Enemy = Cast<AEnemy>(Pawn);
        }
    }

    if (Pawn) {
        FVector speed = Pawn->GetVelocity();
        FVector lateralSpeed = FVector(speed.X, speed.Y, 0.f);
        MovementSpeed = lateralSpeed.Size();
    }
}
