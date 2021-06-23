// Fill out your copyright notice in the Description page of Project Settings.


#include "MachineSpirit.h"
#include "MotorsportGameModeBase.h"
#include "Actors/RouteActor.h"
#include "Components/LandraiderMovementComponent.h"
#include "Pawns/Landraider.h"

#include "Components/SplineComponent.h" // Spline points navigation.
#include "Engine/Engine.h" // Debug strings
#include "Engine/World.h" // GetWorld
#include "Kismet/GameplayStatics.h" // GetGameMode - Получаем ГеймМод! Вот и не нужны мне ваши раковины!
#include "Kismet/KismetMathLibrary.h" // LookAtRotation
#include "Kismet/KismetSystemLibrary.h" // Trace
#include "Math/Box.h"


AMachineSpirit::AMachineSpirit()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
	
	SpiritState = EMachineSpiritState::Sleeps;
	GroundBounds = FVector(0.f, 0.f, 0.f);

	// Карты "опасности" препятствий: Лв(0), ЛП(1), П(2), ПП(3), Пр(4).
	for (int i = 0; i < 5; ++i)
	{
		// Мутновато...
		ThreatMap.Add(FThreatInstigator());
	}

	// Карта "весов" препятствий (Лв, ЛП, П, ПП, Пр), границ и маршрута.
	WeightMap.Add(TEXT("Left"), 0.f);
	WeightMap.Add(TEXT("LeftForward"), 0.f);
	WeightMap.Add(TEXT("Forward"), 0.f);
	WeightMap.Add(TEXT("RightForward"), 0.f);
	WeightMap.Add(TEXT("Right"), 0.f);
	WeightMap.Add(TEXT("Route"), 0.f);
	WeightMap.Add(TEXT("Bounds"), 0.f);

	// Карта "весов" направлений: Лв(0), П(1), Пр(2).
	DirectionWeightMap.Add(FDirectionWeight{ ESteerDirection::Left, 0.f });
	DirectionWeightMap.Add(FDirectionWeight{ ESteerDirection::Forward, 0.f });
	DirectionWeightMap.Add(FDirectionWeight{ ESteerDirection::Right, 0.f });
}

FVector AMachineSpirit::GetOwnedPawnDimensions()
{
	if (IsValid(OwnedLandraider))
	{
		FVector Origin = FVector();
		FVector Extent = FVector();

		OwnedLandraider->GetActorBounds(true, Origin, Extent);

		return Extent * 2.f;
	}

	return FVector();
}

void AMachineSpirit::DefineGroundBounds()
{
	if (!GameMode) return;
	GroundBounds = GameMode->GetGroundBounds();
}

void AMachineSpirit::Throttle(float InputIntensity)
{
	if (IsValid(OwnedLandraider))
	{
		OwnedLandraider->GetLandraiderMovementComponent()->SetDriveAction(EDriveAction::FullThrottle, InputIntensity);
	}
}

void AMachineSpirit::ReverseGear(float InputIntensity)
{
	if (IsValid(OwnedLandraider))
	{
		OwnedLandraider->GetLandraiderMovementComponent()->SetDriveAction(EDriveAction::ReverseGear, InputIntensity);
	}
}

void AMachineSpirit::Brake(float InputIntensity)
{
	if (IsValid(OwnedLandraider))
	{
		OwnedLandraider->GetLandraiderMovementComponent()->SetDriveAction(EDriveAction::Braking, InputIntensity);
	}
}

void AMachineSpirit::IdleMoving(float InputIntensity)
{
	if (IsValid(OwnedLandraider))
	{
		OwnedLandraider->GetLandraiderMovementComponent()->SetDriveAction(EDriveAction::IdleMoving, InputIntensity);
	}
}

void AMachineSpirit::TurnRight(float InputIntensity)
{
	if (IsValid(OwnedLandraider))
	{
		OwnedLandraider->GetLandraiderMovementComponent()->Steering(InputIntensity);
	}
}

void AMachineSpirit::BeginPlay()
{
	Super::BeginPlay();

	SpiritState = EMachineSpiritState::Sleeps;

	GameMode = Cast<AMotorsportGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (IsValid(GameMode))
	{
		CurrentRoute = GameMode->GetRouteActor();
		GroundBounds = GameMode->GetGroundBounds();
	}
}

void AMachineSpirit::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	OwnedLandraider = Cast<ALandraider>(GetPawn());

	if (IsValid(OwnedLandraider))
	{
		PawnDimensions = GetOwnedPawnDimensions();
	}
}

void AMachineSpirit::OnUnPossess()
{
	Super::OnUnPossess();
}

void AMachineSpirit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (SpiritState)
	{
	case EMachineSpiritState::Awake:
		DefineSteerBehaviour();
		break;
	
	case EMachineSpiritState::Sleeps:
		
		break;
	
	default:
		break;
	}
}

