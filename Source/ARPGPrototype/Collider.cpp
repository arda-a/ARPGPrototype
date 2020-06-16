// Fill out your copyright notice in the Description page of Project Settings.


#include "Collider.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "ColliderMovementComponent.h"

// Sets default values
ACollider::ACollider()
{
    // Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
    SetRootComponent(SphereComponent);

    SphereComponent->InitSphereRadius(40.f);
    SphereComponent->SetCollisionProfileName(TEXT("Pawn"));

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(GetRootComponent());
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshComponentAsset(TEXT("StaticMesh'/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere'"));

    if (MeshComponentAsset.Succeeded()) {
        MeshComponent->SetStaticMesh(MeshComponentAsset.Object);
        MeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -40.f));
        MeshComponent->SetWorldScale3D(FVector(0.8f, 0.8f, 0.8f));
    }

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(GetRootComponent());
    SpringArm->RelativeRotation = FRotator(-45.f, 0.f, 0.f);
    SpringArm->TargetArmLength = 400.f;
    SpringArm->bEnableCameraLag = true;
    SpringArm->CameraLagSpeed = 3.f;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

    OurMovementComponent = CreateDefaultSubobject<UColliderMovementComponent>(TEXT("OurMovementComponent"));
    OurMovementComponent->UpdatedComponent = RootComponent;

    AutoPossessPlayer = EAutoReceiveInput::Player0;
}

// Called when the game starts or when spawned
void ACollider::BeginPlay()
{
    Super::BeginPlay();

}

// Called every frame
void ACollider::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACollider::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ACollider::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ACollider::MoveRight);
}

UPawnMovementComponent* ACollider::GetMovementComponent() const
{
    return OurMovementComponent;
}

void ACollider::MoveForward(float input)
{
    FVector forward = GetActorForwardVector();
    if (OurMovementComponent)
    {
        OurMovementComponent->AddInputVector(input * forward);
    }
}

void ACollider::MoveRight(float input)
{
    FVector right = GetActorRightVector();
    if (OurMovementComponent)
    {
        OurMovementComponent->AddInputVector(input * right);
    }
}

