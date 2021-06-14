// Fill out your copyright notice in the Description page of Project Settings.


#include "LandraiderMovementComponent.h"

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
		// �������� ������������ ����� ������������� �������� �������������� ������� "��������� ����������" ����������� ������� "��������� ��������".
		if (CurrentSpeed <= SpeedChangeStep && SpeedChangeStep != AccelerationStep)
		{
			SpeedChangeStep = 0.f;
		}
		
		SpeedChangeStep = FMath::Clamp(SpeedChangeStep + AccelerationStep, 0.f, MaxAccelerationSpeed);
	}
	
	CurrentSpeed = (CurrentSpeed + SpeedChangeStep) * GetWorld()->GetDeltaSeconds();
	// ���������� � ����� �������� ����������, ������� ���� ������������ ������ ��� � �������� ������ �������.
	// � �������� ������� - ���� �� ������������, ��������������� ������������� ������� ��������.
	FMath::Clamp(CurrentSpeed, MaxReverseSpeed, MaxMoveSpeed * InputIntensity);
}

void ULandraiderMovementComponent::ReverseGear(float InputIntensity)
{
	// ������������ ������ ��������.
	if (CurrentSpeed == MaxReverseSpeed * InputIntensity) return;
	
	// �� ������ ������ �������.
	if (InputIntensity < 0.f)
	{
		InputIntensity *= -1.f;
	}
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

		CurrentSpeed = (CurrentSpeed - SpeedChangeStep) * GetWorld()->GetDeltaSeconds();
	}
	else
	{
		// �������� ������������ ����� ������������� �������� �������������� ������� "��������� ����������" ����������� ������� "��������� ��������".
		if (CurrentSpeed >= -SpeedChangeStep && SpeedChangeStep != -AccelerationStep)
		{
			SpeedChangeStep = 0.f;
		}

		SpeedChangeStep = FMath::Clamp(SpeedChangeStep - AccelerationStep, MaxAccelerationSpeed, 0.f);
		CurrentSpeed = (CurrentSpeed + SpeedChangeStep) * GetWorld()->GetDeltaSeconds();
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
		CurrentSpeed = (CurrentSpeed + SpeedChangeStep) * GetWorld()->GetDeltaSeconds();
		
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
		CurrentSpeed = (CurrentSpeed - SpeedChangeStep) * GetWorld()->GetDeltaSeconds();

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
		CurrentSpeed = (CurrentSpeed + SpeedChangeStep) * GetWorld()->GetDeltaSeconds();
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
		CurrentSpeed = (CurrentSpeed + SpeedChangeStep) * GetWorld()->GetDeltaSeconds();
		FMath::Clamp(CurrentSpeed, MaxReverseSpeed, 0.f);
	}
}



void ULandraiderMovementComponent::MoveForward()
{
	GetOwner()->AddActorLocalOffset(FVector(CurrentSpeed, 0.f, 0.f), true);
}

void ULandraiderMovementComponent::CalculateTurnRate(float TurnRateInput)
{
	// ��� ���� �������� - ��� ������� ���������.
	
	
	//CurrentTurnRate = MaxTurnSpeed * 
}

void ULandraiderMovementComponent::Turn()
{
	GetOwner()->AddActorLocalRotation(CurrentTurn, true);
}

void ULandraiderMovementComponent::Skidding()
{

}

// Called when the game starts
void ULandraiderMovementComponent::BeginPlay()
{
	Super::BeginPlay();
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
	
	case EDriveAction::IdleMoveing:
		IdleMoving();
		break;
	
	case EDriveAction::Braking:
		Braking();
		break;
	
	default:
		break;
	}

	return CurrentSpeed;
}

FQuat ULandraiderMovementComponent::Steering(float TurnRateInput)
{
	//float RotateAmount = Value * RotateSpeed * GetWorld()->DeltaTimeSeconds;
	//FRotator Rotation = FRotator(0, RotateAmount, 0);
	//RotationDirection = FQuat(Rotation);
	if (CurrentSpeed != 0.f)
	{
		CurrentTurnRate = MaxTurnRate * FMath::Clamp(1.f - (abs(CurrentSpeed) / MaxMoveSpeed), 0.1f, 1.f);
		FRotator Rotation = FRotator(0, CurrentTurnRate, 0);
		CurrentTurn = FQuat(Rotation);
	}
	else
	{
		CurrentTurn = FQuat(FVector(0.f, 0.f, 0.f), 0.f);
	}

	return CurrentTurn;
}

// Called every frame
void ULandraiderMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

