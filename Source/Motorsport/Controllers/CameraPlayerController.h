// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CameraPlayerController.generated.h"

class AStaticMeshActor;
class ACameraPlayerPawn;
class AMotorsportGameModeBase;

class ARouteActor;

UCLASS()
class MOTORSPORT_API ACameraPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	ACameraPlayerController();

private:

	int32 ScreenSizeX = 0;
	int32 ScreenSizeY = 0;
	// Чтобы объект не спавился в "полу". Не универсальненько, а что делать.
	float GroundThickness = 20.f;
	bool bMouseButtonIsPressed = false;

	UPROPERTY()
	ACameraPlayerPawn* PlayerCamera = nullptr;

	UPROPERTY()
	AMotorsportGameModeBase* GameMode = nullptr;

	void MouseLeftClick();

	void MouseLeftReleased();

	void ZoomIn();

	void ZoomOut();

	FVector MoveScreen();

protected:

	UPROPERTY(EditAnywhere, Category = "Zoom")
	float ZoomRate = 10.f;

	UPROPERTY(EditAnywhere, Category = "FlipThrough")
	int32 Margin = 10;

	UPROPERTY(EditAnywhere, Category = "FlipThrough")
	float FlipThroughSpeed = 30.f;

	UPROPERTY(EditAnywhere, Category = "RouteBuild")
	int32 RouteNodesAmount = 10;

	UPROPERTY(BlueprintReadWrite)
	AStaticMeshActor* FocusedActor = nullptr;

	// Явно не на своём месте, но ничего не поделаешь.
	UPROPERTY(EditDefaultsOnly, Category = "Obstacles")
	TSubclassOf<AStaticMeshActor> ObstacleClass;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	// Spawn object of random AStaticMeshClass
	UFUNCTION(BlueprintCallable)
	void SpawnObstacle();

	// Build Spline Route for Landraider
	UFUNCTION(BlueprintCallable)
	void BuildRoute();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupInputComponent() override;
};
