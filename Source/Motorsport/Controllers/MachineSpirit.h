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
	//FHitResult HitObstacle; // �� ���� �� ����������. � ���, ����� ���������� � ������ ���������� �� AActor.
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

	// �������� ������.
	FVector PawnDimensions;
	// ������� Ground
	FVector GroundBounds;

	float TraceLength = 500.f; // TODO: ����� ������� ����������� ����������, � ����������� �� ��������.
	// ����, ������ � ������� �������� �� �����, ����� ��������� �������� �� �����.
	float PointReachField = 50.f;
	// ��� ��������� � ���������� �� -0.5 �� 0.5. ���� ���������� ����������� ��������: ����-�����.
//	float CurrentRoutePointWeight = 0.f;
	// ��� ��������� � ���������� �� -1 �� 1. ���� ���������� ����������� ��������: ����-�����.
//	float GroundBoundsWeight = 0.f;

	// ����� "���������" �����������: ��, ��, �, ��, ��.
	TArray<FThreatInstigator> ThreatMap;
	// ����� "�����" �����������: ��, �, ��
	TArray<float> DirectionWeightMap;

	// ========== FUNCTIONS ============

	// ������
	FVector GetOwnedPawnDimensions();
	// "������" �������.
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

	// ����������� � ����������� �����������.
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
