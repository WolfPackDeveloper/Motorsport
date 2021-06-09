// Fill out your copyright notice in the Description page of Project Settings.


#include "MotorsportGameModeBase.h"

#include "Engine/StaticMesh.h" // Ну хз... поможет ли?

//#include "ConstructorHelpers.h" // Random Meshes

AMotorsportGameModeBase::AMotorsportGameModeBase()
{
	// To get proper path to asset go to editor and right click the asset you want and click "Copy Refrence", it will copy asset path in to clipboard so you can paste it anywhere.
	AddMeshForObstacles(TEXT("StaticMesh'/Game/Meshes/Cone.Cone'"));
	AddMeshForObstacles(TEXT("StaticMesh'/Game/Meshes/Cube.Cube'"));
	AddMeshForObstacles(TEXT("StaticMesh'/Game/Meshes/Cylinder.Cylinder'"));
	AddMeshForObstacles(TEXT("StaticMesh'/Game/Meshes/Sphere.Sphere'"));
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

void AMotorsportGameModeBase::BeginPlay()
{	
	
}

TArray<UStaticMesh*> AMotorsportGameModeBase::GetObstacleMeshes() const
{
	return ObstacleMeshes;
}
