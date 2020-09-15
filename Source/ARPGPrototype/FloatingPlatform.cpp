// Fill out your copyright notice in the Description page of Project Settings.


#include "FloatingPlatform.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"

// Sets default values
AFloatingPlatform::AFloatingPlatform()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;

    StartPoint = FVector(0.f);
    EndPoint = FVector(0.f);

    bInterping = false;

    InterpSpeed = 4.f;
    InterpTime = 1.f;
}

// Called when the game starts or when spawned
void AFloatingPlatform::BeginPlay()
{
    Super::BeginPlay();

    StartPoint = GetActorLocation();
    EndPoint += StartPoint;

    GetWorldTimerManager().SetTimer(InterpTimer, this, &AFloatingPlatform::ToggleInterping, InterpTime);

    m_distance = (EndPoint - StartPoint).Size();
}

// Called every frame
void AFloatingPlatform::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bInterping) {
        FVector currentLocation = GetActorLocation();
        FVector interp = FMath::VInterpTo(currentLocation, EndPoint, DeltaTime, InterpSpeed);
        SetActorLocation(interp);

        float distanceTraveled = (GetActorLocation() - StartPoint).Size();
        if (m_distance - distanceTraveled <= 1.f) {
            ToggleInterping();
            GetWorldTimerManager().SetTimer(InterpTimer, this, &AFloatingPlatform::ToggleInterping, InterpTime);
            std::swap(StartPoint, EndPoint);
        }
    }
}

void AFloatingPlatform::ToggleInterping()
{
    bInterping = !bInterping;
}