void AMachineSpirit::TraceLeft()
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwnedLandraider);
	float Distance = 0; // Расстояние до точки столкновения луча с препятствием. Для расчёта уровня угрозы.

	float DeltaX = PawnDimensions.X / 2;
	float DeltaY = PawnDimensions.Y / 2;
	FVector ForwardVector = OwnedLandraider->GetActorForwardVector();
	FVector RightVector = OwnedLandraider->GetActorRightVector();
	FVector TraceStart = OwnedLandraider->GetActorLocation() + (ForwardVector * DeltaX) - (RightVector * DeltaY);
	FVector TraceEnd = TraceStart + (ForwardVector * TraceLength) - (RightVector * DeltaY);

	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("MachineSpirit: Trace left start vector: %s"), *TraceStart.ToString()));

	FHitResult HitResult;

	bool Hit = UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		TraceStart,
		TraceEnd,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		true,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResult,
		true,
		FLinearColor::Green,
		FLinearColor::Red,
		0.1f
	);

	if (Hit)
	{
		Distance = sqrtf(powf(HitResult.Location.X - TraceStart.X, 2.f) + powf(HitResult.Location.Y - TraceStart.Y, 2.f));
		
		ThreatMap[0].Obstacle = HitResult.GetActor();
		ThreatMap[0].ThreatValue = 1 - (Distance / TraceLength);
	}
	else
	{
		ThreatMap[0].Obstacle = nullptr;
		ThreatMap[0].ThreatValue = 0.f;
	}

	//Debug
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("MachineSpirit: Left Threat: %f"), ThreatMap[0].ThreatValue));
}

void AMachineSpirit::TraceForwardLeft()
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwnedLandraider);
	float Distance = 0; // Расстояние до точки столкновения луча с препятствием. Для расчёта уровня угрозы.

	float DeltaX = PawnDimensions.X / 2;
	float DeltaY = PawnDimensions.Y / 2;
	FVector ForwardVector = OwnedLandraider->GetActorForwardVector();
	FVector RightVector = OwnedLandraider->GetActorRightVector();
	FVector TraceStart = OwnedLandraider->GetActorLocation() + (ForwardVector * DeltaX) - (RightVector * DeltaY);
	FVector TraceEnd = TraceStart + (ForwardVector * TraceLength) + (RightVector * 1.f);

	FHitResult HitResult;

	bool Hit = UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		TraceStart,
		TraceEnd,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		true,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResult,
		true,
		FLinearColor::Green,
		FLinearColor::Red,
		0.1f
	);

	if (Hit)
	{
		Distance = sqrtf(powf(HitResult.Location.X - TraceStart.X, 2.f) + powf(HitResult.Location.Y - TraceStart.Y, 2.f));
		ThreatMap[1].Obstacle = HitResult.GetActor();
		ThreatMap[1].ThreatValue = 1 - (Distance / TraceLength);
	}
	else
	{
		ThreatMap[1].Obstacle = nullptr;
		ThreatMap[1].ThreatValue = 0.f;
	}
	//Debug
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("MachineSpirit: LeftForward Threat: %f"), ThreatMap[1].ThreatValue));
}

void AMachineSpirit::TraceForward()
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwnedLandraider);
	float Distance = 0; // Расстояние до точки столкновения луча с препятствием. Для расчёта уровня угрозы.

	float DeltaX = PawnDimensions.X / 2;
	float DeltaY = 1.f;
	FVector ForwardVector = OwnedLandraider->GetActorForwardVector();
	FVector RightVector = OwnedLandraider->GetActorRightVector();
	FVector TraceStart = OwnedLandraider->GetActorLocation() + (ForwardVector * DeltaX) + (RightVector * DeltaY);
	FVector TraceEnd = TraceStart + (ForwardVector * TraceLength) + (RightVector * DeltaY);

	FHitResult HitResult;

	bool Hit = UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		TraceStart,
		TraceEnd,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		true,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResult,
		true,
		FLinearColor::Green,
		FLinearColor::Red,
		0.1f
	);

	if (Hit)
	{
		Distance = sqrtf(powf(HitResult.Location.X - TraceStart.X, 2.f) + powf(HitResult.Location.Y - TraceStart.Y, 2.f));
		ThreatMap[2].Obstacle = HitResult.GetActor();
		ThreatMap[2].ThreatValue = 1 - (Distance / TraceLength);
	}
	else
	{
		ThreatMap[2].Obstacle = nullptr;
		ThreatMap[2].ThreatValue = 0.f;
	}
	
	//Debug
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("MachineSpirit: Forward Threat: %f"), ThreatMap[2].ThreatValue));
}

