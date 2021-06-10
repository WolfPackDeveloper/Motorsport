// Fill out your copyright notice in the Description page of Project Settings.


#include "MotorsportGameModeBase.h"
#include "Actors/RouteActor.h"
#include "Pawns/Landraider.h"

#include "Engine/StaticMesh.h" // Ну хз... поможет ли?
#include "Engine/StaticMeshActor.h" // Find Ground - lazy method
#include "Kismet/GameplayStatics.h" // Find Ground


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

void AMotorsportGameModeBase::BeginPlay()
{	
	Super::BeginPlay();

	DefineGroundActor(TEXT("Ground"));
	
	RouteActor = GetWorld()->SpawnActor<ARouteActor>(RouteClass);

	// Спавним трактор. и назначаем ему позицию.
	Landraider = GetWorld()->SpawnActor<ALandraider>(MachineClass);
}

TArray<UStaticMesh*> AMotorsportGameModeBase::GetObstacleMeshes() const
{
	return ObstacleMeshes;
}

ARouteActor* AMotorsportGameModeBase::GetRouteActor() const
{
	return RouteActor;
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
