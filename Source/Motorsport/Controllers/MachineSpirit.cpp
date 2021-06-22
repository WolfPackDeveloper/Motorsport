// Fill out your copyright notice in the Description page of Project Settings.


#include "MachineSpirit.h"
#include "MotorsportGameModeBase.h"
#include "Actors/RouteActor.h"
#include "Components/LandraiderMovementComponent.h"
#include "Pawns/Landraider.h"

#include "Components/SplineComponent.h" // Spline points navigation.
#include "Engine/Engine.h" // Debug strings
#include "Engine/World.h" // GetWorld
#include "Kismet/GameplayStatics.h" // GetGameMode - �������� �������! ��� � �� ����� ��� ���� ��������!
#include "Kismet/KismetMathLibrary.h" // LookAtRotation
#include "Kismet/KismetSystemLibrary.h" // Trace
#include "Math/Box.h"


AMachineSpirit::AMachineSpirit()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
	
	SpiritState = EMachineSpiritState::Sleeps;
	GroundBounds = FVector(0.f, 0.f, 0.f);

	// ����� "���������" �����������: ��(0), ��(1), �(2), ��(3), ��(4).
	for (int i = 0; i < 5; ++i)
	{
		// ���������...
		ThreatMap.Add(FThreatInstigator());
	}

	// ����� "�����" ����������� (��, ��, �, ��, ��), ������ � ��������.
	WeightMap.Add(TEXT("Left"), 0.f);
	WeightMap.Add(TEXT("LeftForward"), 0.f);
	WeightMap.Add(TEXT("Forward"), 0.f);
	WeightMap.Add(TEXT("RightForward"), 0.f);
	WeightMap.Add(TEXT("Right"), 0.f);
	WeightMap.Add(TEXT("Route"), 0.f);
	WeightMap.Add(TEXT("Bounds"), 0.f);

	// ����� "�����" �����������: ��(0), �(1), ��(2).
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
	float Distance = 0; // ���������� �� ����� ������������ ���� � ������������. ��� ������� ������ ������.

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
	float Distance = 0; // ���������� �� ����� ������������ ���� � ������������. ��� ������� ������ ������.

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
	float Distance = 0; // ���������� �� ����� ������������ ���� � ������������. ��� ������� ������ ������.

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
	float Distance = 0; // ���������� �� ����� ������������ ���� � ������������. ��� ������� ������ ������.
	
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
	float Distance = 0; // ���������� �� ����� ������������ ���� � ������������. ��� ������� ������ ������.
	
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
	// �� �� ���������� �� � ������ ������� ��� ��� ��������...
	if (!IsValid(OwnedLandraider)) return;
	// ����� ��������� � �����-������ ������� ��������� ��� ������������� ���������� �����������.
	// ��� ��� ��� ���������, ��� ������ ������� ��������. ��� � ��� � ���...
	if (ThreatMap.Num() != 5)
	{
		ThreatMap.Empty();
		// ����� "���������" �����������: ��(0), ��(1), �(2), ��(3), ��(4).
		for (int i = 0; i < 5; ++i)
		{
			// ���������...
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
	// ���� ����������� ������� � ��������, ������� �� ��������. � ��� �����.
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
			// ������. ��� ������������ �����, ��� ������������ ����� ��������.
			FVector Location = OwnedLandraider->GetActorLocation() + (OwnedLandraider->GetActorForwardVector() * (PawnDimensions.X / 2));
			//FVector Distance = CurrentRoute->GetRouteSpline()->GetWorldLocationAtSplinePoint(CurrentRoutePointIndex) - Location;
			FVector Point = CurrentRoute->GetRouteSpline()->GetWorldLocationAtSplinePoint(CurrentRoutePointIndex);

			//float DeltaX = PawnDimensions.X / 2;
			//float DeltaY = PawnDimensions.Y / 2;
			//FVector ForwardVector = OwnedLandraider->GetActorForwardVector();
			//FVector RightVector = OwnedLandraider->GetActorRightVector();
			//FVector TraceStart = OwnedLandraider->GetActorLocation() + (ForwardVector * DeltaX) + (RightVector * 1.f);
			//FVector TraceEnd = TraceStart + (ForwardVector * TraceLength) + (RightVector * 1.f);
			//FVector TraceLeft = TraceStart + (ForwardVector * 1.f) + (RightVector * (-DeltaY));
			//FVector TraceRight = TraceStart + (ForwardVector * 1.f) + (RightVector * (DeltaY));
			//FVector RouteLocation = CurrentRoute->GetRouteSpline()->GetWorldLocationAtSplinePoint(CurrentRoutePointIndex);
			//FVector DeltaLocation = RouteLocation - TraceStart;
			//FRotator ForwardRotation = UKismetMathLibrary::FindLookAtRotation(TraceStart, TraceEnd);
			//FRotator LeftRotation = UKismetMathLibrary::FindLookAtRotation(TraceStart, TraceLeft);
			//FRotator RightRotation = UKismetMathLibrary::FindLookAtRotation(TraceStart, TraceRight);
			//FRotator PointRotation = UKismetMathLibrary::FindLookAtRotation(TraceStart, RouteLocation);

			bool bReachX = (abs(Point.X - Location.X) < PointReachField) || (abs(Point.X + Location.X) < PointReachField);
			bool bReachY = (abs(Point.Y - Location.Y) < PointReachField) || (abs(Point.Y + Location.Y) < PointReachField);
			//bool bReachYaw = false;

			//if (ForwardRotation.Yaw >= -90.f && ForwardRotation.Yaw <= 90.f)
			//{
			//	if (PointRotation.Yaw < LeftRotation.Yaw || PointRotation.Yaw > RightRotation.Yaw)
			//	{
			//		bReachYaw = true;
			//	}
			//}
			//else if (ForwardRotation.Yaw < -90.f && ForwardRotation.Yaw >= -180.f || ForwardRotation.Yaw > 90.f && ForwardRotation.Yaw <= 180.f)
			//{
			//	if (PointRotation.Yaw > LeftRotation.Yaw || PointRotation.Yaw < RightRotation.Yaw)
			//	{
			//		bReachYaw = true;
			//	}
			//}

			//if (bReachX && bReachY || bReachYaw)
			//{
			//	CurrentRoutePointIndex++;
			//}

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
	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("MachineSpirit: Current route point: %f"), index));

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

	// ���������� ���������� �� �����: ���� ������ ������� ����������� �� �������� -0.1-0-0.1, � ����������� �� �����������.
	// ����� ���������� �����������
	float DeltaX = PawnDimensions.X / 2;
	float DeltaY = PawnDimensions.Y / 2;
	FVector ForwardVector = OwnedLandraider->GetActorForwardVector();
	FVector RightVector = OwnedLandraider->GetActorRightVector();
	FVector TraceStart = OwnedLandraider->GetActorLocation() + (ForwardVector * DeltaX) + (RightVector * 1.f);
	FVector TraceEnd = TraceStart + (ForwardVector * TraceLength) + (RightVector * 1.f);
	FVector TraceLeft = TraceStart + (ForwardVector * 1.f) + (RightVector * (-DeltaY));
	FVector RouteLocation = CurrentRoute->GetRouteSpline()->GetWorldLocationAtSplinePoint(CurrentRoutePointIndex);
	FVector DeltaLocation = RouteLocation - TraceStart;
	FRotator ForwardRotation = UKismetMathLibrary::FindLookAtRotation(TraceStart, TraceEnd);
	FRotator LeftRotation = UKismetMathLibrary::FindLookAtRotation(TraceStart, TraceLeft);
	FRotator PointRotation = UKismetMathLibrary::FindLookAtRotation(TraceStart, RouteLocation);

	float Distance = sqrtf(powf(DeltaLocation.X, 2.f) + powf(DeltaLocation.Y, 2.f));

	// ���� ���� ����� �� �����, �� ���������� �����.
	//if ((ForwardRotation.Yaw - PointRotation.Yaw) == 0.f) return 0.f;
	if (ForwardRotation.Yaw == PointRotation.Yaw)
	{
		return Weight = 0.f;
	}

	// ���� ���, �� ������� ���� ���������.
	if (Distance > TraceLength)
	{
		Weight = 0.2f;
	}
	else
	{
		Weight = 1.2f - (Distance / TraceLength);
	}

	if (ForwardRotation.Yaw >= -90.f && ForwardRotation.Yaw <= 90.f)
	{
		if (PointRotation.Yaw < ForwardRotation.Yaw && PointRotation.Yaw > LeftRotation.Yaw)
		{
			return Weight *= -1.f;
		}
	}
	else if (ForwardRotation.Yaw < -90.f && ForwardRotation.Yaw >= -180.f || ForwardRotation.Yaw > 90.f && ForwardRotation.Yaw <= 180.f)
	{
		if (PointRotation.Yaw > ForwardRotation.Yaw && PointRotation.Yaw < LeftRotation.Yaw)
		{
			return Weight *= -1.f;
		}
	}

	//Debug
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Yellow, FString::Printf(TEXT("MachineSpirit: Route point weight: %f"), Weight));
	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("MachineSpirit: Route point rotation: %f"), PointRotation.Yaw));
	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("MachineSpirit: Route forward rotation: %f"), ForwardRotation.Yaw));
	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("MachineSpirit: Route left rotation: %f"), LeftRotation.Yaw));

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

	float DeltaX = TraceLength * 0.8f;
	float DeltaY = PawnDimensions.Y;
	FVector Location = OwnedLandraider->GetActorLocation();
	FVector ForwardVector = OwnedLandraider->GetActorForwardVector();
	FVector RightVector = OwnedLandraider->GetActorRightVector();
	FVector ForwardEnd = Location + (ForwardVector * (DeltaX + (PawnDimensions.X / 2))) + (RightVector * 1.f);
	FVector LeftEnd = Location + (ForwardVector * DeltaX) - (RightVector * DeltaY * 2);
	FVector RightEnd = Location + (ForwardVector * DeltaX) + (RightVector * DeltaY * 2);
	
	// ��� ��������� � ���������� �� -1 �� 1. ���� ���������� ����������� ��������: ����-�����.!
	float Weight = 0.f;

	// ���� ������� ������� - ������������ �������.
	if (ForwardEnd.X >= GroundBounds.X || ForwardEnd.X <= -GroundBounds.X)
	{
		// ���� ������ ���� ������� - ������������ ������.
		Weight = (abs(ForwardEnd.X) - abs(GroundBounds.X)) / TraceLength;
		
		//if ((LeftEnd.X >= GroundBounds.X || LeftEnd.X <= -GroundBounds.X || LeftEnd.Y >= GroundBounds.Y || LeftEnd.Y <= -GroundBounds.Y))
		//{
		//	Weight *= 1.f;
		//}
		//else if (RightEnd.X >= GroundBounds.X || RightEnd.X <= -GroundBounds.X || RightEnd.Y >= GroundBounds.Y || RightEnd.Y <= -GroundBounds.Y)
		//{
		//	Weight *= -1.f;
		//}
	}
	else if (ForwardEnd.Y >= GroundBounds.Y || ForwardEnd.Y <= -GroundBounds.Y)
	{
		Weight = (abs(ForwardEnd.Y) - abs(GroundBounds.Y)) / TraceLength;

		//if ((LeftEnd.X >= GroundBounds.X || LeftEnd.X <= -GroundBounds.X || LeftEnd.Y >= GroundBounds.Y || LeftEnd.Y <= -GroundBounds.Y))
		//{
		//	Weight *= 1.f;
		//}
		//else if (RightEnd.X >= GroundBounds.X || RightEnd.X <= -GroundBounds.X || RightEnd.Y >= GroundBounds.Y || RightEnd.Y <= -GroundBounds.Y)
		//{
		//	Weight *= -1.f;
		//}
	}

	if ((LeftEnd.X >= GroundBounds.X || LeftEnd.X <= -GroundBounds.X || LeftEnd.Y >= GroundBounds.Y || LeftEnd.Y <= -GroundBounds.Y))
	{
		Weight *= 1.2f;
	}
	else if (RightEnd.X >= GroundBounds.X || RightEnd.X <= -GroundBounds.X || RightEnd.Y >= GroundBounds.Y || RightEnd.Y <= -GroundBounds.Y)
	{
		Weight *= -1.2f;
	}

	//Debug
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Yellow, FString::Printf(TEXT("MachineSpirit: Ground bounds weight: %f"), Weight));

	// ������� ���� ��� �������� �� �������. � �� �����-�� ��������.
	return Weight * 1.5f;
}

