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
	// На случай ошибки вектора.
	if (InputIntensity < 0.f)
	{
		InputIntensity *= -1.f;
	}
	
	if (CurrentSpeed == MaxMoveSpeed * InputIntensity) return;

	// Если произошла смена направления движения, то сначала снижаем скорость до нуля, а потом уже разгоняемся.
	if (CurrentSpeed < 0.f)
	{
		// Сбрасываем обратное ускорение.
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
	// Неизвестно с какой скорости начиналось, поэтому берём максимальный задний ход в качестве нижней границы.
	// В качестве верхней - долю от максимальной, соответствующую интенсивности нажатия педальки.
	FMath::Clamp(CurrentSpeed, MaxReverseSpeed, MaxMoveSpeed * InputIntensity);
}

void ULandraiderMovementComponent::ReverseGear(float InputIntensity)
{
	// На случай ошибки вектора.
	if (InputIntensity < 0.f)
	{
		InputIntensity *= -1.f;
	}

	// Поддерживаем нужную скорость.
	if (CurrentSpeed == MaxReverseSpeed * InputIntensity) return;

	// Если произошла смена направления движения, то сначала снижаем скорость до нуля, а потом уже разгоняемся назад.
	if (CurrentSpeed > 0.f)
	{
		// Сбрасываем ускорение.
		if (SpeedChangeStep > 0.f)
		{
			SpeedChangeStep = 0.f;
		}
		// Тормозим, пока не поменяется направление движения.
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
	// Если никуда не едем, то и хорошо.
	if (CurrentSpeed == 0.f) return;

	// Катимся назад.
	if (CurrentSpeed < 0.f)
	{
		// По идее данная формулировка позволит на первом тике занулить отрицательное ускорение, а на следующем будет уже расти снижение скорости.
		if (SpeedChangeStep < 0.f)
		{
			SpeedChangeStep = 0.f;
		}
		// "Повышаем" обратную скорость до нуля.
		SpeedChangeStep = FMath::Clamp(SpeedChangeStep + IdleSpeedFadingStep, MaxAccelerationSpeed, 0.f);
		CurrentSpeed += SpeedChangeStep;

		FMath::Clamp(CurrentSpeed, MaxReverseSpeed, 0.f);
	}
	// Катимся вперёд.
	if (CurrentSpeed > 0.f)
	{
		if (SpeedChangeStep > 0.f)
		{
			SpeedChangeStep = 0.f;
		}
		// "Понижаем" скорость до нуля.
		SpeedChangeStep = FMath::Clamp(SpeedChangeStep + IdleSpeedFadingStep, 0.f, MaxAccelerationSpeed);
		CurrentSpeed -= SpeedChangeStep;

		FMath::Clamp(CurrentSpeed, 0.f, MaxMoveSpeed);
	}
}

void ULandraiderMovementComponent::Braking()
{
	// Если никуда не едем, то и хорошо.
	if (CurrentSpeed == 0.f) return;

	// Тормозим заднюю скорость.
	if (CurrentSpeed < 0.f)
	{
		// Сбрасываем ускорение задней скорости.
		if (SpeedChangeStep < 0.f)
		{
			SpeedChangeStep = 0.f;
		}
		// Набираем "ускорение ториожения" и тормозим.
		SpeedChangeStep = FMath::Clamp(SpeedChangeStep + BrakingStep, 0.f, MaxAccelerationSpeed);
		CurrentSpeed += SpeedChangeStep;

		FMath::Clamp(CurrentSpeed, MaxReverseSpeed, 0.f);
	}
	// Тормозим переднюю скорость.
	if(CurrentSpeed > 0.f)
	{
		// Сбрасываем ускорение движения.
		if (SpeedChangeStep > 0.f)
		{
			SpeedChangeStep = 0.f;
		}
		// Набираем "ускорение ториожения" и тормозим.
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
	// Конечная скорость движения.
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

