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

	//FQuat CurrentTurn;
	EDriveAction CurrentDriveAction = EDriveAction::IdleMoving;
	FRotator CurrentTurn;

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

	// Reering Behaviour
	void CalculateTurnRate(float TurnRateInput);

	// TODO: Реализовать занос. Если останется время.
	void Skidding();

protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxMoveSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxReverseSpeed = -300.0f;

	// Максимальная скорость поворота, она же угол.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxTurnRate = 180.0f;
	
	// Время разгона
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AccelerationStep = 0.2f;
	
	// Максимальное ускорение
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxAccelerationSpeed = 2.f;

	//Время остановки
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float IdleSpeedFadingStep = 0.1f;

	//Время торможения
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float BrakingStep = 0.4f;

	
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	
	float GetCurrentSpeed() const;
	float GetMaxSpeed() const;
	float GetMaxReverseSpeed() const;
	float GetMaxTurnRate() const;
	
	UFUNCTION(BlueprintCallable)
	void MoveForward();
	
	UFUNCTION(BlueprintCallable)
	void Turn();
	
	// Газ-Тормоз
	UFUNCTION(BlueprintCallable)
	float SetDriveAction(EDriveAction Action, float InputIntensity);
	// Поворот
	UFUNCTION(BlueprintCallable)
	FRotator Steering(float TurnRateInput);

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
