// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LandraiderMovementComponent.generated.h"


UENUM()
enum class EDriveAction : uint8
{
	FullThrottle,
	ReverseGear,
	IdleMoving,
	Braking
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MOTORSPORT_API ULandraiderMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULandraiderMovementComponent();

private:

	FQuat CurrentTurn;
	EDriveAction CurrentDriveAction = EDriveAction::IdleMoving;

	float CurrentSpeed = 0.f;
	float CurrentTurnSpeed = 0.f;
	float SpeedChangeStep = 0.f;
	float CurrentTurnRate = 0.f;

	bool bFullThrottle = false;
	bool bIsBraking = false;

	// Move Behaviour
	void FullThrottle(float InputIntensity);
	void ReverseGear(float InputIntensity);
	void IdleMoving();
	void Braking();

	void MoveForward();
	
	// Reering Behaviour
	void CalculateTurnRate(float TurnRateInput);
	void Turn();

	// TODO: ����������� �����. ���� ��������� �����.
	void Skidding();

protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxMoveSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxReverseSpeed = -100.0f;
	
	// ������������ �������� ��������, ��� �� ����.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxTurnRate = 100.0f;

	// ����� �������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AccelerationStep = 0.02f;
	
	// ������������ ���������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxAccelerationSpeed = 2.f;

	//����� ���������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float IdleSpeedFadingStep = 0.01f;

	//����� ����������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BrakingStep = 0.04f;

	
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	
	// ���-������
	UFUNCTION(BlueprintCallable)
	float SetDriveAction(EDriveAction Action, float InputIntensity);
		
	// �������
	UFUNCTION(BlueprintCallable)
	FQuat Steering(float TurnRateInput);

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