void AMachineSpirit::TraceForwardRight()
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwnedLandraider);
	float Distance = 0; // Расстояние до точки столкновения луча с препятствием. Для расчёта уровня угрозы.
	
	float DeltaX = PawnDimensions.X / 2;
	float DeltaY = PawnDimensions.Y / 2;
	FVector ForwardVector = OwnedLandraider->GetActorForwardVector();
	FVector RightVector = OwnedLandraider->GetActorRightVector();
	FVector TraceStart = OwnedLandraider->GetActorLocation() + (ForwardVector * DeltaX) + (RightVector * DeltaY);
	FVector TraceEnd = TraceStart + (ForwardVector * TraceLength) - (RightVector * 1.f);

	FHitResult HitResult;

	bool Hit = UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		TraceStart,
		TraceEnd,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		true,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResult,
		true,
		FLinearColor::Green,
		FLinearColor::Red,
		0.1f
	);

	if (Hit)
	{
		Distance = sqrtf(powf(HitResult.Location.X - TraceStart.X, 2.f) + powf(HitResult.Location.Y - TraceStart.Y, 2.f));
		ThreatMap[3].Obstacle = HitResult.GetActor();
		ThreatMap[3].ThreatValue = 1 - (Distance / TraceLength);
	}
	else
	{
		ThreatMap[3].Obstacle = nullptr;
		ThreatMap[3].ThreatValue = 0.f;
	}

	//Debug
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("MachineSpirit: RightForward Threat: %f"), ThreatMap[3].ThreatValue));
}

void AMachineSpirit::TraceRight()
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwnedLandraider);
	float Distance = 0; // Расстояние до точки столкновения луча с препятствием. Для расчёта уровня угрозы.
	
	float DeltaX = PawnDimensions.X / 2;
	float DeltaY = PawnDimensions.Y / 2;
	FVector ForwardVector = OwnedLandraider->GetActorForwardVector();
	FVector RightVector = OwnedLandraider->GetActorRightVector();
	FVector TraceStart = OwnedLandraider->GetActorLocation() + (ForwardVector * DeltaX) + (RightVector * DeltaY);
	FVector TraceEnd = TraceStart + (ForwardVector * TraceLength) + (RightVector * DeltaY);

	FHitResult HitResult;

	bool Hit = UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		TraceStart,
		TraceEnd,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		true,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResult,
		true,
		FLinearColor::Green,
		FLinearColor::Red,
		0.1f
	);

	if (Hit)
	{
		Distance = sqrtf(powf(HitResult.Location.X - TraceStart.X, 2.f) + powf(HitResult.Location.Y - TraceStart.Y, 2.f));
		ThreatMap[4].Obstacle = HitResult.GetActor();
		ThreatMap[4].ThreatValue = 1 - (Distance / TraceLength);
	}
	else
	{
		ThreatMap[4].Obstacle = nullptr;
		ThreatMap[4].ThreatValue = 0.f;
	}
	//Debug
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("MachineSpirit: Right Threat: %f"), ThreatMap[4].ThreatValue));
}

void AMachineSpirit::FillThreatMap()
{
	// Ну не запихивать же в каждую функцию эти две проверки...
	if (!IsValid(OwnedLandraider)) return;
	// Можно запихнуть в какую-нибудь функцию зануления или инициализации правильной размерности.
	// Или тут это оставлять, или внутри функций расчётов. Или и тут и там...
	if (ThreatMap.Num() != 5)
	{
		ThreatMap.Empty();
		// Карты "опасности" препятствий: Лв(0), ЛП(1), П(2), ПП(3), Пр(4).
		for (int i = 0; i < 5; ++i)
		{
			// Мутновато...
			ThreatMap.Add(FThreatInstigator());
		}
	}
	
	TraceLeft();
	TraceForwardLeft();
	TraceForward();
	TraceForwardRight();
	TraceRight();
}


