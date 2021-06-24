// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MachineSpirit.generated.h"


class ALandraider;
class AMotorsportGameModeBase;
class ARouteActor;

class AStaticMeshActor;

UENUM()
enum class EMachineSpiritState : uint8
{
	Awake,
	Sleeps
};

UENUM()
enum class ESteerDirection : uint8
{
	Left,
	Forward,
	Right
};

// Объект карты препятствий.
USTRUCT()
struct FThreatInstigator
{
	GENERATED_BODY()
	//FHitResult HitObstacle; // Ну мало ли пригодится. А так, можно отделаться и просто указателем на AActor.
	AActor* Obstacle;
	float ThreatValue;
};

// Объект карты весов
USTRUCT()
struct FDirectionWeight
{
	GENERATED_BODY()
	
	ESteerDirection Direction;
	float Weight;
};

UCLASS()
class MOTORSPORT_API AMachineSpirit : public AAIController
{
	GENERATED_BODY()
	
public:

	AMachineSpirit();

private:

	// Состояние AI.
	EMachineSpiritState SpiritState = EMachineSpiritState::Sleeps;
	// Направление поворота.
	//ESteerDirection SteerDirection = ESteerDirection::Forward;

	UPROPERTY()
	AMotorsportGameModeBase* GameMode = nullptr;

	UPROPERTY()
	ALandraider* OwnedLandraider = nullptr;

	UPROPERTY()
	ARouteActor* CurrentRoute = nullptr;

	UPROPERTY()
	int32 CurrentRoutePointIndex = 0;
	
	// Условная шкала деления скорости в зависимости от близости препятствия, или цели.
	UPROPERTY()
	int32 SpeedSteps = 5;

	// Габариты машины.
	UPROPERTY()
	FVector PawnDimensions;
	
	// Границы Ground
	UPROPERTY()
	FVector GroundBounds;

	// Дальность "обзора" AI.
	UPROPERTY()
	float TraceLength = 2000.f; // TODO: Можно сделать динамически изменяемой, в зависимости от скорости.
	// Поле, проезд в радиусе которого от точки, будет считаться проездом по точке.
	UPROPERTY()
	float PointReachField = 1000.f;

	UPROPERTY()
	float PointReachAngle = 90.f;

	// Карты "угроз" препятствий: Лв, ЛП, П, ПП, Пр.
	UPROPERTY()
	TArray<FThreatInstigator> ThreatMap;
	
	// Карта "весов" препятствий (Лв, ЛП, П, ПП, Пр), границ и маршрута.
	UPROPERTY()
	TMap<FName, float> WeightMap;
	
	// Карта "весов" направлений: Лв, П, Пр
	UPROPERTY()
	TArray<FDirectionWeight> DirectionWeightMap;

	// ========== FUNCTIONS ============

	// Разное
	UFUNCTION()
	FVector GetOwnedPawnDimensions();
	
	// "Радиус" границы.
	UFUNCTION()
	void DefineGroundBounds();

	// Nodes Movement
	UFUNCTION()
	void Throttle(float InputIntensity);
	
	UFUNCTION()
	void ReverseGear(float InputIntensity);
	
	UFUNCTION()
	void Brake(float InputIntensity);
	
	UFUNCTION()
	void IdleMoving(float InputIntensity);
	
	UFUNCTION()
	void TurnRight(float InputIntensity);

	// Трассировка и определение направления.
	UFUNCTION()
	void TraceLeft();
	
	UFUNCTION()
	void TraceForwardLeft();
	
	UFUNCTION()
	void TraceForward();
	
	UFUNCTION()
	void TraceForwardRight();
	
	UFUNCTION()
	void TraceRight();
	
	UFUNCTION()
	void FillThreatMap();

	UFUNCTION()
	int32 DefineCurrentRoutePoint();
	
	UFUNCTION()
	float DefineRoutePointWeight();
	
	//TODO: Уменьшение веса границы, если расстояние до точки < TraceLength. Чтобы корректно отрабатывался поворот в сторону точки у границы.
	// Также можно поиграться с размером "поля  проезда" по точке.
	UFUNCTION()
	float DefineGroundBoundsWeight();
	
	UFUNCTION()
	void FillWeightMap();

	UFUNCTION()
	ESteerDirection DefineSteerDirection();
	
	UFUNCTION()
	float DefineThrottle(ESteerDirection CurrentDirection);

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

public:

	UFUNCTION(BlueprintCallable)
	EMachineSpiritState GetMachineSpiritState();

	UFUNCTION(BlueprintCallable)
	void SetMachineSpiritState(EMachineSpiritState State);

	UFUNCTION(BlueprintCallable)
	void DefineSteerBehaviour();

	virtual void Tick(float DeltaTime) override;
};
