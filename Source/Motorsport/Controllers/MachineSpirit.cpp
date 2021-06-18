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
//	// "Трассировка" границы
//	TraceGroundBounds();
//
//	// Определить вес "текущей точки маршрута" приближении к границам.
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
	float Distance = 0; // Расстояние до точки столкновения луча с препятствием. Для расчёта уровня угрозы.
	FVector TraceStart;
	FVector TraceEnd;

	float X = OwnedLandraider->GetActorForwardVector().X;
	float Y = OwnedLandraider->GetActorForwardVector().Y - ((1 / 2) * PawnDimensions.Y);
	float Z = OwnedLandraider->GetActorForwardVector().Z;
	TraceStart = FVector(X, Y, Z);
	// Можно, конечно, посчитать длину по X... Но морока с углом...
	// Наверняка есть какая-нибудь хитрость с кватернионами, но, селяви.
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
	float Distance = 0; // Расстояние до точки столкновения луча с препятствием. Для расчёта уровня угрозы.
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
	float Distance = 0; // Расстояние до точки столкновения луча с препятствием. Для расчёта уровня угрозы.
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
	float Distance = 0; // Расстояние до точки столкновения луча с препятствием. Для расчёта уровня угрозы.
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
	float Distance = 0; // Расстояние до точки столкновения луча с препятствием. Для расчёта уровня угрозы.
	FVector TraceStart;
	FVector TraceEnd;

	float X = OwnedLandraider->GetActorForwardVector().X;
	float Y = OwnedLandraider->GetActorForwardVector().Y + ((1 / 2) * PawnDimensions.Y);
	float Z = OwnedLandraider->GetActorForwardVector().Z;
	TraceStart = FVector(X, Y, Z);
	// Можно, конечно, посчитать длину по X... Но морока с углом...
	// Наверняка есть какая-нибудь хитрость с кватернионами, но, селяви.
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
	// Ну не запихивать же в каждую функцию эти две проверки...
	if (!IsValid(OwnedLandraider)) return;
	// Можно запихнуть в какую-нибудь функцию зануления или инициализации правильной размерности.
	if (ThreatMap.Num() != 5)
	{
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
	if (!IsValid(CurrentRoute) || !IsValid(OwnedLandraider)) return -1;
	
	if (CurrentRoutePointIndex < 0 || CurrentRoutePointIndex >= CurrentRoute->GetRouteSpline()->GetNumberOfSplinePoints())
	{
		return -1;
	}

	// Опасно. Это единственное место, где определяется точка маршрута.
	FVector Distance = CurrentRoute->GetRouteSpline()->GetWorldLocationAtSplinePoint(CurrentRoutePointIndex) - OwnedLandraider->GetActorForwardVector();
	// Это гениально! И не нужны никакие углы поворота!
	bool bCurrentPointIsReached = (abs(Distance.X) < PointReachField) || (abs(Distance.Y) < PointReachField);
	
	if (bCurrentPointIsReached)
	{
		++CurrentRoutePointIndex;
	}

	return CurrentRoutePointIndex;
}

float AMachineSpirit::DefineRoutePointWeight()
{
	// Определяем положение активной точки маршрута и расстояние до неё.
	//DefineCurrentRoutePoint();

	if (DefineCurrentRoutePoint() < 0)
	{
		return 0.f;
	}

	float Weight = 0.f;

	// Определяем расстояние до точки: если больше вектора трассировки то значение -0.1-0-0.1, в зависимости от направления.
	// Чисто обозначить направление
	FVector RouteLocation = CurrentRoute->GetRouteSpline()->GetWorldLocationAtSplinePoint(CurrentRoutePointIndex);
	FVector CurrentLocation = OwnedLandraider->GetActorForwardVector();
	FVector DeltaLocation = RouteLocation - CurrentLocation;
	float Distance = sqrtf(powf(DeltaLocation.X, 2.f) + powf(DeltaLocation.Y, 2.f));

	// Если едем прямо на точку, то продолжаем ехать.
	if (DeltaLocation.Y == 0.f) return 0.f;
	// Если нет, то смотрим куда повернуть.
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
	// Определяем "лучи" трассировки.
	FVector Forward = OwnedLandraider->GetActorForwardVector();
	Forward.X += TraceLength;
	FVector Left = OwnedLandraider->GetActorForwardVector();
	Left.X += (TraceLength - PawnDimensions.X);
	Left.Y -= PawnDimensions.Y;
	FVector Right = OwnedLandraider->GetActorForwardVector();
	Right.X += (TraceLength - PawnDimensions.X);
	Right.Y += PawnDimensions.Y;
	// Вес находится в промежутке от -1 до 1. Знак определяет направление поворота: лево-право.
	float Weight = 0.f;

	// Если спереди граница - поворачиваем направо.
	if (Forward.X >= GroundBounds.X || Forward.X <= -GroundBounds.X)
	{
		// Если справа тоже граница - поворачиваем налево.
		Weight = (abs(Forward.X) - abs(GroundBounds.X)) / TraceLength;
		if (Right.X >= GroundBounds.X || Right.X <= -GroundBounds.X)
		{
			Weight *= -1.f;
		}
	}
	// Здесь можно поиграться, чтобы не проводилась проверка с случае "диагонального" выхода за границы.
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
	// Карта "весов" направлений: Лв(0), П(1), Пр(2).
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
	// Сюда прибавляем значение веса точки маршрута и границы (если они отрицательный - то понятно, да?)
	DirectionWeightMap[2] = (1 - ThreatMap[2].ThreatValue) + (1 - ThreatMap[3].ThreatValue) + (1 - ThreatMap[4].ThreatValue) + DefineRoutePointWeight() + DefineGroundBoundsWeight();
}

void AMachineSpirit::DefineSteerAction()
{
	CalculateThreatAmount();
	CalculateDirectionsWeight();

	if (ThreatMap.Num() < 5 || DirectionWeightMap.Num() < 3) return;

	//if(DirectionWeightMap[1] == 3)

	// Если путь пролегает прямо - тапку в пол.
	// Вопрос - как посчитать "средний" вес, и не проебаться с поворотом - ведь в 5 измерениях он может быть сильно разный...
	// Надо думать... Но уже завтра. Сегодня больше не думается от слова "совсем".
	// Если "Поворот" с весом < 25% (вес больше в средне 0.25) - поворачиваем на полшишечки.
	// Если вес поворота 25% < 50% (вес больше в средне 0.5) - начинаем по полной и немножко тормозим.
	// Если вес поворота 50% < 75% (вес больше в средне 0.75) - поворачиваем со всей силы на минимальной скорости.

	// Тут начинается история с задним ходом, которую нужно будет доделать когда-нибудь.
	// Если вес > 66% (70-75) - врубаем заднюю и крутим руль в другую сторону, когда начинаем ехать назад.
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