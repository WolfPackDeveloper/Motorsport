// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CameraPlayerController.generated.h"

class AStaticMeshActor;
class ACameraPlayerPawn;
class AMotorsportGameModeBase;

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

	UPROPERTY(EditAnywhere, Category = "Flip Through")
	int32 Margin = 10;

	UPROPERTY(EditAnywhere, Category = "Flip Through")
	float FlipThroughSpeed = 30;

	UPROPERTY(BlueprintReadWrite)
	AStaticMeshActor* FocusedActor = nullptr;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	UPROPERTY(EditAnywhere, Category = "Obstacles")
	TSubclassOf<AStaticMeshActor> ObstacleClass;

	// Spawn object of random AStaticMeshClass
	UFUNCTION(BlueprintCallable)
	void SpawnObstacle();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupInputComponent() override;
};
