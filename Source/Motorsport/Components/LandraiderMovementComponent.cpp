// Fill out your copyright notice in the Description page of Project Settings.


#include "LandraiderMovementComponent.h"

#include "Engine/Engine.h" // Debug strings
#include "Engine/World.h" // GetWorld
#include "GameFramework/Actor.h" // Owner


// Sets default values for this component's properties
ULandraiderMovementComponent::ULandraiderMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void ULandraiderMovementComponent::FullThrottle(float InputIntensity)
{
	// �� ������ ������ �������.
	if (InputIntensity < 0.f)
	{
		InputIntensity *= -1.f;
	}
	
	if (CurrentSpeed == MaxMoveSpeed * InputIntensity) return;

	// ���� ��������� ����� ����������� ��������, �� ������� ������� �������� �� ����, � ����� ��� �����������.
	if (CurrentSpeed < 0.f)
	{
		// ���������� �������� ���������.
		if (SpeedChangeStep < 0.f)
		{
			SpeedChangeStep = 0.f;
		}

		SpeedChangeStep = FMath::Clamp(SpeedChangeStep + BrakingStep, 0.f, MaxAccelerationSpeed);
	}
	else
	{
		SpeedChangeStep = FMath::Clamp(SpeedChangeStep + AccelerationStep, 0.f, MaxAccelerationSpeed);
	}
	
	CurrentSpeed += SpeedChangeStep;
	// ���������� � ����� �������� ����������, ������� ���� ������������ ������ ��� � �������� ������ �������.
	// � �������� ������� - ���� �� ������������, ��������������� ������������� ������� ��������.
	FMath::Clamp(CurrentSpeed, MaxReverseSpeed, MaxMoveSpeed * InputIntensity);
}

void ULandraiderMovementComponent::ReverseGear(float InputIntensity)
{
	// �� ������ ������ �������.
	if (InputIntensity < 0.f)
	{
		InputIntensity *= -1.f;
	}

	// ������������ ������ ��������.
	if (CurrentSpeed == MaxReverseSpeed * InputIntensity) return;

	// ���� ��������� ����� ����������� ��������, �� ������� ������� �������� �� ����, � ����� ��� ����������� �����.
	if (CurrentSpeed > 0.f)
	{
		// ���������� ���������.
		if (SpeedChangeStep > 0.f)
		{
			SpeedChangeStep = 0.f;
		}
		// ��������, ���� �� ���������� ����������� ��������.
		SpeedChangeStep = FMath::Clamp(SpeedChangeStep + BrakingStep, 0.f, MaxAccelerationSpeed);
		CurrentSpeed -= SpeedChangeStep;
	}
	else
	{
		SpeedChangeStep = FMath::Clamp(SpeedChangeStep - AccelerationStep, MaxAccelerationSpeed, 0.f);
		CurrentSpeed += SpeedChangeStep;
	}

	FMath::Clamp(CurrentSpeed, MaxReverseSpeed * InputIntensity, MaxMoveSpeed);
}

void ULandraiderMovementComponent::IdleMoving()
{
	// ���� ������ �� ����, �� � ������.
	if (CurrentSpeed == 0.f) return;

	// ������� �����.
	if (CurrentSpeed < 0.f)
	{
		// �� ���� ������ ������������ �������� �� ������ ���� �������� ������������� ���������, � �� ��������� ����� ��� ����� �������� ��������.
		if (SpeedChangeStep < 0.f)
		{
			SpeedChangeStep = 0.f;
		}
		// "��������" �������� �������� �� ����.
		SpeedChangeStep = FMath::Clamp(SpeedChangeStep + IdleSpeedFadingStep, MaxAccelerationSpeed, 0.f);
		CurrentSpeed += SpeedChangeStep;

		FMath::Clamp(CurrentSpeed, MaxReverseSpeed, 0.f);
	}
	// ������� �����.
	if (CurrentSpeed > 0.f)
	{
		if (SpeedChangeStep > 0.f)
		{
			SpeedChangeStep = 0.f;
		}
		// "��������" �������� �� ����.
		SpeedChangeStep = FMath::Clamp(SpeedChangeStep + IdleSpeedFadingStep, 0.f, MaxAccelerationSpeed);
		CurrentSpeed -= SpeedChangeStep;

		FMath::Clamp(CurrentSpeed, 0.f, MaxMoveSpeed);
	}
}

