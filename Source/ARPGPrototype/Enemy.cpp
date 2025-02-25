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
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "MainPlayerController.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	AgroSphere->InitSphereRadius(600.f);
	AgroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(75.f);

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	CombatCollision->SetupAttachment(GetMesh(), FName("EnemySocket"));

	bOverlappingCombatSphere = false;
	bAttacking = false;

	Health = 25.f;
	MaxHealth = 100.f;
	Damage = 10.f;

	AttackMinTime = 0.5f;
	AttackMaxTime = 3.5f;

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;

	DeathDelay = 3.f;

	bHasValidTarget = false;
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
	
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
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
	if (OtherActor && Alive()) {
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
			bHasValidTarget = false;
			if (main->CombatTarget == this) {
				main->SetCombatTarget(nullptr);
				main->SetHasCombatTarget(false);
			}
			main->UpdateCombatTarget();
			

			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle);
			if (AIController) {
				AIController->StopMovement();
			}
		}
	}
}

void AEnemy::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (OtherActor && Alive()) {
		AMainCharacter* main = Cast<AMainCharacter>(OtherActor);
		if (main) {
			bHasValidTarget = true;
			main->SetCombatTarget(this);
			main->SetHasCombatTarget(true);
			main->UpdateCombatTarget();

			CombatTarget = main;
			bOverlappingCombatSphere = true;
			
			float attackTime = FMath::RandRange(AttackMinTime, AttackMaxTime);
			GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, attackTime);
		}
	}
}

void AEnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	if (OtherActor && OtherComp) {
		AMainCharacter* main = Cast<AMainCharacter>(OtherActor);
		if (main) {
			bOverlappingCombatSphere = false;
			MoveToTarget(main);
			CombatTarget = nullptr;

			if (main->CombatTarget == this) {
				main->SetCombatTarget(nullptr);
				main->bHasCombatTarget = false;
				main->UpdateCombatTarget();
			}

			if (main->MainPlayerController) {
				USkeletalMeshComponent* mainMesh = Cast<USkeletalMeshComponent>(OtherComp);
				if (mainMesh) {
					main->MainPlayerController->RemoveEnemyHealthBar();
				}
			}

			GetWorldTimerManager().ClearTimer(AttackTimer);
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

			if (DamageTypeClass) {
				UGameplayStatics::ApplyDamage(main, Damage, AIController, this, DamageTypeClass);
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
	if (!Alive() || !bHasValidTarget)
		return;

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
		float attackTime = FMath::RandRange(AttackMinTime, AttackMaxTime);
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, attackTime);
	}
}

void AEnemy::PlaySwingSound() {
	if (SwingSound) {
		UGameplayStatics::PlaySound2D(this, SwingSound);
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) {
	if (Health - DamageAmount < 0.f) {
		Health -= DamageAmount;
		Die(DamageCauser);
	}
	else
		Health -= DamageAmount;

	return DamageAmount;
}

void AEnemy::Die(AActor* causer) {
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && CombatMontage) {
		animInstance->Montage_Play(CombatMontage, 1.35f);
		animInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
	}
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Dead);

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AgroSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AMainCharacter* main = Cast<AMainCharacter>(causer);
	if (main) {
		main->UpdateCombatTarget();
	}
}

void AEnemy::DeathEnd() {
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;

	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::Disappear, DeathDelay);
}

bool AEnemy::Alive() {
	return EnemyMovementStatus != EEnemyMovementStatus::EMS_Dead;
}

void AEnemy::Disappear() {
	Destroy();
}

