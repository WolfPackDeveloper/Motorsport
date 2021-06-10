// Fill out your copyright notice in the Description page of Project Settings.


#include "Landraider.h"

#include "Components/StaticMeshComponent.h"

// Sets default values
ALandraider::ALandraider()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
}

// Called when the game starts or when spawned
void ALandraider::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALandraider::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ALandraider::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