void AMachineSpirit::FillWeightMap()
{
	if (WeightMap.Num() != 7)
	{
		WeightMap.Empty();
		// ����� "�����" ����������� (��, ��, �, ��, ��), ������ � ��������.
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
	for (auto Direction : WeightMap)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Blue, FString::Printf(TEXT("MachineSpirit: Weight direction: %s"), *Direction.Key.ToString()));
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Orange, FString::Printf(TEXT("MachineSpirit: Weight value: %f"), Direction.Value));
	}
}

ESteerDirection AMachineSpirit::DefineSteerDirection()
{
	// ����� "�����" �����������: ��(0), �(1), ��(2).
	if (DirectionWeightMap.Num() != 3)
	{
		DirectionWeightMap.Empty();
		DirectionWeightMap.Add(FDirectionWeight{ ESteerDirection::Left, 0.f });
		DirectionWeightMap.Add(FDirectionWeight{ ESteerDirection::Forward, 0.f });
		DirectionWeightMap.Add(FDirectionWeight{ ESteerDirection::Right, 0.f });
	}

	//Debug
	for (auto Direction : DirectionWeightMap)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Yellow, FString::Printf(TEXT("MachineSpirit: Weight value: %f"), Direction.Weight));
	}

	FDirectionWeight Direction = { ESteerDirection::Forward, 0.f };

	// ===== Left Direction =====
	DirectionWeightMap[0].Weight = WeightMap["Left"] + WeightMap["LeftForward"] + WeightMap["Forward"];
	// ===== Forward Direction =====
	// *** ��� �������� ����� - ���������� ����� ���������� - ����� ����� ��������� �����?
	DirectionWeightMap[1].Weight = WeightMap["LeftForward"] + WeightMap["Forward"] + WeightMap["RightForward"] - abs(WeightMap["Route"]) - abs(WeightMap["Bounds"]);
	// ===== Right Direction =====
	// ���� ���������� �������� ���� ����� �������� � ������� (���� ��� ������������� - �� �������, ��?)
	// *** ��� �������� ����� - ���������� ���������� - ����� ����� ��������� �����?
	DirectionWeightMap[2].Weight = WeightMap["Forward"] + WeightMap["RightForward"] + WeightMap["Right"] + WeightMap["Route"] + WeightMap["Bounds"];

	// ���, ������ ������ ������ ������ - � �� � ����� ���������� �������� ����� ������ - ����� ����.
	// ������� ������ ��� ��� �������.
	if (Direction.Weight < DirectionWeightMap[1].Weight)
	{
		Direction.Weight = DirectionWeightMap[1].Weight;
		Direction.Direction = DirectionWeightMap[1].Direction;
	}
	// ����� ������.
	if (Direction.Weight < DirectionWeightMap[2].Weight)
	{
		Direction.Weight = DirectionWeightMap[2].Weight;
		Direction.Direction = DirectionWeightMap[2].Direction;
	}
	// � � ����� ��� �����.
	if (Direction.Weight < DirectionWeightMap[0].Weight)
	{
		Direction.Weight = DirectionWeightMap[0].Weight;
		Direction.Direction = DirectionWeightMap[0].Direction;
	}

	return Direction.Direction;
}