int AMachineSpirit::DefineCurrentRoutePoint()
{
	if (!IsValid(CurrentRoute))
	{
		CurrentRoute = GameMode->GetRouteActor();
	}
	
	if (CurrentRoute == nullptr)
	{
		return CurrentRoutePointIndex = -1;;
	}

	int RoutePointsNumber = CurrentRoute->GetRouteSpline()->GetNumberOfSplinePoints();
	// Если пересоздать маршрут в рантайме, счётчик не скинется. И это косяк.
	if (RoutePointsNumber > 2)
	{
		if (CurrentRoutePointIndex < 0)
		{
			CurrentRoutePointIndex = 0;
		}
		else if (CurrentRoutePointIndex == 0)
		{
			CurrentRoutePointIndex = 1;
		}
		else if (CurrentRoutePointIndex < RoutePointsNumber)
		{
			// Опасно. Это единственное место, где определяется точка маршрута.
			FVector Location = OwnedLandraider->GetActorLocation() + (OwnedLandraider->GetActorForwardVector() * (PawnDimensions.X / 2));
			//FVector Distance = CurrentRoute->GetRouteSpline()->GetWorldLocationAtSplinePoint(CurrentRoutePointIndex) - Location;
			FVector Point = CurrentRoute->GetRouteSpline()->GetWorldLocationAtSplinePoint(CurrentRoutePointIndex);

			float DeltaX = PawnDimensions.X / 2;
			float DeltaY = PawnDimensions.Y / 2;
			FVector ForwardVector = OwnedLandraider->GetActorForwardVector();
			FVector RightVector = OwnedLandraider->GetActorRightVector();
			FVector TraceStart = OwnedLandraider->GetActorLocation() + (ForwardVector * DeltaX) + (RightVector * 1.f);
			FVector TraceForward = TraceStart + (ForwardVector * PointReachField) + (RightVector * 1.f);
			FVector TraceBack = TraceStart + (ForwardVector * (-PointReachField - DeltaX)) + (RightVector * 1.f);
			FVector TraceLeft = TraceStart + (ForwardVector * 1.f) + (RightVector * (-PointReachField - DeltaY));
			FVector TraceRight = TraceStart + (ForwardVector * 1.f) + (RightVector * (PointReachField + DeltaY));
			//FVector RouteLocation = CurrentRoute->GetRouteSpline()->GetWorldLocationAtSplinePoint(CurrentRoutePointIndex);
			//FVector DeltaLocation = RouteLocation - TraceStart;
			//FRotator ForwardRotation = UKismetMathLibrary::FindLookAtRotation(TraceStart, TraceEnd);
			//FRotator LeftRotation = UKismetMathLibrary::FindLookAtRotation(TraceStart, TraceLeft);
			//FRotator RightRotation = UKismetMathLibrary::FindLookAtRotation(TraceStart, TraceRight);
			//FRotator PointRotation = UKismetMathLibrary::FindLookAtRotation(TraceStart, RouteLocation);

			// Debug trace.
			//FHitResult HitResult;
			//TArray<AActor*> ActorsToIgnore;
			//ActorsToIgnore.Add(OwnedLandraider);

			//UKismetSystemLibrary::LineTraceSingle(
			//	GetWorld(),
			//	TraceStart,
			//	TraceForward,
			//	UEngineTypes::ConvertToTraceType(ECC_Visibility),
			//	true,
			//	ActorsToIgnore,
			//	EDrawDebugTrace::ForDuration,
			//	HitResult,
			//	true,
			//	FLinearColor::Yellow,
			//	FLinearColor::Red,
			//	0.1f
			//);

			//UKismetSystemLibrary::LineTraceSingle(
			//	GetWorld(),
			//	TraceStart,
			//	TraceBack,
			//	UEngineTypes::ConvertToTraceType(ECC_Visibility),
			//	true,
			//	ActorsToIgnore,
			//	EDrawDebugTrace::ForDuration,
			//	HitResult,
			//	true,
			//	FLinearColor::Yellow,
			//	FLinearColor::Red,
			//	0.1f
			//);

			//UKismetSystemLibrary::LineTraceSingle(
			//	GetWorld(),
			//	TraceStart,
			//	TraceLeft,
			//	UEngineTypes::ConvertToTraceType(ECC_Visibility),
			//	true,
			//	ActorsToIgnore,
			//	EDrawDebugTrace::ForDuration,
			//	HitResult,
			//	true,
			//	FLinearColor::Yellow,
			//	FLinearColor::Red,
			//	0.1f
			//);

			//UKismetSystemLibrary::LineTraceSingle(
			//	GetWorld(),
			//	TraceStart,
			//	TraceRight,
			//	UEngineTypes::ConvertToTraceType(ECC_Visibility),
			//	true,
			//	ActorsToIgnore,
			//	EDrawDebugTrace::ForDuration,
			//	HitResult,
			//	true,
			//	FLinearColor::Yellow,
			//	FLinearColor::Red,
			//	0.1f
			//);

			bool bReachX = Point.X < TraceForward.X && Point.X > TraceBack.X || Point.X > TraceForward.X && Point.X < TraceBack.X;
			bool bReachY = Point.Y < TraceRight.Y && Point.Y > TraceLeft.Y || Point.Y > TraceRight.Y && Point.Y < TraceLeft.Y;

			//bool bReachX = (abs(Point.X - Location.X) < PointReachField) || (abs(Point.X + Location.X) < PointReachField);
			//bool bReachY = (abs(Point.Y - Location.Y) < PointReachField) || (abs(Point.Y + Location.Y) < PointReachField);

			if (bReachX && bReachY)
			{
				CurrentRoutePointIndex++;
			}
		}
	}

	//Debug
	//float number = CurrentRoute->GetRouteSpline()->GetNumberOfSplinePoints();
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("MachineSpirit: Current route points number: %f"), number));
	float index = CurrentRoutePointIndex;
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("MachineSpirit: Current route point: %f"), index));

	return CurrentRoutePointIndex;
}

