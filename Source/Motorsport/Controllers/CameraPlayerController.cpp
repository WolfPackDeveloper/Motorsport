// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraPlayerController.h"
#include "Motorsport/Pawns/CameraPlayerPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h" // Debug strings
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"

ACameraPlayerController::ACameraPlayerController()
{
	FocusedActor = nullptr;
	bMouseButtonIsPressed = false;
	Margin = 10;
	FlipThroughSpeed = 20.f;

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	DefaultMouseCursor = EMouseCursor::GrabHand;
}

void ACameraPlayerController::MouseLeftClick()
{
	bMouseButtonIsPressed = true;
	FocusedActor = nullptr;

	FHitResult HitResult;
	bool bIsHit = GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, HitResult);

	if (bIsHit)
	{
		FocusedActor = Cast<AStaticMeshActor>(HitResult.GetActor());
		DefaultMouseCursor = EMouseCursor::GrabHandClosed;
	}
}

void ACameraPlayerController::MouseLeftReleased()
{
	bMouseButtonIsPressed = false;
	DefaultMouseCursor = EMouseCursor::GrabHand;
}

void ACameraPlayerController::ZoomIn()
{
	if (IsValid(PlayerCamera))
	{
		PlayerCamera->SpringArm->TargetArmLength -= ZoomRate;
	}
}

void ACameraPlayerController::ZoomOut()
{
	PlayerCamera->SpringArm->TargetArmLength += ZoomRate;
}

FVector ACameraPlayerController::MoveScreen()
{
	float MouseCursorX = 0.f;
	float MouseCursorY = 0.f;

	float CameraDirectionX = 0.f;
	float CameraDirectionY = 0.f;

	GetMousePosition(MouseCursorX, MouseCursorY);

	if (MouseCursorX <= Margin) { CameraDirectionY = -FlipThroughSpeed; }
	if (MouseCursorX >= ScreenSizeX - Margin) { CameraDirectionY = FlipThroughSpeed; }
	if (MouseCursorY <= Margin) { CameraDirectionX = FlipThroughSpeed; }
	if (MouseCursorY >= ScreenSizeY - Margin) { CameraDirectionX = -FlipThroughSpeed; }

	return FVector(CameraDirectionX, CameraDirectionY, 0.f);
}

void ACameraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// "Стратегический" курсор.
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	SetInputMode(InputMode);
	GetViewportSize(ScreenSizeX, ScreenSizeY);

	PlayerCamera = Cast<ACameraPlayerPawn>(GetPawn());

}

void ACameraPlayerController::SpawnObstacle(TSubclassOf<AStaticMeshActor> ObstacleClass)
{
	float SpawnScaleX = FMath::FRandRange(1.f, 10.f);
	float SpawnScaleY = FMath::FRandRange(1.f, 10.f);
	float SpawnScaleZ = 3.f;

	float SpawnLocationX = 0.f;
	float SpawnLocationY = 0.f;
	float SpawnLocationZ = SpawnScaleZ / 2 + GroundThickness; // = 1/2 SpawnedObjectZ + 20.f (1/2 FloorMeshZ?)

	GetMousePosition(SpawnLocationX, SpawnLocationY);

	FVector Location = FVector(SpawnLocationX, SpawnLocationY, SpawnLocationZ);
	FVector Scale = FVector(SpawnScaleX, SpawnScaleY, SpawnScaleZ);

	FocusedActor = GetWorld()->SpawnActor<AStaticMeshActor>(ObstacleClass);
	FocusedActor->SetActorLocation(Location);
	FocusedActor->GetStaticMeshComponent()->SetWorldScale3D(Scale);
	FocusedActor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
	
	FocusedActor = nullptr;
}

void ACameraPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Двигаем объекты.
	if (bMouseButtonIsPressed && IsValid(FocusedActor))
	{
		bool bActorIsMovable = FocusedActor->GetStaticMeshComponent()->Mobility == EComponentMobility::Movable;
		
		if (bActorIsMovable)
		{
			// Смотрим координаты под курсором.
			FHitResult Hit;
			GetHitResultUnderCursor(ECC_Visibility, false, Hit);

			if (Hit.bBlockingHit)
			{
				float X = Hit.ImpactPoint.X;
				float Y = Hit.ImpactPoint.Y;
				float Z = FocusedActor->GetActorLocation().Z;

				FVector TargetLocation = FVector(X, Y, Z);

				FocusedActor->SetActorLocation(TargetLocation);
			}

			//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Green, FString::Printf(TEXT("Mouse world something: %s"), *Hit.ImpactPoint.ToString()));
		}
	}

	// Двигаем камеру
	GetPawn()->AddActorWorldOffset(MoveScreen());
}

void ACameraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction(TEXT("ZoomIn"), EInputEvent::IE_Pressed, this, &ACameraPlayerController::ZoomIn);
	InputComponent->BindAction(TEXT("ZoomOut"), EInputEvent::IE_Pressed, this, &ACameraPlayerController::ZoomOut);

	InputComponent->BindAction(TEXT("Select"), EInputEvent::IE_Pressed, this, &ACameraPlayerController::MouseLeftClick);
	InputComponent->BindAction(TEXT("Select"), EInputEvent::IE_Released, this, &ACameraPlayerController::MouseLeftReleased);
}
