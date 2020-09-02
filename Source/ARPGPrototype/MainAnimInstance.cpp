// Fill out your copyright notice in the Description page of Project Settings.


#include "MainAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"

void UMainAnimInstance::NativeInitializeAnimation() {
    if (Pawn == nullptr) {
        Pawn = TryGetPawnOwner();
    }
}

void UMainAnimInstance::UpdateAnimationProperties() {
    if (Pawn == nullptr) {
        Pawn = TryGetPawnOwner();
    }

    if (Pawn) {
        FVector speed = Pawn->GetVelocity();
        FVector lateralSpeed = FVector(speed.X, speed.Y, 0.f);
        MovementSpeed = lateralSpeed.Size();

        bIsInAir = Pawn->GetMovementComponent()->IsFalling();
    }
}