float AMachineSpirit::DefineRoutePointWeight()
{
	DefineCurrentRoutePoint();

	if (CurrentRoutePointIndex < 1 || CurrentRoutePointIndex >= CurrentRoute->GetRouteSpline()->GetNumberOfSplinePoints())
	{
		return 0.f;
	}

	float Weight = 0.f;
	float PointWeightAdd = 0.2f;
	float DeltaX = PawnDimensions.X / 2;
	float DeltaY = PawnDimensions.Y / 2;
	FVector ForwardVector = OwnedLandraider->GetActorForwardVector();
	FVector RightVector = OwnedLandraider->GetActorRightVector();
	FVector TraceStart = OwnedLandraider->GetActorLocation() + (ForwardVector * DeltaX) + (RightVector * 1.f);
	FVector TraceForward = TraceStart + (ForwardVector * (TraceLength + DeltaX)) + (RightVector * 1.f);
	FVector TraceBack = TraceStart + (ForwardVector * (-PointReachField - DeltaX)) + (RightVector * 1.f);
	FVector TraceLeft = TraceStart + (ForwardVector * 1.f) + (RightVector * (-TraceLength - DeltaY));
	FVector TraceRight = TraceStart + (ForwardVector * 1.f) + (RightVector * (TraceLength + DeltaY));
	FVector RouteLocation = CurrentRoute->GetRouteSpline()->GetWorldLocationAtSplinePoint(CurrentRoutePointIndex);
	//FVector TracePoint = RouteLocation;
	FRotator ForwardRotation = UKismetMathLibrary::FindLookAtRotation(TraceStart, TraceForward);
	//FRotator BackRotation = UKismetMathLibrary::FindLookAtRotation(TraceStart, TraceBack);
	FRotator LeftRotation = UKismetMathLibrary::FindLookAtRotation(TraceStart, TraceLeft);
	FRotator PointRotation = UKismetMathLibrary::FindLookAtRotation(TraceStart, RouteLocation);

	FVector DeltaLocation = RouteLocation - TraceStart;

	float Distance = sqrtf(powf(DeltaLocation.X, 2.f) + powf(DeltaLocation.Y, 2.f));

	FHitResult HitResult;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwnedLandraider);

	UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		TraceStart,
		RouteLocation,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		true,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResult,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		0.1f
	);

	UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		TraceStart,
		TraceForward,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		true,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResult,
		true,
		FLinearColor::Yellow,
		FLinearColor::Red,
		0.1f
	);

	UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		TraceStart,
		TraceBack,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		true,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResult,
		true,
		FLinearColor::Yellow,
		FLinearColor::Red,
		0.1f
	);

	UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		TraceStart,
		TraceLeft,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		true,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResult,
		true,
		FLinearColor::Yellow,
		FLinearColor::Red,
		0.1f
	);

	UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		TraceStart,
		TraceRight,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		true,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResult,
		true,
		FLinearColor::Yellow,
		FLinearColor::Red,
		0.1f
	);

	// Если едем прямо на точку, то продолжаем ехать.
	//if ((ForwardRotation.Yaw - PointRotation.Yaw) == 0.f) return 0.f;
	if (ForwardRotation.Yaw == PointRotation.Yaw)
	{
		return Weight = 0.f;
	}

	// Если нет, то смотрим вес.
	if (Distance > TraceLength)
	{
		Weight = PointWeightAdd;
	}
	else
	{
		Weight = PointWeightAdd + (1.f - (Distance / TraceLength));
	}
	// И куда повернуть.

	bool bNordWest = ForwardRotation.Yaw < 0.f && LeftRotation.Yaw < 0.f;
	bool bNordEast = ForwardRotation.Yaw > 0.f && LeftRotation.Yaw < 0.f;
	bool bSouthWest = ForwardRotation.Yaw < 0.f && LeftRotation.Yaw < 0.f;
	bool bSouthEast = ForwardRotation.Yaw > 0.f && LeftRotation.Yaw < 0.f;

	if (bNordWest)
	{

	}

	bool bLeftTurn = PointRotation.Yaw < ForwardRotation.Yaw&& PointRotation.Yaw > LeftRotation.Yaw;

	if (bLeftTurn)
	{
		Weight *= -1.f;
	}

	//Debug
	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("MachineSpirit: Route left rotation: %f"), LeftRotation.Yaw));
	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("MachineSpirit: Route point rotation: %f"), PointRotation.Yaw));
	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("MachineSpirit: Route forward rotation: %f"), ForwardRotation.Yaw));
	

	return Weight;
}