float AMachineSpirit::DefineThrottle(ESteerDirection CurrentDirection)
{
	// ���������� �������� ��������.
	// ����������� �������� ���������. �� ��� ����� ������ �����-������ ������� ����������...
	// �� ��� ������������ ������� ������ ������ ������� "����-������" � �������� ��������.
	// ���� ������ �� �����������, ��� ��� ����� �������� ��� ������ ��������� ����������� ���������� - ������ ���������� �����.
	// ����� ����� ������ �������� �����������, �� ��, ��������� ��� ��������� �������.
	// � ����� ����� �������� ����������� � ����� ��������� ��������. ��� ��� ������������.
	// ��������, ��� ����� 0.2. �������� "���� �������".
	float SpeedIntencity = 0.f;
	float ThreatValue = 0.f;
	float PointBoundWeight = WeightMap["Route"] + WeightMap["Bounds"];
	// �������� �� �������� ������������ ��������.
	float SpeedLimiter = 0.f;
	// 4 - �������� �����, ���������� � 5-�������� ����������. 
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

	// ������ ��� ���� �� ��������??????
	// ����������� SpeedLimiter ������, ����� ThreatValue ������.
	// � � ���������� ��������� ���������� �������.
	if (ThreatValue > PointBoundWeight)
	{
		SpeedLimiter = ThreatValue;
	}
	else if (ThreatValue <= PointBoundWeight)
	{
		SpeedLimiter = PointBoundWeight;
	}
	// � ������, ����������, ��������.
	for (int i = 1; i < SpeedSteps; i++)
	{
		// ��� �� �����, ������ if ��������� �� �����������???
		// ������ ��� ������������� SpeedStepAmount = 2, � ����� ��� �� 0?????
		float SpeedStepAmount = float(i) / float(SpeedSteps);
		if (SpeedLimiter <= SpeedStepAmount)
		{
			SpeedIntencity = 1.f - float(((i - 1)/ SpeedSteps));
			break;
		}
	}
	// ���� ������ ����� - ��, ������ ������... � �� ���. �� �����.
	if (SpeedLimiter >= ReverseGearInstigationValue)
	{
		// ������� ������ � ������������ � �������� �������. �� ��� ��� ������ ������ �������.
	}

	//Debug
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Orange, FString::Printf(TEXT("MachineSpirit: Throttle - speed intencity: %f"), SpeedIntencity));

	return SpeedIntencity;
}

void AMachineSpirit::DefineSteerBehaviour()
{
	if (!IsValid(OwnedLandraider)) return;
	// ���������� ����������� ��������.
	FillThreatMap();
	FillWeightMap();
	ESteerDirection CurrentDirection = DefineSteerDirection();
	float FullThrottle = DefineThrottle(CurrentDirection);
	
	// ��� �� ��������.
	if (FullThrottle > 0.f)
	{
		float CurrentSpeed = OwnedLandraider->GetLandraiderMovementComponent()->GetCurrentSpeed();
		float MaxSpeed = OwnedLandraider->GetLandraiderMovementComponent()->GetMaxSpeed();
		// ���� �������� ������, ��� ���� - ��������.
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
	// ������ �������.
	// TODO: ���������� ������� �������� ���� �� ����������.
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
	// �����, ���� ��� �� �������???!
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