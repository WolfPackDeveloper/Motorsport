// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraPlayerController.h"
#include "Actors/RouteActor.h"
#include "MachineSpirit.h"	// On-Off MachineSpirit
#include "Pawns/CameraPlayerPawn.h"
#include "Pawns/Landraider.h" // Ручное управление для теста MovementComponent
#include "Components/LandraiderMovementComponent.h" // Ручное управление для теста MovementComponent
#include "MotorsportGameModeBase.h"

#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ConstructorHelpers.h" // Random Meshes
#include "Engine/Engine.h" // Debug strings
//#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h" // GetGameMode - Получаем ГеймМод! Вот и не нужны мне ваши раковины!

//#include "Math/BoxSphereBounds.h" // Debug spawned mesh.

ACameraPlayerController::ACameraPlayerController()
{
	FocusedActor = nullptr;
	bMouseButtonIsPressed = false;
	Margin = 10;
	FlipThroughSpeed = 20.f;
	RouteNodesAmount = 10;

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

	// Debug - координаты на карте.
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Mouse world something: %s"), *HitResult.ImpactPoint.ToString()));

	if (bIsHit)
	{
		// С таким подходом не двигает машинку и всякое такое... А надо ли? Потом, если время останется.
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

void ACameraPlayerController::GetLandraider()
{
	
	if (IsValid(GameMode))
	{
		CurrentLandraider = Cast<ALandraider>(GameMode->GetMachineActor());
	}
	if (IsValid(CurrentLandraider))
	{
		CurrentMachineSpirit = Cast<AMachineSpirit>(CurrentLandraider->GetController());
	}
}

void ACameraPlayerController::LandraiderMoveForward()
{

}

void ACameraPlayerController::LandraiderMoveBack()
{

}

void ACameraPlayerController::LandraiderBrake()
{

}

void ACameraPlayerController::LandraiderIdle()
{

}

void ACameraPlayerController::LandraiderTurnRight()
{

}

void ACameraPlayerController::LandraiderTurnLeft()
{

}

// TODO: Надо было делать делегатом.(
void ACameraPlayerController::SetCurrentMachineSpiritState()
{
	if (!IsValid(CurrentLandraider) || !IsValid(CurrentMachineSpirit))
	{
		CurrentLandraider = Cast<ALandraider>(GameMode->GetMachineActor());
		CurrentMachineSpirit = Cast<AMachineSpirit>(CurrentLandraider->GetController());
	}
	else
	{
		CurrentSpiritState = CurrentMachineSpirit->GetMachineSpiritState();

		switch (CurrentSpiritState)
		{
		case EMachineSpiritState::Awake:
			CurrentSpiritState = EMachineSpiritState::Sleeps;
			break;

		case EMachineSpiritState::Sleeps:
			CurrentSpiritState = EMachineSpiritState::Awake;
			break;

		default:
			break;
		}

		CurrentMachineSpirit->SetMachineSpiritState(CurrentSpiritState);

	}
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
	GameMode = Cast<AMotorsportGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	// Возня для тестов движения и включения-выключения Духа Машины.
	CurrentLandraider = Cast<ALandraider>(GameMode->GetMachineActor());
	if (IsValid(CurrentLandraider))
	{
		CurrentMachineSpirit = Cast<AMachineSpirit>(CurrentLandraider->GetController());
	}
}

void ACameraPlayerController::SpawnObstacle()
{
	if (!IsValid(GameMode)) return;
	
	// Выбираем рандомный меш
	TArray<UStaticMesh*> MeshArray = GameMode->GetObstacleMeshes();

	if (MeshArray.Num() < 1) return;
	
	int32 MeshIndex = FMath::RandRange(0, MeshArray.Num() - 1);

	// Debug
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Ramdom Mesh: %i"), MeshIndex));

	if (MeshArray[MeshIndex] == nullptr) return;

	// Debug
	//if (MeshArray[MeshIndex] != nullptr)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Mesh Exists?!")));
	//}
	
	// Выбираем рандомный размер меша
	float SpawnScaleX = FMath::FRandRange(1.f, 10.f);
	float SpawnScaleY = FMath::FRandRange(1.f, 10.f);
	float SpawnScaleZ = 3.f;

	// Спавним под курсором препятствие
	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	if (Hit.bBlockingHit)
	{
		float X = Hit.ImpactPoint.X;
		float Y = Hit.ImpactPoint.Y;
		float Z = SpawnScaleZ * 50 + GroundThickness; // = 1*100/2 SpawnedObjectZ + 20.f (1/2 FloorMeshZ?)

		FVector Location = FVector(X, Y, Z);
		FRotator Rotation = FRotator(0.f, 0.f, 0.f);
		FVector Scale = FVector(SpawnScaleX, SpawnScaleY, SpawnScaleZ);

		FocusedActor = GetWorld()->SpawnActor<AStaticMeshActor>(ObstacleClass, Location, Rotation);

		FVector ActorLocation = FocusedActor->GetActorLocation();
		UStaticMesh* Mesh = MeshArray[MeshIndex];
		
		// Debug
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("Spawned Obstacle Location: %s"), *ActorLocation.ToString()));

		// Настраиваем меш
		FocusedActor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
		FocusedActor->GetStaticMeshComponent()->SetStaticMesh(MeshArray[MeshIndex]);
		FocusedActor->GetStaticMeshComponent()->SetWorldScale3D(Scale);
	}
	FocusedActor = nullptr;
}

void ACameraPlayerController::BuildRoute()
{
	//AActor* Ground = GameMode->GetGround();
	ARouteActor* Route = GameMode->GetRouteActor();

	Route->BuildRoute(EDirection::Up, EDirection::Right, ERouteType::Snake);
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
	InputComponent->BindAction(TEXT("CreateObstacle"), EInputEvent::IE_Pressed, this, &ACameraPlayerController::SpawnObstacle);

	InputComponent->BindAction(TEXT("BuildRoute"), EInputEvent::IE_Pressed, this, &ACameraPlayerController::BuildRoute);

	InputComponent->BindAction(TEXT("AwakenMachineSpirit"), EInputEvent::IE_Pressed, this, &ACameraPlayerController::SetCurrentMachineSpiritState);
	
	// Test MovementComponent
	InputComponent->BindAction(TEXT("MoveLandraiderForward"), EInputEvent::IE_Pressed, this, &ACameraPlayerController::LandraiderMoveForward);
	InputComponent->BindAction(TEXT("MoveLandraiderBack"), EInputEvent::IE_Pressed, this, &ACameraPlayerController::LandraiderMoveBack);
	InputComponent->BindAction(TEXT("BrakeLandraider"), EInputEvent::IE_Pressed, this, &ACameraPlayerController::LandraiderBrake);
	InputComponent->BindAction(TEXT("TurnLandraiderRight"), EInputEvent::IE_Pressed, this, &ACameraPlayerController::LandraiderTurnRight);
	InputComponent->BindAction(TEXT("TurnLandraiderLeft"), EInputEvent::IE_Pressed, this, &ACameraPlayerController::LandraiderTurnLeft);

	InputComponent->BindAction(TEXT("MoveLandraiderForward"), EInputEvent::IE_Released, this, &ACameraPlayerController::LandraiderIdle);
	InputComponent->BindAction(TEXT("MoveLandraiderBack"), EInputEvent::IE_Released, this, &ACameraPlayerController::LandraiderIdle);
	InputComponent->BindAction(TEXT("BrakeLandraider"), EInputEvent::IE_Released, this, &ACameraPlayerController::LandraiderIdle);


}