float AMachineSpirit::DefineGroundBoundsWeight()
{	
	if (GroundBounds == FVector(0.f, 0.f, 0.f))
	{
		if (!IsValid(GameMode)) return 0.f;

		GroundBounds = GameMode->GetGroundBounds();
	}
	if (!IsValid(OwnedLandraider)) return 0.f;
	if (PawnDimensions == FVector(0.f, 0.f, 0.f)) return 0.f;

	float DeltaX = PawnDimensions.X / 2.f;
	float DeltaY = PawnDimensions.Y * 3.f;
	//float TurnMultiplier = 1.2f;
	FVector Location = OwnedLandraider->GetActorLocation();
	FVector ForwardVector = OwnedLandraider->GetActorForwardVector();
	FVector RightVector = OwnedLandraider->GetActorRightVector();
	FVector ForwardEnd = Location + (ForwardVector * ((TraceLength * 1.5f) + DeltaX)) + (RightVector * 1.f);
	FVector LeftEnd = Location + (ForwardVector * DeltaX) - (RightVector * DeltaY);
	FVector RightEnd = Location + (ForwardVector * DeltaX) + (RightVector * DeltaY);
	
	// Debug trace.
	//FHitResult HitResult;
	//TArray<AActor*> ActorsToIgnore;

	//UKismetSystemLibrary::LineTraceSingle(
	//	GetWorld(),
	//	Location,
	//	ForwardEnd,
	//	UEngineTypes::ConvertToTraceType(ECC_Visibility),
	//	true,
	//	ActorsToIgnore,
	//	EDrawDebugTrace::ForDuration,
	//	HitResult,
	//	true,
	//	FLinearColor::Yellow,
	//	FLinearColor::Red,
	//	0.1f
	//);

	//UKismetSystemLibrary::LineTraceSingle(
	//	GetWorld(),
	//	Location,
	//	LeftEnd,
	//	UEngineTypes::ConvertToTraceType(ECC_Visibility),
	//	true,
	//	ActorsToIgnore,
	//	EDrawDebugTrace::ForDuration,
	//	HitResult,
	//	true,
	//	FLinearColor::Yellow,
	//	FLinearColor::Red,
	//	0.1f
	//);

	//UKismetSystemLibrary::LineTraceSingle(
	//	GetWorld(),
	//	Location,
	//	RightEnd,
	//	UEngineTypes::ConvertToTraceType(ECC_Visibility),
	//	true,
	//	ActorsToIgnore,
	//	EDrawDebugTrace::ForDuration,
	//	HitResult,
	//	true,
	//	FLinearColor::Yellow,
	//	FLinearColor::Red,
	//	0.1f
	//);

	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Yellow, FString::Printf(TEXT("MachineSpirit: ForwardBoundEndX: %f"), abs(ForwardEnd.X)));
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Yellow, FString::Printf(TEXT("MachineSpirit: ForwardBoundEndY: %f"), abs(ForwardEnd.Y)));

	// Вес находится в промежутке от -1 до 1. Знак определяет направление поворота: лево-право.!
	float Weight = 0.f;

	bool bForwardXIntersection = abs(ForwardEnd.X) >= abs(GroundBounds.X);
	bool bForwardYIntersection = abs(ForwardEnd.Y) >= abs(GroundBounds.Y);
	bool bRightIntersection = abs(RightEnd.X) >= abs(GroundBounds.X) || abs(RightEnd.Y) >= abs(GroundBounds.Y);

	if (bForwardXIntersection)
	{
		Weight = (abs(ForwardEnd.X) - abs(GroundBounds.X)) / TraceLength;
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("MachineSpirit: ForwardBoundEndX: %f"), abs(ForwardEnd.X)));

		if (bRightIntersection)
		{
			Weight *= -1.f;
		}
	}
	else if (bForwardYIntersection)
	{
		Weight = (abs(ForwardEnd.Y) - abs(GroundBounds.Y)) / TraceLength;
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("MachineSpirit: ForwardBoundEndY: %f"), abs(ForwardEnd.Y)));

		if (bRightIntersection)
		{
			Weight *= -1.f;
		}
	}

	return Weight;
}

void AMachineSpirit::FillWeightMap()
{
	if (WeightMap.Num() != 7)
	{
		WeightMap.Empty();
		// Карта "весов" препятствий (Лв, ЛП, П, ПП, Пр), границ и маршрута.
		WeightMap.Add(TEXT("Left"), 0.f);
		WeightMap.Add(TEXT("LeftForward"), 0.f);
		WeightMap.Add(TEXT("Forward"), 0.f);
		WeightMap.Add(TEXT("RightForward"), 0.f);
		WeightMap.Add(TEXT("Right"), 0.f);
		WeightMap.Add(TEXT("Route"), 0.f);
		WeightMap.Add(TEXT("Bounds"), 0.f);
	}

	WeightMap["Left"] = 1 - ThreatMap[0].ThreatValue;
	WeightMap["LeftForward"] = 1 - ThreatMap[1].ThreatValue;
	WeightMap["Forward"] = 1 - ThreatMap[2].ThreatValue;
	WeightMap["RightForward"] = 1 - ThreatMap[3].ThreatValue;
	WeightMap["Right"] = 1 - ThreatMap[4].ThreatValue;
	WeightMap["Route"] = DefineRoutePointWeight();
	WeightMap["Bounds"] = DefineGroundBoundsWeight();

	//Debug
	//for (auto Direction : WeightMap)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Blue, FString::Printf(TEXT("MachineSpirit: Weight direction: %s"), *Direction.Key.ToString()));
	//	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Orange, FString::Printf(TEXT("MachineSpirit: Weight value: %f"), Direction.Value));
	//}
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Blue, FString::Printf(TEXT("MachineSpirit: Weight Bound: %f"), WeightMap["Route"]));
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Orange, FString::Printf(TEXT("MachineSpirit: Weight value: %f"), Direction.Value));
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Blue, FString::Printf(TEXT("MachineSpirit: Weight direction: %s"), *Direction.Key.ToString()));
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Orange, FString::Printf(TEXT("MachineSpirit: Weight value: %f"), Direction.Value));
}

