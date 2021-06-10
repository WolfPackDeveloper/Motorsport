// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "ConstructorHelpers.h" // Random Meshes

#include "MotorsportGameModeBase.generated.h"

class UStaticMesh;

class ARouteActor;
class ALandraider;

UENUM()
enum class EGameStatus : uint8
{
	GamePlayed,
	GamePaused,
	GameStopped
};

UCLASS()
class MOTORSPORT_API AMotorsportGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	AMotorsportGameModeBase();

private:

	UPROPERTY()
	AActor* GroundActor = nullptr;

	UPROPERTY()
	ARouteActor* RouteActor = nullptr;

	UPROPERTY()
		ALandraider* Landraider = nullptr;

	UPROPERTY()
	TArray<UStaticMesh*> ObstacleMeshes;

	void AddMeshForObstacles(TCHAR* ObjectFQName);
	
	// Согласен - суперкриво, но времени уже не хватает на изящные решения.
	void DefineGroundActor(FName GroundActorTag);

protected:

	UPROPERTY(BlueprintReadWrite)
	EGameStatus GameStatus = EGameStatus::GameStopped;

	// И это тоже.
	UPROPERTY(EditDefaultsOnly, Category = "Route")
	TSubclassOf<ARouteActor> RouteClass;

	UPROPERTY(EditDefaultsOnly, Category = "Machine")
	TSubclassOf<ALandraider> MachineClass;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	UFUNCTION()
	TArray<UStaticMesh*> GetObstacleMeshes() const;

	UFUNCTION()
	ARouteActor* GetRouteActor() const;

	UFUNCTION()
	ALandraider* GetMachineActor() const;

	UFUNCTION()
	void SetGameStatus(EGameStatus Status);

	UFUNCTION()
	EGameStatus GetGameStatus() const;

	UFUNCTION()
	AActor* GetGround() const;
};
