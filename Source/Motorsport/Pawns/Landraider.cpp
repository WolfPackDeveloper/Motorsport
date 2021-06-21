// Fill out your copyright notice in the Description page of Project Settings.


#include "Landraider.h"
#include "Components/LandraiderMovementComponent.h"
#include "Controllers/MachineSpirit.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h" // Debug strings
#include "Engine/StaticMesh.h"

// Sets default values
ALandraider::ALandraider()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);

	MovementComponent = CreateDefaultSubobject<ULandraiderMovementComponent>(TEXT("MovementComponent"));

}

// Called when the game starts or when spawned
void ALandraider::BeginPlay()
{
	Super::BeginPlay();
	
}

ULandraiderMovementComponent* ALandraider::GetLandraiderMovementComponent()
{
	return MovementComponent;
}

// Called every frame
void ALandraider::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Debug - координаты на карте.
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Green, FString::Printf(TEXT("Landraider: Current Location: %s"), *GetActorLocation().ToString()));
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Yellow, FString::Printf(TEXT("Landraider: Current Rotation: %s"), *GetActorRotation().ToString()));
}

// Called to bind functionality to input
void ALandraider::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

