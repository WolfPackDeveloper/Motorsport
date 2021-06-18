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
#include "Kismet/KismetSystemLibrary.h" // Trace
#include "Math/Box.h"


AMachineSpirit::AMachineSpirit()
{
	SpiritState = EMachineSpiritState::Sleeps;
	//SteerBehaviour = ESteerBehaviour::Wait;
	GroundBounds = FVector();
}

FVector AMachineSpirit::GetOwnedPawnDimensions()
{
	if (IsValid(OwnedLandraider))
	{
		FVector Origin = FVector();
		FVector Extent = FVector();

		OwnedLandraider->GetActorBounds(true, Origin, Extent);

		return Extent - Origin;
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
		OwnedLandraider->GetMovementComponent()->SetDriveAction(EDriveAction::FullThrottle, InputIntensity);
	}
}

void AMachineSpirit::ReverseGear(float InputIntensity)
{
	if (IsValid(OwnedLandraider))
	{
		OwnedLandraider->GetMovementComponent()->SetDriveAction(EDriveAction::ReverseGear, InputIntensity);
	}
}

void AMachineSpirit::Brake(float InputIntensity)
{
	if (IsValid(OwnedLandraider))
	{
		OwnedLandraider->GetMovementComponent()->SetDriveAction(EDriveAction::Braking, InputIntensity);
	}
}

void AMachineSpirit::IdleMoving(float InputIntensity)
{
	if (IsValid(OwnedLandraider))
	{
		OwnedLandraider->GetMovementComponent()->SetDriveAction(EDriveAction::IdleMoving, InputIntensity);
	}
}

void AMachineSpirit::TurnRight(float InputIntensity)
{
	if (IsValid(OwnedLandraider))
	{
		OwnedLandraider->GetMovementComponent()->Steering(InputIntensity);
	}
}

//void AMachineSpirit::BuildRoute()
//{
//	if (!GameMode) return;
//	CurrentRoute = GameMode->GetRouteActor();
//
//	if (IsValid(CurrentRoute))
//	{
//		SteerBehaviour = ESteerBehaviour::FollowTheRoute;
//		CurrentRoutePointIndex = 0;
//	}
//	else
//	{
//		SteerBehaviour = ESteerBehaviour::Explore;
//	}
//}

//void AMachineSpirit::ExploreSteerBehaviour()
//{
//	if (!IsValid(OwnedLandraider)) return;
//
//	// "�����������" �������
//	TraceGroundBounds();
//
//	// ���������� ��� "������� ����� ��������" ����������� � ��������.
//
//
//
//}

void AMachineSpirit::BeginPlay()
{
	Super::BeginPlay();

	GameMode = Cast<AMotorsportGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (IsValid(GameMode))
	{
		CurrentRoute = GameMode->GetRouteActor();
	}

	OwnedLandraider = Cast<ALandraider>(GetPawn());

	if (IsValid(OwnedLandraider))
	{
		PawnDimensions = GetOwnedPawnDimensions();
	}

	//BuildRoute();
}

void AMachineSpirit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (SpiritState == EMachineSpiritState::Awake)
	{

	}

}

void AMachineSpirit::TraceLeft()
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwnedLandraider);
	float Distance = 0; // ���������� �� ����� ������������ ���� � ������������. ��� ������� ������ ������.
	FVector TraceStart;
	FVector TraceEnd;

	float X = OwnedLandraider->GetActorForwardVector().X;
	float Y = OwnedLandraider->GetActorForwardVector().Y - ((1 / 2) * PawnDimensions.Y);
	float Z = OwnedLandraider->GetActorForwardVector().Z;
	TraceStart = FVector(X, Y, Z);
	// �����, �������, ��������� ����� �� X... �� ������ � �����...
	// ��������� ���� �����-������ �������� � �������������, ��, ������.
	TraceEnd = FVector(TraceStart.X + TraceLength - PawnDimensions.X, TraceStart.Y - ((1 / 2) * PawnDimensions.Y), TraceStart.Z);
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
		5.f
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
}

void AMachineSpirit::TraceForwardLeft()
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwnedLandraider);
	float Distance = 0; // ���������� �� ����� ������������ ���� � ������������. ��� ������� ������ ������.
	FVector TraceStart;
	FVector TraceEnd;
	float X = OwnedLandraider->GetActorForwardVector().X;
	float Y = OwnedLandraider->GetActorForwardVector().Y - ((1 / 2) * PawnDimensions.Y);
	float Z = OwnedLandraider->GetActorForwardVector().Z;
	TraceStart = FVector(X, Y, Z);
	TraceEnd = FVector(TraceStart.X + TraceLength, TraceStart.Y, TraceStart.Z);
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
		5.f
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
}

