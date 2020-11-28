// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "MainCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Animation/AnimInstance.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	AgroSphere->InitSphereRadius(600.f);

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(75.f);

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	CombatCollision->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("EnemySocket"));

	bOverlappingCombatSphere = false;
	bAttacking = false;

	Health = 25.f;
	MaxHealth = 100.f;
	Damage = 10.f;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AIController = Cast<AAIController>(GetController());

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapEnd);

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapEnd);

	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnCombatOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemy::OnCombatOverlapEnd);

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (OtherActor) {
		AMainCharacter* main = Cast<AMainCharacter>(OtherActor);
		if (main) {
			MoveToTarget(main);
		}
	}
}

void AEnemy::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	if (OtherActor) {
		AMainCharacter* main = Cast<AMainCharacter>(OtherActor);
		if (main) {
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle);
			if (AIController) {
				AIController->StopMovement();
			}
		}
	}
}

void AEnemy::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (OtherActor) {
		AMainCharacter* main = Cast<AMainCharacter>(OtherActor);
		if (main) {
			CombatTarget = main;
			bOverlappingCombatSphere = true;
			Attack();
		}
	}
}

void AEnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	if (OtherActor) {
		AMainCharacter* main = Cast<AMainCharacter>(OtherActor);
		if (main) {
			bOverlappingCombatSphere = false;
			if (EnemyMovementStatus != EEnemyMovementStatus::EMS_Attacking) {
				MoveToTarget(main);
				CombatTarget = nullptr;
			}
		}
	}
}

void AEnemy::MoveToTarget(AMainCharacter* target) {
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);

	if (AIController) {
		FAIMoveRequest moveReq;
		moveReq.SetGoalActor(target);
		moveReq.SetAcceptanceRadius(AcceptanceRadius);

		FNavPathSharedPtr navPath;

		AIController->MoveTo(moveReq, &navPath);

		/** Draw navigation path points for debugging 
		* 
		TArray<FNavPathPoint> pathPoints = navPath->GetPathPoints();
		for (auto p : pathPoints) {
			FVector location = p.Location;
			UKismetSystemLibrary::DrawDebugSphere(this, location, 25.f, 8, FLinearColor::Blue, 10.f, 1.5f);
		}
		*/
	}
}

void AEnemy::OnCombatOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (OtherActor) {
		AMainCharacter* main = Cast<AMainCharacter>(OtherActor);
		if (main) {
			if (main->HitParticles) {
				const USkeletalMeshSocket* tipSocket = GetMesh()->GetSocketByName("TipSocket");
				if (tipSocket) {
					FVector socketLocation = tipSocket->GetSocketLocation(GetMesh());
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), main->HitParticles, socketLocation, FRotator(0.f), false);
				}
			}

			if (main->HitSound) {
				UGameplayStatics::PlaySound2D(this, main->HitSound);
			}
		}
	}
}

void AEnemy::OnCombatOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
}

void AEnemy::ActivateCollision() {
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateCollision() {
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::Attack() {
	if (AIController) {
		AIController->StopMovement();
		SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);
	}

	if (!bAttacking) {
		bAttacking = true;
		UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
		if (animInstance && CombatMontage) {
			animInstance->Montage_Play(CombatMontage, 1.35f);
			animInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
		}
	}
}

void AEnemy::AttackEnd() {
	bAttacking = false;
	if (bOverlappingCombatSphere) {
		Attack();
	}
}

void AEnemy::PlaySwingSound() {
	if (SwingSound) {
		UGameplayStatics::PlaySound2D(this, SwingSound);
	}
}