void ULandraiderMovementComponent::Braking()
{
	// ���� ������ �� ����, �� � ������.
	if (CurrentSpeed == 0.f) return;

	// �������� ������ ��������.
	if (CurrentSpeed < 0.f)
	{
		// ���������� ��������� ������ ��������.
		if (SpeedChangeStep < 0.f)
		{
			SpeedChangeStep = 0.f;
		}
		// �������� "��������� ����������" � ��������.
		SpeedChangeStep = FMath::Clamp(SpeedChangeStep + BrakingStep, 0.f, MaxAccelerationSpeed);
		CurrentSpeed += SpeedChangeStep;

		FMath::Clamp(CurrentSpeed, MaxReverseSpeed, 0.f);
	}
	// �������� �������� ��������.
	if(CurrentSpeed > 0.f)
	{
		// ���������� ��������� ��������.
		if (SpeedChangeStep > 0.f)
		{
			SpeedChangeStep = 0.f;
		}
		// �������� "��������� ����������" � ��������.
		SpeedChangeStep = FMath::Clamp(SpeedChangeStep - BrakingStep, -MaxAccelerationSpeed, 0.f);
		CurrentSpeed -= SpeedChangeStep;

		FMath::Clamp(CurrentSpeed, MaxReverseSpeed, 0.f);
	}
}

void ULandraiderMovementComponent::MoveForward()
{
	FVector CurrentLocation = GetOwner()->GetActorLocation();
	FVector TargetLocation = CurrentLocation + (GetOwner()->GetActorForwardVector() * CurrentSpeed);
	FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, GetWorld()->GetDeltaSeconds(), 1.f);
	GetOwner()->SetActorLocation(NewLocation);
}

void ULandraiderMovementComponent::CalculateTurnRate(float TurnRateInput)
{
	if (CurrentSpeed != 0.f)
	{
		CurrentTurnRate = TurnRateInput * MaxTurnRate * FMath::Clamp(1.f - (abs(CurrentSpeed) / MaxMoveSpeed), 0.1f, 1.f);
		CurrentTurn = FRotator(0, CurrentTurnRate, 0);
	}
	else
	{
		CurrentTurn = FRotator(0.f, 0.f, 0.f);
	}

	//Debug
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Cyan, FString::Printf(TEXT("MovementComponent: Currnt turn: %s"), *CurrentTurn.ToString()));
}

void ULandraiderMovementComponent::Turn()
{
	FRotator Rotation = GetOwner()->GetActorRotation();
	FRotator SetTurn = FMath::RInterpTo(Rotation, Rotation + CurrentTurn, GetWorld()->GetDeltaSeconds(), 1.f);
	GetOwner()->SetActorRotation(SetTurn);
}

void ULandraiderMovementComponent::Skidding()
{

}

// Called when the game starts
void ULandraiderMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

float ULandraiderMovementComponent::GetCurrentSpeed() const
{
	return CurrentSpeed;
}

float ULandraiderMovementComponent::GetMaxSpeed() const
{
	return MaxMoveSpeed;
}

float ULandraiderMovementComponent::GetMaxReverseSpeed() const
{
	return MaxReverseSpeed;
}

float ULandraiderMovementComponent::GetMaxTurnRate() const
{
	return MaxTurnRate;
}

float ULandraiderMovementComponent::SetDriveAction(EDriveAction Action, float InputIntensity)
{
	// �������� �������� ��������.
	float ThrottleInput = FMath::Clamp(InputIntensity, -1.f, 1.f);
	CurrentDriveAction = Action;

	switch (CurrentDriveAction)
	{
	case EDriveAction::FullThrottle:
		FullThrottle(ThrottleInput);
		break;
	
	case EDriveAction::ReverseGear:
		ReverseGear(ThrottleInput);
		break;
	
	case EDriveAction::IdleMoving:
		IdleMoving();
		break;
	
	case EDriveAction::Braking:
		Braking();
		break;
	
	default:
		break;
	}

	MoveForward();

	return CurrentSpeed;
}

FRotator ULandraiderMovementComponent::Steering(float TurnRateInput)
{
	CalculateTurnRate(TurnRateInput);
	Turn();

	return CurrentTurn;
}

// Called every frame
void ULandraiderMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

