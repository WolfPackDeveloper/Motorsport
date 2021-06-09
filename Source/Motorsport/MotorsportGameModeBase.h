// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "ConstructorHelpers.h" // Random Meshes

#include "MotorsportGameModeBase.generated.h"

class UStaticMesh;

UCLASS()
class MOTORSPORT_API AMotorsportGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	AMotorsportGameModeBase();

private:

	UPROPERTY()
	TArray<UStaticMesh*> ObstacleMeshes;

	//void AddMeshForObstacles(UStaticMesh* Mesh);
	void AddMeshForObstacles(TCHAR* ObjectFQName);

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	UFUNCTION()
	TArray<UStaticMesh*> GetObstacleMeshes() const;

};
