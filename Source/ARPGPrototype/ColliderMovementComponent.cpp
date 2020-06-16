// Fill out your copyright notice in the Description page of Project Settings.


#include "ColliderMovementComponent.h"

void UColliderMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!PawnOwner || !UpdatedComponent || ShouldSkipUpdate(DeltaTime)) {
        return;
    }

    FVector desiredMovementThisFrame = ConsumeInputVector().GetClampedToMaxSize(1.0f);

    if (!desiredMovementThisFrame.IsNearlyZero()) {
        FHitResult hit;
        SafeMoveUpdatedComponent(desiredMovementThisFrame, UpdatedComponent->GetComponentRotation(), true, hit);

        // If we bump into something, slide along the surface.
        if (hit.IsValidBlockingHit()) {
            SlideAlongSurface(desiredMovementThisFrame, 1.f - hit.Time, hit.Normal, hit);
            UE_LOG(LogTemp, Warning, TEXT("Valid blocking hit !"));
        }
    }
}
