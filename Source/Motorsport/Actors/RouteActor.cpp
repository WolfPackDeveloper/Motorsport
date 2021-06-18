// Fill out your copyright notice in the Description page of Project Settings.


#include "RouteActor.h"
#include "MotorsportGameModeBase.h"

#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Engine/Engine.h" // Debug strings
#include "Kismet/GameplayStatics.h" // GetGameMode

// Sets default values
ARouteActor::ARouteActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//RouteBoundMargin = DefaultMargin;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	RouteSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RouteSpline"));
	RouteSpline->SetupAttachment(Root);
	RouteSpline->SetDrawDebug(true);
	RouteSpline->ScaleVisualizationWidth = 50.f;
}

void ARouteActor::GetBoundsForRoute()
{
	if (GameMode)
	{
		RouteBounds = GameMode->GetGroundBounds();
		RouteBounds.X -= RouteBoundMargin;
		RouteBounds.Y -= RouteBoundMargin;
	}

	// Debug
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Bound Extent %s"), *RouteBounds.ToString()));
}

FVector ARouteActor::GetDistanceToBounds(EDirection UpDown, EDirection RightLeft)
{
	FVector DistanceToBound = FVector();
	FVector CurrentPosition = GetActorLocation();
	// ��, ����� ����������.
	FVector CurentTangent = FVector();

	int32 LastPointIndex = RouteSpline->GetNumberOfSplinePoints() - 1;

	RouteSpline->GetLocationAndTangentAtSplinePoint(LastPointIndex, CurrentPosition, CurentTangent, ESplineCoordinateSpace::World);

	switch (UpDown)
	{
	case EDirection::Up:
		// ���������� �� �������� ���� ����.
		DistanceToBound.X = RouteBounds.X - CurrentPosition.X;
		break;

	case EDirection::Down:
		// ���������� �� ������� ���� ����.
		DistanceToBound.X = abs(-RouteBounds.X - CurrentPosition.X);
		break;

	default:
		break;
	}
	
	switch (RightLeft)
	{
	case EDirection::Right:
		// ���������� �� ������� ���� ����.
		DistanceToBound.Y = RouteBounds.Y - CurrentPosition.Y;
		break;

	case EDirection::Left:
		// ���������� �� ������ ���� ����.
		DistanceToBound.Y = abs(-RouteBounds.Y - CurrentPosition.Y);
		break;
	
	default:
		break;
	}

	// Debug
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("Distance to bounds: %s"), *DistanceToBound.ToString()));

	return DistanceToBound;
}

