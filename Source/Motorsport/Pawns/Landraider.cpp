// Fill out your copyright notice in the Description page of Project Settings.


#include "Landraider.h"
#include "Components/LandraiderMovementComponent.h"

#include "Components/StaticMeshComponent.h"
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
	// Ќужно подн€ть меш, а то он будет при спавне "заземл€тьс€" и не будет спавнитьс€. ѕока что в Blueprint.
	Mesh->SetSimulatePhysics(true);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECollisionResponse::ECR_Block);
	Mesh->GetBodyInstance()->SetMassOverride(Mass);
	Mesh->GetBodyInstance()->UpdateMassProperties();

	MovementComponent = CreateDefaultSubobject<ULandraiderMovementComponent>(TEXT("MovementComponent"));

}

// Called when the game starts or when spawned
void ALandraider::BeginPlay()
{
	Super::BeginPlay();
	
}

ULandraiderMovementComponent* ALandraider::GetMovementComponent()
{
	return MovementComponent;
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

