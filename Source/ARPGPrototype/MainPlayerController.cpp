// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "Blueprint/UserWidget.h"

void AMainPlayerController::BeginPlay(){
    Super::BeginPlay();

    if(HUDOverlayAsset){
        HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayAsset);
    }

    HUDOverlay->AddToViewport();
    HUDOverlay->SetVisibility(ESlateVisibility::Visible);

    if (WEnemyHealthBar) {
        EnemyHealthBar = CreateWidget<UUserWidget>(this, WEnemyHealthBar);
        if (EnemyHealthBar) {
            EnemyHealthBar->AddToViewport();
            EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
            
            FVector2D alignmentVec(0.f, 0.f);
            EnemyHealthBar->SetAlignmentInViewport(alignmentVec);
        }
    }
}

void AMainPlayerController::DisplayEnemyHealthBar() {
    if (EnemyHealthBar) {
        bEnemyHealthBarVisible = true;
        EnemyHealthBar->SetVisibility(ESlateVisibility::Visible);
    }
}

void AMainPlayerController::RemoveEnemyHealthBar() {
    if (EnemyHealthBar) {
        bEnemyHealthBarVisible = false;
        EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
    }
}

void AMainPlayerController::Tick(float deltaTime) {
    Super::Tick(deltaTime);

    if (EnemyHealthBar) {
        FVector2D positionInViewport;
        ProjectWorldLocationToScreen(EnemyLocation, positionInViewport);
        positionInViewport.Y -= 100.f;

        FVector2D sizeInViewport(200.f, 25.f);

        EnemyHealthBar->SetPositionInViewport(positionInViewport);
        EnemyHealthBar->SetDesiredSizeInViewport(sizeInViewport);
    }
}

