// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MachineSpirit.generated.h"


class ALandraider;
class AMotorsportGameModeBase;
class ARouteActor;

class AStaticMeshActor;

USTRUCT()
struct FThreatInstigator
{
	//FHitResult HitObstacle; // Ну мало ли пригодится. А так, можно отделаться и просто указателем на AActor.
	AActor* Obstacle;
	float ThreatValue;
};

UENUM()
enum class EMachineSpiritState : uint8
{
	Awake,
	Sleeps
};

//UENUM()
//enum class ESteerBehaviour : uint8
//{
//	Wait,
//	Explore,
//	FollowTheRoute
//};

UCLASS()
class MOTORSPORT_API AMachineSpirit : public AAIController
{
	GENERATED_BODY()
	
public:

	AMachineSpirit();

private:

	EMachineSpiritState SpiritState = EMachineSpiritState::Sleeps;
	//ESteerBehaviour SteerBehaviour = ESteerBehaviour::Wait;

	UPROPERTY()
	AMotorsportGameModeBase* GameMode = nullptr;

	UPROPERTY()
	ALandraider* OwnedLandraider = nullptr;

	UPROPERTY()
	ARouteActor* CurrentRoute = nullptr;

	int32 CurrentRoutePointIndex = 0;

	// Габариты машины.
	FVector PawnDimensions;
	// Границы Ground
	FVector GroundBounds;

	float TraceLength = 500.f; // TODO: Можно сделать динамически изменяемой, в зависимости от скорости.
	// Поле, проезд в радиусе которого от точки, будет считаться проездом по точке.
	float PointReachField = 50.f;
	// Вес находится в промежутке от -0.5 до 0.5. Знак определяем направление поворота: лево-право.
//	float CurrentRoutePointWeight = 0.f;
	// Вес находится в промежутке от -1 до 1. Знак определяет направление поворота: лево-право.
//	float GroundBoundsWeight = 0.f;

	// Карты "опасности" препятствий: Лв, ЛП, П, ПП, Пр.
	TArray<FThreatInstigator> ThreatMap;
	// Карта "весов" направлений: Лв, П, Пр
	TArray<float> DirectionWeightMap;

	// ========== FUNCTIONS ============

	// Разное
	FVector GetOwnedPawnDimensions();
	// "Радиус" границы.
	void DefineGroundBounds();

	// Nodes Movement
	void Throttle(float InputIntensity);
	void ReverseGear(float InputIntensity);
	void Brake(float InputIntensity);
	void IdleMoving(float InputIntensity);
	void TurnRight(float InputIntensity);

	// Nodes Route Following
//	void BuildRoute();

//	void ExploreSteerBehaviour();
//	void RouteFollowingSteerBehaviour();
	void DefineSteerAction();

	// Трассировка и определение направления.
	void TraceLeft();
	void TraceForwardLeft();
	void TraceForward();
	void TraceForwardRight();
	void TraceRight();
	void CalculateThreatAmount();

	int32 DefineCurrentRoutePoint();
	float DefineRoutePointWeight();
	float DefineGroundBoundsWeight();
	void CalculateDirectionsWeight();

	void ObstacleTracing();

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintCallable)
	EMachineSpiritState GetMachineSpiritState();

	UFUNCTION(BlueprintCallable)
	void SetMachineSpiritState(EMachineSpiritState State);

	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