ESteerDirection AMachineSpirit::DefineSteerDirection()
{
	// Карта "весов" направлений: Лв(0), П(1), Пр(2).
	if (DirectionWeightMap.Num() != 3)
	{
		DirectionWeightMap.Empty();
		DirectionWeightMap.Add(FDirectionWeight{ ESteerDirection::Left, 0.f });
		DirectionWeightMap.Add(FDirectionWeight{ ESteerDirection::Forward, 0.f });
		DirectionWeightMap.Add(FDirectionWeight{ ESteerDirection::Right, 0.f });
	}

	//Debug
	//for (auto Direction : DirectionWeightMap)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Yellow, FString::Printf(TEXT("MachineSpirit: Weight value: %f"), Direction.Weight));
	//}

	FDirectionWeight Direction = { ESteerDirection::Forward, 0.f };

	// ===== Left Direction =====
	DirectionWeightMap[0].Weight = WeightMap["Left"] + WeightMap["LeftForward"] + WeightMap["Forward"];
	// ===== Forward Direction =====
	// *** При сплошных нулях - получается минус экспонента - может какой побитовый сдвиг?
	DirectionWeightMap[1].Weight = WeightMap["LeftForward"] + WeightMap["Forward"] + WeightMap["RightForward"] - abs(WeightMap["Route"]) - abs(WeightMap["Bounds"]);
	// ===== Right Direction =====
	// Сюда прибавляем значение веса точки маршрута и границы (если они отрицательный - то понятно, да?)
	// *** При сплошных нулях - получается экспонента - может какой побитовый сдвиг?
	DirectionWeightMap[2].Weight = WeightMap["Forward"] + WeightMap["RightForward"] + WeightMap["Right"] + WeightMap["Route"] + WeightMap["Bounds"];

	// Так, теперь меняем логику слегка - а то в любой непонятной ситуации ехать налево - такое себе.
	// Сначала глядим что там спереди.
	//if (Direction.Weight < DirectionWeightMap[1].Weight)
	//{
	//	Direction.Weight = DirectionWeightMap[1].Weight;
	//	Direction.Direction = DirectionWeightMap[1].Direction;
	//}
	//// Потом справа.
	//if (Direction.Weight < DirectionWeightMap[2].Weight)
	//{
	//	Direction.Weight = DirectionWeightMap[2].Weight;
	//	Direction.Direction = DirectionWeightMap[2].Direction;
	//}
	//// И в конце уже слева.
	//if (Direction.Weight < DirectionWeightMap[0].Weight)
	//{
	//	Direction.Weight = DirectionWeightMap[0].Weight;
	//	Direction.Direction = DirectionWeightMap[0].Direction;
	//}

	// Forward
	if (Direction.Weight < DirectionWeightMap[1].Weight && DirectionWeightMap[1].Weight >= 3.f)
	{
		Direction.Weight = DirectionWeightMap[1].Weight;
		Direction.Direction = DirectionWeightMap[1].Direction;
	}
	// Right turn.
	if (Direction.Weight < DirectionWeightMap[2].Weight)
	{
		Direction.Weight = DirectionWeightMap[2].Weight;
		Direction.Direction = DirectionWeightMap[2].Direction;
	}
	// Lreft turn.
	if (Direction.Weight < DirectionWeightMap[0].Weight)
	{
		Direction.Weight = DirectionWeightMap[0].Weight;
		Direction.Direction = DirectionWeightMap[0].Direction;
	}

	return Direction.Direction;
}

