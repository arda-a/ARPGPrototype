// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values 
AMainCharacter::AMainCharacter()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(GetRootComponent());
    CameraBoom->TargetArmLength = 600.f; //Camera follows at this distance
    CameraBoom->bUsePawnControlRotation = true; //Rotate arm based on the controller

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

    // Attach the camera to the end of the boom and let the boom adjust to match
    // the controller orientation
    FollowCamera->bUsePawnControlRotation = false;

    // Set turn rates for input
    BaseTurnRate = 65.f;
    BaseLookUpRate = 65.f;
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
    Super::BeginPlay();

}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AMainCharacter::MoveForward(float value) {
    if (Controller != nullptr && value != 0.f) {
        //Find out which way is forward
        const FRotator rotation = Controller->GetControlRotation();
        const FRotator yawRotation(0.f, rotation.Yaw, 0.f);

        const FVector direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(direction, value);
    }
}

void AMainCharacter::MoveRight(float value) {
    if (Controller != nullptr && value != 0.f) {
        //Find out which way is forward
        const FRotator rotation = Controller->GetControlRotation();
        const FRotator yawRotation(0.f, rotation.Yaw, 0.f);

        const FVector direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(direction, value);
    }

void AMainCharacter::TurnAtRate(float rate) {
    AddControllerYawInput(rate * BaseLookUpRate * )
    }

void AMainCharacter::LookUpRate(float rate) {

    }
}

