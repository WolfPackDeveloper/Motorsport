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

// ������ ����� �����������.
USTRUCT()
struct FThreatInstigator
{
	GENERATED_BODY()
	//FHitResult HitObstacle; // �� ���� �� ����������. � ���, ����� ���������� � ������ ���������� �� AActor.
	AActor* Obstacle;
	float ThreatValue;
};

// ������ ����� �����
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

	// ��������� AI.
	EMachineSpiritState SpiritState = EMachineSpiritState::Sleeps;
	// ����������� ��������.
	//ESteerDirection SteerDirection = ESteerDirection::Forward;

	UPROPERTY()
	AMotorsportGameModeBase* GameMode = nullptr;

	UPROPERTY()
	ALandraider* OwnedLandraider = nullptr;

	UPROPERTY()
	ARouteActor* CurrentRoute = nullptr;

	UPROPERTY()
	int32 CurrentRoutePointIndex = 0;
	
	// �������� ����� ������� �������� � ����������� �� �������� �����������, ��� ����.
	UPROPERTY()
	int32 SpeedSteps = 5;

	// �������� ������.
	UPROPERTY()
	FVector PawnDimensions;
	
	// ������� Ground
	UPROPERTY()
	FVector GroundBounds;

	// ��������� "������" AI.
	UPROPERTY()
	float TraceLength = 2000.f; // TODO: ����� ������� ����������� ����������, � ����������� �� ��������.
	// ����, ������ � ������� �������� �� �����, ����� ��������� �������� �� �����.
	UPROPERTY()
	float PointReachField = 1000.f;

	UPROPERTY()
	float PointReachAngle = 90.f;

	// ����� "�����" �����������: ��, ��, �, ��, ��.
	UPROPERTY()
	TArray<FThreatInstigator> ThreatMap;
	
	// ����� "�����" ����������� (��, ��, �, ��, ��), ������ � ��������.
	UPROPERTY()
	TMap<FName, float> WeightMap;
	
	// ����� "�����" �����������: ��, �, ��
	UPROPERTY()
	TArray<FDirectionWeight> DirectionWeightMap;

	// ========== FUNCTIONS ============

	// ������
	UFUNCTION()
	FVector GetOwnedPawnDimensions();
	
	// "������" �������.
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

	// ����������� � ����������� �����������.
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
	
	//TODO: ���������� ���� �������, ���� ���������� �� ����� < TraceLength. ����� ��������� ������������� ������� � ������� ����� � �������.
	// ����� ����� ���������� � �������� "����  �������" �� �����.
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