float AMachineSpirit::DefineThrottle(ESteerDirection CurrentDirection)
{
	// Определяем скорость движения.
	// Определение скорости движенния. Ну тут нужно пихать какую-нибудь сложную математику...
	// Но для примитивного примера просто возьмём уровень "весо-угрозы" в качестве критерия.
	// Если угроза от препятствий, или вес точки маршрути или гранцы превышают определённый показатель - меняем скоростной режим.
	// Можно взять просто обратную зависимость, но хз, насколько это грамотное решение.
	// А можно взять обратную зависимость с шагом изменения скорости. Вот это поинтереснее.
	// Допустим, шаг будет 0.2. Условные "пять передач".
	float SpeedIntencity = 0.f;
	float ThreatValue = 0.f;
	float PointBoundWeight = WeightMap["Route"] + WeightMap["Bounds"];
	// Значение по которому определяется скорость.
	float SpeedLimiter = 0.f;
	// 4 - условное число, подходящее к 5-шаговому замедлению. 
	float ReverseGearInstigationValue = 4 / SpeedSteps;

	switch (CurrentDirection)
	{

	case ESteerDirection::Left:
		ThreatValue = ThreatMap[0].ThreatValue + ThreatMap[1].ThreatValue + ThreatMap[2].ThreatValue;
		break;

	case ESteerDirection::Forward:
		ThreatValue = ThreatMap[1].ThreatValue + ThreatMap[2].ThreatValue + ThreatMap[3].ThreatValue;
		break;

	case ESteerDirection::Right:
		ThreatValue = ThreatMap[2].ThreatValue + ThreatMap[3].ThreatValue + ThreatMap[4].ThreatValue;
		break;

	default:
		break;
	}

	// Почему эта лажа не работает??????
	// Назначается SpeedLimiter только, когда ThreatValue больше.
	// И в результате ускорение получается нулевое.
	if (ThreatValue > PointBoundWeight)
	{
		SpeedLimiter = ThreatValue;
	}
	else if (ThreatValue <= PointBoundWeight)
	{
		SpeedLimiter = PointBoundWeight;
	}
	// А теперь, собственно, скорость.
	for (int i = 1; i < SpeedSteps; i++)
	{
		// Что за херня, почему if правильно не разрешается???
		// Почему при инициализации SpeedStepAmount = 2, а потом тут же 0?????
		float SpeedStepAmount = float(i) / float(SpeedSteps);
		if (SpeedLimiter <= SpeedStepAmount)
		{
			SpeedIntencity = 1.f - float(((i - 1)/ SpeedSteps));
			break;
		}
	}
	// Если совсем тоска - то, врубай заднюю... И не раз. Но потом.
	if (SpeedLimiter >= ReverseGearInstigationValue)
	{
		// Врубаем заднюю и поворачиваем в обратную сторону. Но это уже совсем другая история.
	}

	//Debug
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Orange, FString::Printf(TEXT("MachineSpirit: Throttle - speed intencity: %f"), SpeedIntencity));

	return SpeedIntencity;
}

void AMachineSpirit::DefineSteerBehaviour()
{
	if (!IsValid(OwnedLandraider)) return;
	// Определяем направление поворота.
	FillThreatMap();
	FillWeightMap();
	ESteerDirection CurrentDirection = DefineSteerDirection();
	float FullThrottle = DefineThrottle(CurrentDirection);
	
	// Жмём на педальку.
	if (FullThrottle > 0.f)
	{
		float CurrentSpeed = OwnedLandraider->GetLandraiderMovementComponent()->GetCurrentSpeed();
		float MaxSpeed = OwnedLandraider->GetLandraiderMovementComponent()->GetMaxSpeed();
		// Если скорость больше, чем надо - сбавляем.
		if ((CurrentSpeed / MaxSpeed) - FullThrottle > 0.5f)
		{
			OwnedLandraider->GetLandraiderMovementComponent()->SetDriveAction(EDriveAction::Braking, FullThrottle);
		}
		else if ((CurrentSpeed / MaxSpeed) > FullThrottle)
		{
			OwnedLandraider->GetLandraiderMovementComponent()->SetDriveAction(EDriveAction::IdleMoving, FullThrottle);
		}
		else
		{
			OwnedLandraider->GetLandraiderMovementComponent()->SetDriveAction(EDriveAction::FullThrottle, FullThrottle);
		}
	}
	// Крутим баранку.
	// TODO: Определить условия поворота руля на полшишечки.
	switch (CurrentDirection)
	{
	case ESteerDirection::Left:
		OwnedLandraider->GetLandraiderMovementComponent()->Steering(-1.f);
		break;
	
	case ESteerDirection::Forward:
		OwnedLandraider->GetLandraiderMovementComponent()->Steering(0.f);
		break;
	
	case ESteerDirection::Right:
		OwnedLandraider->GetLandraiderMovementComponent()->Steering(1.f);
		break;
	
	default:
		break;
	}
	// Может, хоть так ты поедешь???!
	//OwnedLandraider->GetLandraiderMovementComponent()->MoveForward();
	//OwnedLandraider->GetLandraiderMovementComponent()->Turn();
}

EMachineSpiritState AMachineSpirit::GetMachineSpiritState()
{
	return SpiritState;
}

void AMachineSpirit::SetMachineSpiritState(EMachineSpiritState State)
{
	SpiritState = State;
}