// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Landraider.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class MOTORSPORT_API ALandraider : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ALandraider();

private:
	
	UPROPERTY()
	USceneComponent* Root = nullptr;

protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mass")
	float Mass = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UStaticMeshComponent* Mesh;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
