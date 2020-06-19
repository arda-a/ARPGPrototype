// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Collider.generated.h"

UCLASS()
class ARPGPROTOTYPE_API ACollider : public APawn
{
    GENERATED_BODY()

public:
    // Sets default values for this pawn's properties
    ACollider();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    virtual UPawnMovementComponent* GetMovementComponent() const override;

    UPROPERTY(VisibleAnywhere, Category = "Mesh")
        class UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, Category = "Mesh")
        class USphereComponent* SphereComponent;

    UPROPERTY(VisibleAnywhere, Category = "Mesh")
        class UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere, Category = "Mesh")
        class USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
        class UColliderMovementComponent* OurMovementComponent;

    FORCEINLINE UStaticMeshComponent* GetMeshComponent() { return MeshComponent; }
    FORCEINLINE void SetMeshComponent(UStaticMeshComponent* mesh) { MeshComponent = mesh; }

    FORCEINLINE USphereComponent* GetSphereComponent() { return SphereComponent; }
    FORCEINLINE void SetSphereComponent(USphereComponent* sphere) { SphereComponent = sphere; }

    FORCEINLINE UCameraComponent* GetCameraComponent() { return Camera; }
    FORCEINLINE void SetCameraComponent(UCameraComponent* camera) { Camera = camera; }

    FORCEINLINE USpringArmComponent* GetSpringArmComponent() { return SpringArm; }
    FORCEINLINE void SetSpringArmComponent(USpringArmComponent* springArm) { SpringArm = springArm; }

private:
    void MoveForward(float value);
    void MoveRight(float value);
    void CameraPitch(float axisValue);
    void CameraYaw(float axisValue);

    FVector2D m_cameraInput;
};
