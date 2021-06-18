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
	// Ну, может пригодится.
	FVector CurentTangent = FVector();

	int32 LastPointIndex = RouteSpline->GetNumberOfSplinePoints() - 1;

	RouteSpline->GetLocationAndTangentAtSplinePoint(LastPointIndex, CurrentPosition, CurentTangent, ESplineCoordinateSpace::World);

	switch (UpDown)
	{
	case EDirection::Up:
		// Расстояние до верхнего края поля.
		DistanceToBound.X = RouteBounds.X - CurrentPosition.X;
		break;

	case EDirection::Down:
		// Расстояние до нижнего края поля.
		DistanceToBound.X = abs(-RouteBounds.X - CurrentPosition.X);
		break;

	default:
		break;
	}
	
	switch (RightLeft)
	{
	case EDirection::Right:
		// Расстояние до правого края поля.
		DistanceToBound.Y = RouteBounds.Y - CurrentPosition.Y;
		break;

	case EDirection::Left:
		// Расстояние до левого края поля.
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
	
	// Фигура из 8 рёбер. Длина (высота) - 6. Ширина =~ 1.33
	EDirection ForwardDirection = InForwardDirection;
	float Hypotenuse = ForvardStep / LagCosinus;
	float LongCathetus = ForvardStep;
	float ShortCathetusCosinus = powf(1 - powf(LagCosinus, 2.f), 1/2);
	float ShortCathetus = Hypotenuse * ShortCathetusCosinus;
	FVector PointLocation = FVector();
	FVector PointTangent = FVector();

	// Координаты начальной точки
	int StartPointIndex = RouteSpline->GetNumberOfSplinePoints() - 1;
	
	if (StartPointIndex < 0)
	{
		StartPointIndex = 0;
	}
	
	RouteSpline->GetLocationAndTangentAtSplinePoint(StartPointIndex, PointLocation, PointTangent, ESplineCoordinateSpace::World);

	switch (ForwardDirection)
	{
	case EDirection::Up:
		// Ребро 1 с лагом вправо.
		PointLocation = FVector(PointLocation.X + LongCathetus, PointLocation.Y + ShortCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 2 с лагом вправо.
		//PointLocation = FVector(PointLocation.X + LongCathetus, PointLocation.Y + ShortCathetus, PointLocation.Z);
		//RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 2 прямое.
		PointLocation = FVector(PointLocation.X + LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 3 прямое.
		PointLocation = FVector(PointLocation.X + LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 4 прямое.
		PointLocation = FVector(PointLocation.X + LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 5 с лагом влево.
		PointLocation = FVector(PointLocation.X + LongCathetus, PointLocation.Y - ShortCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 6 с лагом влево.
		//PointLocation = FVector(PointLocation.X + LongCathetus, PointLocation.Y - ShortCathetus, PointLocation.Z);
		//RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 6 прямое.
		PointLocation = FVector(PointLocation.X + LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 7 прямое.
		PointLocation = FVector(PointLocation.X + LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 8 прямое.
		PointLocation = FVector(PointLocation.X + LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Debug
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("SnakeUp has builded.")));
		break;

	case EDirection::Down:
		// Ребро 1 с лагом вправо.
		PointLocation = FVector(PointLocation.X - LongCathetus, PointLocation.Y - ShortCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 2 с лагом вправо.
		//PointLocation = FVector(PointLocation.X - LongCathetus, PointLocation.Y - ShortCathetus, PointLocation.Z);
		//RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 2 прямое.
		PointLocation = FVector(PointLocation.X - LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 3 прямое.
		PointLocation = FVector(PointLocation.X - LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 4 прямое.
		PointLocation = FVector(PointLocation.X - LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 5 с лагом влево.
		PointLocation = FVector(PointLocation.X - LongCathetus, PointLocation.Y + ShortCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 6 с лагом влево.
		//PointLocation = FVector(PointLocation.X - LongCathetus, PointLocation.Y + ShortCathetus, PointLocation.Z);
		//RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 6 прямое.
		PointLocation = FVector(PointLocation.X - LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 7 прямое.
		PointLocation = FVector(PointLocation.X - LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 8 прямое.
		PointLocation = FVector(PointLocation.X - LongCathetus, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Debug
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("SnakeDown has builded.")));
		break;

	case EDirection::Right:
		// Ребро 1 с лагом вправо.
		PointLocation = FVector(PointLocation.X - ShortCathetus, PointLocation.Y + LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 2 с лагом вправо.
		//PointLocation = FVector(PointLocation.X - ShortCathetus, PointLocation.Y + LongCathetus, PointLocation.Z);
		//RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 2 прямое.
		PointLocation = FVector(PointLocation.X, PointLocation.Y + LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 3 прямое.
		PointLocation = FVector(PointLocation.X, PointLocation.Y + LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 4 прямое.
		PointLocation = FVector(PointLocation.X, PointLocation.Y + LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 5 с лагом влево.
		PointLocation = FVector(PointLocation.X + ShortCathetus, PointLocation.Y + LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 6 с лагом влево.
		//PointLocation = FVector(PointLocation.X + ShortCathetus, PointLocation.Y + LongCathetus, PointLocation.Z);
		//RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 6 прямое.
		PointLocation = FVector(PointLocation.X, PointLocation.Y + LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 7 прямое.
		PointLocation = FVector(PointLocation.X, PointLocation.Y + LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 8 прямое.
		PointLocation = FVector(PointLocation.X, PointLocation.Y + LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Debug
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("SnakeRight has builded.")));
		break;

	case EDirection::Left:
		// Ребро 1 с лагом вправо.
		PointLocation = FVector(PointLocation.X + ShortCathetus, PointLocation.Y - LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 2 с лагом вправо.
		//PointLocation = FVector(PointLocation.X + ShortCathetus, PointLocation.Y - LongCathetus, PointLocation.Z);
		//RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 2 прямое.
		PointLocation = FVector(PointLocation.X, PointLocation.Y - LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 3 прямое.
		PointLocation = FVector(PointLocation.X, PointLocation.Y - LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 4 прямое.
		PointLocation = FVector(PointLocation.X, PointLocation.Y - LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 5 с лагом влево.
		PointLocation = FVector(PointLocation.X - ShortCathetus, PointLocation.Y - LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 6 с лагом влево.
		//PointLocation = FVector(PointLocation.X - ShortCathetus, PointLocation.Y - LongCathetus, PointLocation.Z);
		//RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 6 прямое.
		PointLocation = FVector(PointLocation.X, PointLocation.Y - LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 7 прямое.
		PointLocation = FVector(PointLocation.X, PointLocation.Y - LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Ребро 8 прямое.
		PointLocation = FVector(PointLocation.X, PointLocation.Y - LongCathetus, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
		// Debug
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("SnakeLeft has builded.")));
		break;

	default:
		break;
	}
	
	 // Натягиваем сову на глобус.
	 // Координаты конечной точки.
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
	// Фигура из 5 рёбер. Длина (высота) = 2. Ширина = 4.
	// Угол лага 45 градусов.
	
	if (!(IsValid(RouteSpline))) return InForwardDirection;

	EDirection ForwardDirection = InForwardDirection;
	EDirection TurnDirection = InTurnDirection;
	float Hypotenuse = ForvardStep / LagCosinus;
	float Cathetus = ForvardStep;
	//float ShortCathetusCosinus = powf(1 - powf(LagCosinus, 2.f), 1 / 2);
	//float ShortCathetus = LongCathetus;
	FVector PointLocation = FVector();
	FVector PointTangent = FVector();

	// Координаты начальной точки
	int StartPointIndex = RouteSpline->GetNumberOfSplinePoints() - 1;

	if (StartPointIndex < 0)
	{
		StartPointIndex = 0;
	}

	RouteSpline->GetLocationAndTangentAtSplinePoint(StartPointIndex, PointLocation, PointTangent, ESplineCoordinateSpace::World);

	switch (ForwardDirection)
	{
	case EDirection::Up:	
		// Разворот вниз через...
		switch (TurnDirection)
		{
		case EDirection::Right:
			// ...правый поворот
			// Ребро 1 вверх. Это ребро строится для ровности построения и простоты расчётов.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 2 с лагом вверх и вправо 45 градусов.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 3 с лагом вправо 45 градусов.
			PointLocation = FVector(PointLocation.X, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 3+ для увеличения радиуса поворота.
			PointLocation = FVector(PointLocation.X, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 4 с лагом вправо и вниз 45 градусов.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 5 с лагом вниз 45.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);

			ForwardDirection = EDirection::Down;
			// Debug
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("TurnUpRightDown has builded.")));
			break;
		
		case EDirection::Left:
			// ...левый поворот
			// Ребро 1 вверх. Это ребро строится для ровности построения и простоты расчётов.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 2 с лагом вверх и влево 45 градусов.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 3 с лагом влево 45 градусов.
			PointLocation = FVector(PointLocation.X, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 3+ для увеличения радиуса поворота.
			PointLocation = FVector(PointLocation.X, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 4 с лагом влево и вниз 45 градусов.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 5 с лагом вниз 45.
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
		// Разворот вверх через...
		switch (TurnDirection)
		{
		case EDirection::Right:
			// ...правый поворот
			// Ребро 1 вниз. Это ребро строится для ровности построения и простоты расчётов.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 2 с лагом вниз и вправо 45 градусов.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 3 с лагом вправо 45 градусов.
			PointLocation = FVector(PointLocation.X, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 3+ для увеличения радиуса поворота.
			PointLocation = FVector(PointLocation.X, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 4 с лагом вправо и вверх 45 градусов.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 5 с лагом вверх 45.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);

			ForwardDirection = EDirection::Up;
			// Debug
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("TurnDownRightUp has builded.")));
			break;

		case EDirection::Left:
			// ...левый поворот
			// Ребро 1 вниз. Это ребро строится для ровности построения и простоты расчётов.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 2 с лагом вниз и влево 45 градусов.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 3 с лагом влево 45 градусов.
			PointLocation = FVector(PointLocation.X, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 3+ для увеличения радиуса поворота.
			PointLocation = FVector(PointLocation.X, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 4 с лагом влево и вверх 45 градусов.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 5 с лагом вверх 45.
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
		// Разворот влево через...
		switch (TurnDirection)
		{
		case EDirection::Up:
			// ...верхний поворот
			// Ребро 1 вправо. Это ребро строится для ровности построения и простоты расчётов.
			PointLocation = FVector(PointLocation.X, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 2 с лагом вправо и вверх 45 градусов.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 3 с лагом вверх 45 градусов.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 3+ для увеличения радиуса поворота.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 4 с лагом вверх и влево 45 градусов.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 5 с лагом влево 45.
			PointLocation = FVector(PointLocation.X, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);

			ForwardDirection = EDirection::Left;
			// Debug
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("TurnRightUpLeft has builded.")));
			break;

		case EDirection::Down:
			// ...нижний поворот
			// Ребро 1 вправо. Это ребро строится для ровности построения и простоты расчётов.
			PointLocation = FVector(PointLocation.X, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 2 с лагом вправо и вниз 45 градусов.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 3 с лагом вниз 45 градусов.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 3+ для увеличения радиуса поворота.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 4 с лагом вниз и влево 45 градусов.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 5 с лагом влево 45.
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
		// Разворот вправо через...
		switch (TurnDirection)
		{
		case EDirection::Up:
			// ...верхний поворот
			// Ребро 1 влево. Это ребро строится для ровности построения и простоты расчётов.
			PointLocation = FVector(PointLocation.X, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 2 с лагом влево и вверх 45 градусов.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 3 с лагом вверх 45 градусов.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 3+ для увеличения радиуса поворота.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 4 с лагом вверх и вправо 45 градусов.
			PointLocation = FVector(PointLocation.X + Cathetus, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 5 с лагом вправо 45.
			PointLocation = FVector(PointLocation.X, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);

			ForwardDirection = EDirection::Right;
			// Debug
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("TurnLeftUpRight has builded.")));
			break;

		case EDirection::Down:
			// ...нижний поворот
			// Ребро 1 влево. Это ребро строится для ровности построения и простоты расчётов.
			PointLocation = FVector(PointLocation.X, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 2 с лагом влево и вниз 45 градусов.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y - Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 3 с лагом вниз 45 градусов.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 3+ для увеличения радиуса поворота.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 4 с лагом вниз и вправо 45 градусов.
			PointLocation = FVector(PointLocation.X - Cathetus, PointLocation.Y + Cathetus, PointLocation.Z);
			RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);
			// Ребро 5 с лагом вправо 45.
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
	// Фигура из 1 ребра. Длина (высота) = 1. Ширина = 0.
	// Угол лага 0 градусов.
	
	if (!(IsValid(RouteSpline))) return InForwardDirection;

	//float Hypotenuse = ForvardStep / LagCosinus;
	//float LongCathetus = ForvardStep;
	//float ShortCathetusCosinus = powf(1 - powf(LagCosinus, 2.f), 1 / 2);
	//float ShortCathetus = Hypotenuse * ShortCathetusCosinus;
	
	EDirection ForwardDirection = InForwardDirection;
	FVector PointLocation = FVector();
	FVector PointTangent = FVector();

	// Координаты начальной точки
	int StartPointIndex = RouteSpline->GetNumberOfSplinePoints() - 1;

	if (StartPointIndex < 0)
	{
		StartPointIndex = 0;
	}

	RouteSpline->GetLocationAndTangentAtSplinePoint(StartPointIndex, PointLocation, PointTangent, ESplineCoordinateSpace::World);

	switch (ForwardDirection)
	{
	case EDirection::Up:
		// 1 Ребро вверх.
		PointLocation = FVector(PointLocation.X + ForvardStep, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);

		// Debug
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("FillerUp has builded.")));
		break;

	case EDirection::Down:
		// 1 Ребро вверх.
		PointLocation = FVector(PointLocation.X - ForvardStep, PointLocation.Y, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);

		// Debug
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("FillerDown has builded.")));
		break;

	case EDirection::Right:
		// 1 Ребро вверх.
		PointLocation = FVector(PointLocation.X, PointLocation.Y + ForvardStep, PointLocation.Z);
		RouteSpline->AddSplinePoint(PointLocation, ESplineCoordinateSpace::World);

		// Debug
		//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("FillerRight has builded.")));
		break;

	case EDirection::Left:
		// 1 Ребро вверх.
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

	// -1 потому что асфальт  кладётся от точки до следующей точки.
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
	// Определяем значения переменных для построения маршрута.
	// Определяем длину шага.
	// Потом подумаю. А пока - количество сегментов в длину в самой длинной секции маршрута (Змейка = 8) + 1 длина шага для разворота.
	EDirection CurrentDirection = InForwardDirection;
	// Вот эту историю в будущем можно на функцию поставить.
	int8 SnakeSectionsForward = 8;
	int8 SnakeSectionsRight = 2;
	int8 TurnSectionsForward = 2;
	int8 TurnSectionsRight = 4;
	int8 FillerSectionForward = 1;
	int8 FillerSectionRight = 0;
	// Возможностоит добавить резерв, то есть дополнительные юниты для сокращения шага... Но зачем, я пока не знаю.
	int8 ReserveForvardSections = 2;
	int8 ReserveRightSections = 0;

	// TODO: Реализовать сглаживание Spline при построении маршрута.

	int32 MaxSectionsComboForward = SnakeSectionsForward + TurnSectionsForward + FillerSectionForward + ReserveForvardSections;
	int32 MaxSectionsComboRight = SnakeSectionsRight + TurnSectionsRight + FillerSectionRight + ReserveForvardSections;
	
	// Тоже потом нужно вынести куда-нибудь.
	float Cosinus0 = 1.f;
	float Cosinus30 = 0.866f;
	float Cosinus45 = 0.7071f;
	// Для финального поворота (Filler) в сторону начала маршрута, чтобы петля не получалась уж слишком крутой.
	// Да похер Сойдёт и с нулём, там всё равно не используются расчёты.
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

	// Упростим алгоритм - если вверх, то направо. Вниз - налево.
	while (!RouteFinished)
	{
		CurrentDistanceToBounds = GetDistanceToBounds(CurrentDirection, InTurnDirection);
		bool UpEnoughth = false;
		bool RightEnoughth = false;

		// Выглядит суперублюдочно, согласен.
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
						// Если построить никакой сегмент не удалось - всё, баста.
						// Закручиваем налево, и кольцуем маршрут.
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
						// Если построить никакой сегмент не удалось - всё, баста.
						// Закручиваем налево, и кольцуем маршрут.
						CurrentDirection = BuildTemplateFiller(EDirection::Left, DeltaForward, Cosinus0);
						GetRouteSpline()->SetClosedLoop(true);
						RouteFinished = true;
					}
				}
			}
			break;
		case EDirection::Right:
			// Здесь потом достроим такую же тему, на случай, если маршрут нужно будет строить по направлению Y.
			// Если время останется.
			break;
		case EDirection::Left:
			// Здесь потом достроим такую же тему, на случай, если маршрут нужно будет строить по направлению Y.
			// Если время останется.
			break;
		default:
			
			break;
		}

	}

	//TODO: Зациклить маршрут.

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
//	// -1 потому что асфальт  кладётся от точки до следующей точки.
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
	// TODO: Проверка входных направлений: Должны быть Up->Right, Left; Right->UpDown и т.д.
	
	// Получаем границы построения маршрута и точку старта.
	GetBoundsForRoute();
	RouteOrigin = GetActorLocation();

	// Очищаем старый маршрут
	RouteSpline->ClearSplinePoints();
	// Удаляем все меши старого маршрута.
	for (int i = 0; i < MeshesOnTheRoad.Num(); i++)
	{
		MeshesOnTheRoad[i]->DestroyComponent();
	}

	//UpdateMesh(0, RouteSpline->GetNumberOfSplinePoints() - 1);

	// Строим маршрут в указанном направлении
	switch (RouteType)
	{
	case ERouteType::Snake:
		BuildRouteSnake(InForwardDirection, InTurnDirection);
		break;
	default:
		break;
	}

}

