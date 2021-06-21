// Fill out your copyright notice in the Description page of Project Settings.


#include "MotorsportGameModeBase.h"
#include "Actors/RouteActor.h"
#include "Pawns/Landraider.h"
#include "Controllers/MachineSpirit.h"

#include "Engine/StaticMesh.h" // Ну хз... поможет ли?
#include "Engine/StaticMeshActor.h" // Find Ground - lazy method
#include "Kismet/GameplayStatics.h" // Find Ground
#include "Runtime/Renderer/Public/MeshDrawShaderBindings.h" // Strange Error - identifier "FMeshDrawSingleShaderBindings" is undefined


//#include "ConstructorHelpers.h" // Random Meshes

AMotorsportGameModeBase::AMotorsportGameModeBase()
{
	// To get proper path to asset go to editor and right click the asset you want and click "Copy Refrence", it will copy asset path in to clipboard so you can paste it anywhere.
	AddMeshForObstacles(TEXT("StaticMesh'/Game/Meshes/Cone.Cone'"));
	AddMeshForObstacles(TEXT("StaticMesh'/Game/Meshes/Cube.Cube'"));
	AddMeshForObstacles(TEXT("StaticMesh'/Game/Meshes/Cylinder.Cylinder'"));
	AddMeshForObstacles(TEXT("StaticMesh'/Game/Meshes/Sphere.Sphere'"));

	GameStatus = EGameStatus::GameStopped;
}

void AMotorsportGameModeBase::AddMeshForObstacles(TCHAR* ObjectFQName)
{
	//v.1.0 - Not Working
	//auto MeshAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(FObjectFinderParam);

	//ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(FObjectFinderParam);

	//if (MeshAsset.Object != nullptr)
	//{
	//	ObstacleMeshReferences.Add(MeshAsset);
	//	ObstacleMeshes.Add(ObstacleMeshReferences[ObstacleMeshReferences.Num() - 1].Object);
	//}

	//v.2.0 - Not Working
	//if (Mesh != nullptr)
	//{
	//	ObstacleMeshes.Add(Mesh);
	//}
	
	//v.3.0 - ITS ALIVE!!!
	UStaticMesh* ObstacleMesh = LoadObject<UStaticMesh>(NULL, ObjectFQName, NULL, LOAD_None, NULL);

	if (ObstacleMesh)
	{
		ObstacleMeshes.Add(ObstacleMesh);
	}
}

void AMotorsportGameModeBase::DefineGroundActor(FName GroundActorTag)
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), FoundActors);

	for (auto Actor : FoundActors)
	{
		if (Actor->ActorHasTag(GroundActorTag))
		{
			GroundActor = Actor;
			return;
		}
	}
}

FVector AMotorsportGameModeBase::DefineAccessBounds(AActor* Ground, float Margin)
{
	if (!IsValid(Ground)) return FVector(0.f, 0.f, 0.f);

	FVector BoundOrigin;
	FVector BoundExtent;

	Ground->GetActorBounds(true, BoundOrigin, BoundExtent);

	BoundExtent.X -= Margin;
	BoundExtent.Y -= Margin;

	return FVector(BoundExtent.X, BoundExtent.Y, BoundExtent.Z);
}

void AMotorsportGameModeBase::BeginPlay()
{	
	Super::BeginPlay();

	DefineGroundActor(TEXT("Ground"));
	GroundAccessBounds = DefineAccessBounds(GroundActor, BoundMargin);
	
	float SpawnMargin = 300.f;
	// Или маяться с ActorSpawnParams.SpawnCollisionHandlingOverride, или Z + 1
	// Это в случае, если меш поднят целиком над нулём! Иначе нужно вылавливать: Z = Scale.Z / 2 * 100 + 1
	FVector Location = FVector(SpawnMargin - GroundAccessBounds.X, SpawnMargin - GroundAccessBounds.Y, GroundAccessBounds.Z + 1.f);
	FRotator Rotation = FRotator(0.f, 0.f, 0.f);

	// Десантируем дроппод с Лэндрейдером.
	if (!IsValid(Landraider))
	{
		Landraider = GetWorld()->SpawnActor<ALandraider>(MachineClass, Location, Rotation);
		Landraider->SpawnDefaultController();
		// Может поможет потикать...
//		MachineSpirit = GetWorld()->SpawnActor<AMachineSpirit>();
//		MachineSpirit->Possess(Landraider);
	}

	// Спавним маршрут перед носом у Лэндрейдера.
	Location.X += SpawnMargin;

	if (!IsValid(RouteActor))
	{
		RouteActor = GetWorld()->SpawnActor<ARouteActor>(RouteClass, Location, Rotation);
	}
}

TArray<UStaticMesh*> AMotorsportGameModeBase::GetObstacleMeshes() const
{
	return ObstacleMeshes;
}

ARouteActor* AMotorsportGameModeBase::GetRouteActor() const
{
	return RouteActor;
}

ALandraider* AMotorsportGameModeBase::GetMachineActor() const
{
	return Landraider;
}

void AMotorsportGameModeBase::SetGameStatus(EGameStatus Status)
{
	GameStatus = Status;
}

EGameStatus AMotorsportGameModeBase::GetGameStatus() const
{
	return GameStatus;
}

AActor* AMotorsportGameModeBase::GetGround() const
{
	return GroundActor;
}

FVector AMotorsportGameModeBase::GetGroundBounds() const
{
	return GroundAccessBounds;
}