void AMachineSpirit::TraceForward()
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwnedLandraider);
	float Distance = 0; // ���������� �� ����� ������������ ���� � ������������. ��� ������� ������ ������.
	FVector TraceStart = OwnedLandraider->GetActorForwardVector();
	FVector TraceEnd = FVector(TraceStart.X + TraceLength, TraceStart.Y, TraceStart.Z);
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
		5.f
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
}

void AMachineSpirit::TraceForwardRight()
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwnedLandraider);
	float Distance = 0; // ���������� �� ����� ������������ ���� � ������������. ��� ������� ������ ������.
	FVector TraceStart;
	FVector TraceEnd;
	float X = OwnedLandraider->GetActorForwardVector().X;
	float Y = OwnedLandraider->GetActorForwardVector().Y + ((1 / 2) * PawnDimensions.Y);
	float Z = OwnedLandraider->GetActorForwardVector().Z;
	TraceStart = FVector(X, Y, Z);
	TraceEnd = FVector(TraceStart.X + TraceLength, TraceStart.Y, TraceStart.Z);
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
		5.f
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
}

void AMachineSpirit::TraceRight()
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwnedLandraider);
	float Distance = 0; // ���������� �� ����� ������������ ���� � ������������. ��� ������� ������ ������.
	FVector TraceStart;
	FVector TraceEnd;

	float X = OwnedLandraider->GetActorForwardVector().X;
	float Y = OwnedLandraider->GetActorForwardVector().Y + ((1 / 2) * PawnDimensions.Y);
	float Z = OwnedLandraider->GetActorForwardVector().Z;
	TraceStart = FVector(X, Y, Z);
	// �����, �������, ��������� ����� �� X... �� ������ � �����...
	// ��������� ���� �����-������ �������� � �������������, ��, ������.
	TraceEnd = FVector(TraceStart.X + TraceLength - PawnDimensions.X, TraceStart.Y + ((1 / 2) * PawnDimensions.Y), TraceStart.Z);
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
		5.f
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
}