EDirection ARouteActor::BuildTemplateSnake(EDirection InForwardDirection, float ForvardStep, float LagCosinus)
{
	if (!(IsValid(RouteSpline))) return InForwardDirection;
	
	// ������ �� 8 ����. ����� (������) - 6. ������ =~ 1.33
	EDirection ForwardDirection = InForwardDirection;
	float Hypotenuse = ForvardStep / LagCosinus;
	float LongCathetus = ForvardStep;
	float ShortCathetusCosinus = powf(1 - powf(LagCosinus, 2.f), 1/2);
	float ShortCathetus = Hypotenuse * ShortCathetusCosinus;
	FVector PointLocation = FVector();
	FVector PointTangent = FVector();

	// ���������� ��������� �����
	int StartPointIndex = RouteSpline->GetNumberOfSplinePoints() - 1;
	
	if (StartPointIndex < 0)
	{
		StartPointIndex = 0;
	}
	
	RouteSpline->GetLocationAndTangentAtSplinePoint(StartPointIndex, PointLocation, PointTangent, ESplineCoordinateSpace::World);

	switch (ForwardDirection)
	{
	case EDirection::Up:
		// ����� 1 � ����� ������.
		PointLocation = FVector(PointLocation.X + LongCathetus, PointLocation.Y + ShortCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 2 � ����� ������.
		//PointLocation = FVector(PointLocation.X + LongCathetus, PointLocation.Y + ShortCathetus, PointLocation.Z);
		//RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 2 ������.
		PointLocation = FVector(PointLocation.X + LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 3 ������.
		PointLocation = FVector(PointLocation.X + LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 4 ������.
		PointLocation = FVector(PointLocation.X + LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 5 � ����� �����.
		PointLocation = FVector(PointLocation.X + LongCathetus, PointLocation.Y - ShortCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 6 � ����� �����.
		//PointLocation = FVector(PointLocation.X + LongCathetus, PointLocation.Y - ShortCathetus, PointLocation.Z);
		//RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 6 ������.
		PointLocation = FVector(PointLocation.X + LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 7 ������.
		PointLocation = FVector(PointLocation.X + LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 8 ������.
		PointLocation = FVector(PointLocation.X + LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Debug
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("SnakeUp has builded.")));
		break;

	case EDirection::Down:
		// ����� 1 � ����� ������.
		PointLocation = FVector(PointLocation.X - LongCathetus, PointLocation.Y - ShortCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 2 � ����� ������.
		//PointLocation = FVector(PointLocation.X - LongCathetus, PointLocation.Y - ShortCathetus, PointLocation.Z);
		//RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 2 ������.
		PointLocation = FVector(PointLocation.X - LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 3 ������.
		PointLocation = FVector(PointLocation.X - LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 4 ������.
		PointLocation = FVector(PointLocation.X - LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 5 � ����� �����.
		PointLocation = FVector(PointLocation.X - LongCathetus, PointLocation.Y + ShortCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 6 � ����� �����.
		//PointLocation = FVector(PointLocation.X - LongCathetus, PointLocation.Y + ShortCathetus, PointLocation.Z);
		//RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 6 ������.
		PointLocation = FVector(PointLocation.X - LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 7 ������.
		PointLocation = FVector(PointLocation.X - LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 8 ������.
		PointLocation = FVector(PointLocation.X - LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Debug
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("SnakeDown has builded.")));
		break;

	case EDirection::Right:
		// ����� 1 � ����� ������.
		PointLocation = FVector(PointLocation.X - ShortCathetus, PointLocation.Y + LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 2 � ����� ������.
		//PointLocation = FVector(PointLocation.X - ShortCathetus, PointLocation.Y + LongCathetus, PointLocation.Z);
		//RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 2 ������.
		PointLocation = FVector(PointLocation.X, PointLocation.Y + LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 3 ������.
		PointLocation = FVector(PointLocation.X, PointLocation.Y + LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 4 ������.
		PointLocation = FVector(PointLocation.X, PointLocation.Y + LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 5 � ����� �����.
		PointLocation = FVector(PointLocation.X + ShortCathetus, PointLocation.Y + LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 6 � ����� �����.
		//PointLocation = FVector(PointLocation.X + ShortCathetus, PointLocation.Y + LongCathetus, PointLocation.Z);
		//RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 6 ������.
		PointLocation = FVector(PointLocation.X, PointLocation.Y + LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 7 ������.
		PointLocation = FVector(PointLocation.X, PointLocation.Y + LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 8 ������.
		PointLocation = FVector(PointLocation.X, PointLocation.Y + LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Debug
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("SnakeRight has builded.")));
		break;

	case EDirection::Left:
		// ����� 1 � ����� ������.
		PointLocation = FVector(PointLocation.X + ShortCathetus, PointLocation.Y - LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 2 � ����� ������.
		//PointLocation = FVector(PointLocation.X + ShortCathetus, PointLocation.Y - LongCathetus, PointLocation.Z);
		//RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 2 ������.
		PointLocation = FVector(PointLocation.X, PointLocation.Y - LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 3 ������.
		PointLocation = FVector(PointLocation.X, PointLocation.Y - LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 4 ������.
		PointLocation = FVector(PointLocation.X, PointLocation.Y - LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 5 � ����� �����.
		PointLocation = FVector(PointLocation.X - ShortCathetus, PointLocation.Y - LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 6 � ����� �����.
		//PointLocation = FVector(PointLocation.X - ShortCathetus, PointLocation.Y - LongCathetus, PointLocation.Z);
		//RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 6 ������.
		PointLocation = FVector(PointLocation.X, PointLocation.Y - LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 7 ������.
		PointLocation = FVector(PointLocation.X, PointLocation.Y - LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// ����� 8 ������.
		PointLocation = FVector(PointLocation.X, PointLocation.Y - LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Debug
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("SnakeLeft has builded.")));
		break;

	default:
		break;
	}
	
	 // ���������� ���� �� ������.
	 // ���������� �������� �����.
	int EndPointIndex = RouteSpline->GetNumberOfSplinePoints() - 1;
	
	if (EndPointIndex < 0)
	{
		EndPointIndex = 0;
	}

	UpdateMesh(StartPointIndex, EndPointIndex);

	return ForwardDirection;
}

EDirection ARouteActor::BuildTemplateTurn(EDirection InForwardDirection, EDirection InTurnDirection, float ForvardStep, float LagCosinus)
{
	// ������ �� 5 ����. ����� (������) = 2. ������ = 4.
	// ���� ���� 45 ��������.
	
	if (!(IsValid(RouteSpline))) return InForwardDirection;

	EDirection ForwardDirection = InForwardDirection;
	EDirection TurnDirection = InTurnDirection;
	float Hypotenuse = ForvardStep / LagCosinus;
	float Cathetus = ForvardStep;
	//float ShortCathetusCosinus = powf(1 - powf(LagCosinus, 2.f), 1 / 2);
	//float ShortCathetus = LongCathetus;
	FVector PointLocation = FVector();
	FVector PointTangent = FVector();

	// ���������� ��������� �����
	int StartPointIndex = RouteSpline->GetNumberOfSplinePoints() - 1;

	if (StartPointIndex < 0)
	{
		StartPointIndex = 0;
	}

	RouteSpline->GetLocationAndTangentAtSplinePoint(StartPointIndex, PointLocation, PointTangent, ESplineCoordinateSpace::World);

	switch (ForwardDirection)
	{
	case EDirection::Up:	
		// �������� ���� �����...
		switch (TurnDirection)
		{
		case EDirection::Right:
			// ...������ �������
			// ����� 1 �����. ��� ����� �������� ��� �������� ���������� � �������� ��������.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 2 � ����� ����� � ������ 45 ��������.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 3 � ����� ������ 45 ��������.
			PointLocation = FVector(PointLocation.X, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 3+ ��� ���������� ������� ��������.
			PointLocation = FVector(PointLocation.X, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 4 � ����� ������ � ���� 45 ��������.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 5 � ����� ���� 45.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);

			ForwardDirection = EDirection::Down;
			// Debug
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("TurnUpRightDown has builded.")));
			break;
		
		case EDirection::Left:
			// ...����� �������
			// ����� 1 �����. ��� ����� �������� ��� �������� ���������� � �������� ��������.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 2 � ����� ����� � ����� 45 ��������.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 3 � ����� ����� 45 ��������.
			PointLocation = FVector(PointLocation.X, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 3+ ��� ���������� ������� ��������.
			PointLocation = FVector(PointLocation.X, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 4 � ����� ����� � ���� 45 ��������.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 5 � ����� ���� 45.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);

			ForwardDirection = EDirection::Down;
			// Debug
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("TurnUpLeftDown has builded.")));
			break;

		default:
			break;
		}	
		break;

	case EDirection::Down:
		// �������� ����� �����...
		switch (TurnDirection)
		{
		case EDirection::Right:
			// ...������ �������
			// ����� 1 ����. ��� ����� �������� ��� �������� ���������� � �������� ��������.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 2 � ����� ���� � ������ 45 ��������.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 3 � ����� ������ 45 ��������.
			PointLocation = FVector(PointLocation.X, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 3+ ��� ���������� ������� ��������.
			PointLocation = FVector(PointLocation.X, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 4 � ����� ������ � ����� 45 ��������.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 5 � ����� ����� 45.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);

			ForwardDirection = EDirection::Up;
			// Debug
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("TurnDownRightUp has builded.")));
			break;

		case EDirection::Left:
			// ...����� �������
			// ����� 1 ����. ��� ����� �������� ��� �������� ���������� � �������� ��������.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 2 � ����� ���� � ����� 45 ��������.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 3 � ����� ����� 45 ��������.
			PointLocation = FVector(PointLocation.X, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 3+ ��� ���������� ������� ��������.
			PointLocation = FVector(PointLocation.X, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 4 � ����� ����� � ����� 45 ��������.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 5 � ����� ����� 45.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);

			ForwardDirection = EDirection::Up;
			// Debug
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("TurnDownLeftUp has builded.")));
			break;

		default:
			break;
		}
		break;

	case EDirection::Right:
		// �������� ����� �����...
		switch (TurnDirection)
		{
		case EDirection::Up:
			// ...������� �������
			// ����� 1 ������. ��� ����� �������� ��� �������� ���������� � �������� ��������.
			PointLocation = FVector(PointLocation.X, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 2 � ����� ������ � ����� 45 ��������.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 3 � ����� ����� 45 ��������.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 3+ ��� ���������� ������� ��������.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 4 � ����� ����� � ����� 45 ��������.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 5 � ����� ����� 45.
			PointLocation = FVector(PointLocation.X, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);

			ForwardDirection = EDirection::Left;
			// Debug
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("TurnRightUpLeft has builded.")));
			break;

		case EDirection::Down:
			// ...������ �������
			// ����� 1 ������. ��� ����� �������� ��� �������� ���������� � �������� ��������.
			PointLocation = FVector(PointLocation.X, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 2 � ����� ������ � ���� 45 ��������.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 3 � ����� ���� 45 ��������.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 3+ ��� ���������� ������� ��������.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 4 � ����� ���� � ����� 45 ��������.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 5 � ����� ����� 45.
			PointLocation = FVector(PointLocation.X, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);

			ForwardDirection = EDirection::Left;
			// Debug
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("TurnRightDownLeft has builded.")));
			break;

		default:
			break;
		}
		break;

	case EDirection::Left:
		// �������� ������ �����...
		switch (TurnDirection)
		{
		case EDirection::Up:
			// ...������� �������
			// ����� 1 �����. ��� ����� �������� ��� �������� ���������� � �������� ��������.
			PointLocation = FVector(PointLocation.X, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 2 � ����� ����� � ����� 45 ��������.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 3 � ����� ����� 45 ��������.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 3+ ��� ���������� ������� ��������.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 4 � ����� ����� � ������ 45 ��������.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 5 � ����� ������ 45.
			PointLocation = FVector(PointLocation.X, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);

			ForwardDirection = EDirection::Right;
			// Debug
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("TurnLeftUpRight has builded.")));
			break;

		case EDirection::Down:
			// ...������ �������
			// ����� 1 �����. ��� ����� �������� ��� �������� ���������� � �������� ��������.
			PointLocation = FVector(PointLocation.X, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 2 � ����� ����� � ���� 45 ��������.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 3 � ����� ���� 45 ��������.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 3+ ��� ���������� ������� ��������.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 4 � ����� ���� � ������ 45 ��������.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// ����� 5 � ����� ������ 45.
			PointLocation = FVector(PointLocation.X, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);

			ForwardDirection = EDirection::Right;
			// Debug
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("TurnLeftDownRight has builded.")));
			break;

		default:
			break;
		}
		break;
		
	default:
		break;
	}

	int EndPointIndex = RouteSpline->GetNumberOfSplinePoints() - 1;
	
	if (EndPointIndex < 0)
	{
		EndPointIndex = 0;
	}
	
	UpdateMesh(StartPointIndex, EndPointIndex);

	return ForwardDirection;
}

EDirection ARouteActor::BuildTemplateFiller(EDirection InForwardDirection, float ForvardStep, float LagCosinus)
{
	// ������ �� 1 �����. ����� (������) = 1. ������ = 0.
	// ���� ���� 0 ��������.
	
	if (!(IsValid(RouteSpline))) return InForwardDirection;

	//float Hypotenuse = ForvardStep / LagCosinus;
	//float LongCathetus = ForvardStep;
	//float ShortCathetusCosinus = powf(1 - powf(LagCosinus, 2.f), 1 / 2);
	//float ShortCathetus = Hypotenuse * ShortCathetusCosinus;
	
	EDirection ForwardDirection = InForwardDirection;
	FVector PointLocation = FVector();
	FVector PointTangent = FVector();

	// ���������� ��������� �����
	int StartPointIndex = RouteSpline->GetNumberOfSplinePoints() - 1;

	if (StartPointIndex < 0)
	{
		StartPointIndex = 0;
	}

	RouteSpline->GetLocationAndTangentAtSplinePoint(StartPointIndex, PointLocation, PointTangent, ESplineCoordinateSpace::World);

	switch (ForwardDirection)
	{
	case EDirection::Up:
		// 1 ����� �����.
		PointLocation = FVector(PointLocation.X + ForvardStep, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);

		// Debug
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("FillerUp has builded.")));
		break;

	case EDirection::Down:
		// 1 ����� �����.
		PointLocation = FVector(PointLocation.X - ForvardStep, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);

		// Debug
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("FillerDown has builded.")));
		break;

	case EDirection::Right:
		// 1 ����� �����.
		PointLocation = FVector(PointLocation.X, PointLocation.Y + ForvardStep, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);

		// Debug
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("FillerRight has builded.")));
		break;

	case EDirection::Left:
		// 1 ����� �����.
		PointLocation = FVector(PointLocation.X, PointLocation.Y - ForvardStep, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);

		// Debug
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("FillerLeft has builded.")));
		break;

	default:
		break;
	}

	int EndPointIndex = RouteSpline->GetNumberOfSplinePoints() - 1;

	if (EndPointIndex < 0)
	{
		EndPointIndex = 0;
	}

	UpdateMesh(StartPointIndex, EndPointIndex);

	return ForwardDirection;
}

void ARouteActor::UpdateMesh(int StartPointIndex, int EndPointIndex)
{
	if (!(IsValid(RouteSpline))) return;
	if (!RoadMesh) return;
	if (StartPointIndex == EndPointIndex) return;

	// -1 ������ ��� �������  ������� �� ����� �� ��������� �����.
	for (int PointIndex = StartPointIndex; PointIndex < EndPointIndex; PointIndex++)
	{
		USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());

		SplineMesh->SetStaticMesh(RoadMesh);
		SplineMesh->SetMobility(EComponentMobility::Movable);
		SplineMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		SplineMesh->RegisterComponentWithWorld(GetWorld());
		SplineMesh->AttachToComponent(RouteSpline, FAttachmentTransformRules::KeepRelativeTransform);
	
		FVector StartPointLocation = RouteSpline->GetLocationAtSplinePoint(PointIndex, ESplineCoordinateSpace::Local);
		FVector StartPontTangent = RouteSpline->GetTangentAtSplinePoint(PointIndex, ESplineCoordinateSpace::Local);
		FVector EndPointLocation = RouteSpline->GetLocationAtSplinePoint(PointIndex + 1, ESplineCoordinateSpace::Local);
		FVector EndPontTangent = RouteSpline->GetTangentAtSplinePoint(PointIndex + 1, ESplineCoordinateSpace::Local);

		SplineMesh->SetStartAndEnd(StartPointLocation, StartPontTangent, EndPointLocation, EndPontTangent, true);
		SplineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		MeshesOnTheRoad.Add(SplineMesh);
	}
}

void ARouteActor::BuildRouteSnake(EDirection InForwardDirection, EDirection InTurnDirection)
{
	// ���������� �������� ���������� ��� ���������� ��������.
	// ���������� ����� ����.
	// ����� �������. � ���� - ���������� ��������� � ����� � ����� ������� ������ �������� (������ = 8) + 1 ����� ���� ��� ���������.
	EDirection CurrentDirection = InForwardDirection;
	// ��� ��� ������� � ������� ����� �� ������� ���������.
	int8 SnakeSectionsForward = 8;
	int8 SnakeSectionsRight = 2;
	int8 TurnSectionsForward = 2;
	int8 TurnSectionsRight = 4;
	int8 FillerSectionForward = 1;
	int8 FillerSectionRight = 0;
	// ������������� �������� ������, �� ���� �������������� ����� ��� ���������� ����... �� �����, � ���� �� ����.
	int8 ReserveForvardSections = 2;
	int8 ReserveRightSections = 0;

	// TODO: ����������� ����������� Spline ��� ���������� ��������.

	int32 MaxSectionsComboForward = SnakeSectionsForward + TurnSectionsForward + FillerSectionForward + ReserveForvardSections;
	int32 MaxSectionsComboRight = SnakeSectionsRight + TurnSectionsRight + FillerSectionRight + ReserveForvardSections;
	
	// ���� ����� ����� ������� ����-������.
	float Cosinus0 = 1.f;
	float Cosinus30 = 0.866f;
	float Cosinus45 = 0.7071f;
	// ��� ���������� �������� (Filler) � ������� ������ ��������, ����� ����� �� ���������� �� ������� ������.
	// �� ����� ����� � � ����, ��� �� ����� �� ������������ �������.
	float Cosinus90 = 0.f;

	float DeltaForward = 0.f;
	bool RouteFinished = false;
	FVector CurrentDistanceToBounds = FVector();
	
	switch (CurrentDirection)
	{
	case EDirection::Up:
		DeltaForward = (RouteBounds.X * 2) / MaxSectionsComboForward;
		break;
	case EDirection::Down:
		DeltaForward = (RouteBounds.X * 2) / MaxSectionsComboForward;
		break;
	case EDirection::Right:
		DeltaForward = (RouteBounds.Y * 2) / MaxSectionsComboForward;
		break;
	case EDirection::Left:
		DeltaForward = (RouteBounds.Y * 2) / MaxSectionsComboForward;
		break;
	default:
		break;
	}

	// �������� �������� - ���� �����, �� �������. ���� - ������.
	while (!RouteFinished)
	{
		CurrentDistanceToBounds = GetDistanceToBounds(CurrentDirection, InTurnDirection);
		bool UpEnoughth = false;
		bool RightEnoughth = false;

		// �������� ��������������, ��������.
		switch (CurrentDirection)
		{
		case EDirection::Up:
			UpEnoughth = CurrentDistanceToBounds.X >= DeltaForward * (SnakeSectionsForward + TurnSectionsForward);
			RightEnoughth = CurrentDistanceToBounds.Y >= DeltaForward * SnakeSectionsRight;
			
			if (UpEnoughth && RightEnoughth)
			{
				CurrentDirection = BuildTemplateSnake(CurrentDirection, DeltaForward, Cosinus30);
			}
			else
			{
				UpEnoughth = CurrentDistanceToBounds.X >= DeltaForward * (FillerSectionForward + TurnSectionsForward);
				RightEnoughth = CurrentDistanceToBounds.Y >= DeltaForward * FillerSectionRight;

				if (UpEnoughth && RightEnoughth)
				{
					CurrentDirection = BuildTemplateFiller(CurrentDirection, DeltaForward, Cosinus0);
				}
				else
				{
					UpEnoughth = CurrentDistanceToBounds.X >= DeltaForward * (TurnSectionsForward);
					RightEnoughth = CurrentDistanceToBounds.Y >= DeltaForward * TurnSectionsRight;

					if (UpEnoughth && RightEnoughth)
					{
						CurrentDirection = BuildTemplateTurn(CurrentDirection, InTurnDirection, DeltaForward, Cosinus45);
					}
					else
					{
						// ���� ��������� ������� ������� �� ������� - ��, �����.
						// ����������� ������, � �������� �������.
						CurrentDirection = BuildTemplateFiller(EDirection::Left, DeltaForward, Cosinus0);
						GetRouteSpline()->SetClosedLoop(true);
						RouteFinished = true;
					}
				}
			}
			break;
		case EDirection::Down:
			UpEnoughth = CurrentDistanceToBounds.X >= DeltaForward * (SnakeSectionsForward + TurnSectionsForward);
			RightEnoughth = CurrentDistanceToBounds.Y >= DeltaForward * SnakeSectionsRight;

			if (UpEnoughth && RightEnoughth)
			{
				CurrentDirection = BuildTemplateSnake(CurrentDirection, DeltaForward, Cosinus30);
			}
			else
			{
				UpEnoughth = CurrentDistanceToBounds.X >= DeltaForward * (FillerSectionForward + TurnSectionsForward);
				RightEnoughth = CurrentDistanceToBounds.Y >= DeltaForward * FillerSectionRight;

				if (UpEnoughth && RightEnoughth)
				{
					CurrentDirection = BuildTemplateFiller(CurrentDirection, DeltaForward, Cosinus0);
				}
				else
				{
					UpEnoughth = CurrentDistanceToBounds.X >= DeltaForward * (TurnSectionsForward);
					RightEnoughth = CurrentDistanceToBounds.Y >= DeltaForward * TurnSectionsRight;

					if (UpEnoughth && RightEnoughth)
					{
						CurrentDirection = BuildTemplateTurn(CurrentDirection, InTurnDirection, DeltaForward, Cosinus45);
					}
					else
					{
						// ���� ��������� ������� ������� �� ������� - ��, �����.
						// ����������� ������, � �������� �������.
						CurrentDirection = BuildTemplateFiller(EDirection::Left, DeltaForward, Cosinus0);
						GetRouteSpline()->SetClosedLoop(true);
						RouteFinished = true;
					}
				}
			}
			break;
		case EDirection::Right:
			// ����� ����� �������� ����� �� ����, �� ������, ���� ������� ����� ����� ������� �� ����������� Y.
			// ���� ����� ���������.
			break;
		case EDirection::Left:
			// ����� ����� �������� ����� �� ����, �� ������, ���� ������� ����� ����� ������� �� ����������� Y.
			// ���� ����� ���������.
			break;
		default:
			
			break;
		}

	}

	//TODO: ��������� �������.

}

// Called when the game starts or when spawned
void ARouteActor::BeginPlay()
{
	Super::BeginPlay();
	
	GameMode = Cast<AMotorsportGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	GetBoundsForRoute();

	if (IsValid(RouteSpline))
	{
		int32 EndPointIndex = RouteSpline->GetNumberOfSplinePoints() - 1;
		UpdateMesh(0, EndPointIndex);
	}
}

// Called every frame
void ARouteActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//void ARouteActor::OnConstruction(const FTransform& Transform)
//{
//
//	if (!(IsValid(RouteSpline))) return;
//
//
//
//	// -1 ������ ��� �������  ������� �� ����� �� ��������� �����.
//	for (int PointIndex = 0; RouteSpline->GetNumberOfSplinePoints()-2; PointIndex++)
//	{
//		USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
//
//		SplineMesh->SetStaticMesh(RoadMesh);
//		SplineMesh->SetMobility(EComponentMobility::Static);
//		SplineMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
//		SplineMesh->RegisterComponentWithWorld(GetWorld());
//		SplineMesh->AttachToComponent(RouteSpline, FAttachmentTransformRules::KeepRelativeTransform);
//
//		FVector StartPointLocation = RouteSpline->GetLocationAtSplinePoint(PointIndex, ESplineCoordinateSpace::Local);
//		FVector StartPontTangent = RouteSpline->GetTangentAtSplinePoint(PointIndex, ESplineCoordinateSpace::Local);
//		FVector EndPointLocation = RouteSpline->GetLocationAtSplinePoint(PointIndex + 1, ESplineCoordinateSpace::Local);
//		FVector EndPontTangent = RouteSpline->GetTangentAtSplinePoint(PointIndex + 1, ESplineCoordinateSpace::Local);
//
//		SplineMesh->SetStartAndEnd(StartPointLocation, StartPontTangent, EndPointLocation, EndPontTangent, true);
//		SplineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
//	}

//}

USplineComponent* ARouteActor::GetRouteSpline() const
{
	return RouteSpline;
}

void ARouteActor::BuildRoute(EDirection InForwardDirection, EDirection InTurnDirection, ERouteType RouteType)
{
	// TODO: �������� ������� �����������: ������ ���� Up->Right, Left; Right->UpDown � �.�.
	
	// �������� ������� ���������� �������� � ����� ������.
	GetBoundsForRoute();
	RouteOrigin = GetActorLocation();

	// ������� ������ �������
	RouteSpline->ClearSplinePoints();
	// ������� ��� ���� ������� ��������.
	for (int i = 0; i < MeshesOnTheRoad.Num(); i++)
	{
		MeshesOnTheRoad[i]->DestroyComponent();
	}

	//UpdateMesh(0, RouteSpline->GetNumberOfSplinePoints() - 1);

	// ������ ������� � ��������� �����������
	switch (RouteType)
	{
	case ERouteType::Snake:
		BuildRouteSnake(InForwardDirection, InTurnDirection);
		break;
	default:
		break;
	}

}