void AMachineSpirit::CalculateThreatAmount()
{
	// �� �� ���������� �� � ������ ������� ��� ��� ��������...
	if (!IsValid(OwnedLandraider)) return;
	// ����� ��������� � �����-������ ������� ��������� ��� ������������� ���������� �����������.
	if (ThreatMap.Num() != 5)
	{
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
	if (!IsValid(CurrentRoute) || !IsValid(OwnedLandraider)) return -1;
	
	if (CurrentRoutePointIndex < 0 || CurrentRoutePointIndex >= CurrentRoute->GetRouteSpline()->GetNumberOfSplinePoints())
	{
		return -1;
	}

	// ������. ��� ������������ �����, ��� ������������ ����� ��������.
	FVector Distance = CurrentRoute->GetRouteSpline()->GetWorldLocationAtSplinePoint(CurrentRoutePointIndex) - OwnedLandraider->GetActorForwardVector();
	// ��� ���������! � �� ����� ������� ���� ��������!
	bool bCurrentPointIsReached = (abs(Distance.X) < PointReachField) || (abs(Distance.Y) < PointReachField);
	
	if (bCurrentPointIsReached)
	{
		++CurrentRoutePointIndex;
	}

	return CurrentRoutePointIndex;
}

float AMachineSpirit::DefineRoutePointWeight()
{
	// ���������� ��������� �������� ����� �������� � ���������� �� ��.
	//DefineCurrentRoutePoint();

	if (DefineCurrentRoutePoint() < 0)
	{
		return 0.f;
	}

	float Weight = 0.f;

	// ���������� ���������� �� �����: ���� ������ ������� ����������� �� �������� -0.1-0-0.1, � ����������� �� �����������.
	// ����� ���������� �����������
	FVector RouteLocation = CurrentRoute->GetRouteSpline()->GetWorldLocationAtSplinePoint(CurrentRoutePointIndex);
	FVector CurrentLocation = OwnedLandraider->GetActorForwardVector();
	FVector DeltaLocation = RouteLocation - CurrentLocation;
	float Distance = sqrtf(powf(DeltaLocation.X, 2.f) + powf(DeltaLocation.Y, 2.f));

	// ���� ���� ����� �� �����, �� ���������� �����.
	if (DeltaLocation.Y == 0.f) return 0.f;
	// ���� ���, �� ������� ���� ���������.
	if (Distance > TraceLength)
	{
		Weight = 0.1f;
	}
	else
	{
		Weight = 1 - (Distance / TraceLength);
	}

	if (DeltaLocation.Y < 0.f)
	{
		return Weight *= -1.f;
	}

	return Weight;
}

float AMachineSpirit::DefineGroundBoundsWeight()
{	
	// ���������� "����" �����������.
	FVector Forward = OwnedLandraider->GetActorForwardVector();
	Forward.X += TraceLength;
	FVector Left = OwnedLandraider->GetActorForwardVector();
	Left.X += (TraceLength - PawnDimensions.X);
	Left.Y -= PawnDimensions.Y;
	FVector Right = OwnedLandraider->GetActorForwardVector();
	Right.X += (TraceLength - PawnDimensions.X);
	Right.Y += PawnDimensions.Y;
	// ��� ��������� � ���������� �� -1 �� 1. ���� ���������� ����������� ��������: ����-�����.
	float Weight = 0.f;

	// ���� ������� ������� - ������������ �������.
	if (Forward.X >= GroundBounds.X || Forward.X <= -GroundBounds.X)
	{
		// ���� ������ ���� ������� - ������������ ������.
		Weight = (abs(Forward.X) - abs(GroundBounds.X)) / TraceLength;
		if (Right.X >= GroundBounds.X || Right.X <= -GroundBounds.X)
		{
			Weight *= -1.f;
		}
	}
	// ����� ����� ����������, ����� �� ����������� �������� � ������ "�������������" ������ �� �������.
	else if (Forward.X >= GroundBounds.Y || Forward.X <= -GroundBounds.Y)
	{
		Weight = (abs(Forward.X) - abs(GroundBounds.Y)) / TraceLength;
		if (Right.X >= GroundBounds.X || Right.X <= -GroundBounds.X)
		{
			Weight *= -1.f;
		}
	}

	return Weight;
}

void AMachineSpirit::CalculateDirectionsWeight()
{
	// ����� "�����" �����������: ��(0), �(1), ��(2).
	if(DirectionWeightMap.Num() < 3)
	for (int i = 0; i < 3; ++i)
	{
		DirectionWeightMap.Add(0.f);
	}

	// ===== Left Direction =====
	DirectionWeightMap[0] = (1 - ThreatMap[0].ThreatValue) + (1 - ThreatMap[1].ThreatValue) + (1 - ThreatMap[2].ThreatValue);

	// ===== Forward Direction =====
	DirectionWeightMap[1] = (1 - ThreatMap[1].ThreatValue) + (1 - ThreatMap[2].ThreatValue) + (1 - ThreatMap[3].ThreatValue);

	// ===== Right Direction =====
	// ���� ���������� �������� ���� ����� �������� � ������� (���� ��� ������������� - �� �������, ��?)
	DirectionWeightMap[2] = (1 - ThreatMap[2].ThreatValue) + (1 - ThreatMap[3].ThreatValue) + (1 - ThreatMap[4].ThreatValue) + DefineRoutePointWeight() + DefineGroundBoundsWeight();
}

void AMachineSpirit::DefineSteerAction()
{
	CalculateThreatAmount();
	CalculateDirectionsWeight();

	if (ThreatMap.Num() < 5 || DirectionWeightMap.Num() < 3) return;

	//if(DirectionWeightMap[1] == 3)

	// ���� ���� ��������� ����� - ����� � ���.
	// ������ - ��� ��������� "�������" ���, � �� ���������� � ��������� - ���� � 5 ���������� �� ����� ���� ������ ������...
	// ���� ������... �� ��� ������. ������� ������ �� �������� �� ����� "������".
	// ���� "�������" � ����� < 25% (��� ������ � ������ 0.25) - ������������ �� ����������.
	// ���� ��� �������� 25% < 50% (��� ������ � ������ 0.5) - �������� �� ������ � �������� ��������.
	// ���� ��� �������� 50% < 75% (��� ������ � ������ 0.75) - ������������ �� ���� ���� �� ����������� ��������.

	// ��� ���������� ������� � ������ �����, ������� ����� ����� �������� �����-������.
	// ���� ��� > 66% (70-75) - ������� ������ � ������ ���� � ������ �������, ����� �������� ����� �����.
}


//void AMachineSpirit::ObstacleTracing()
//{
//
//
//}

EMachineSpiritState AMachineSpirit::GetMachineSpiritState()
{
	return SpiritState;
}

void AMachineSpirit::SetMachineSpiritState(EMachineSpiritState State)
{
	SpiritState = State;
}