// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
//#include "Engine/SplineMeshActor.h"
#include "RouteActor.generated.h"

class USceneComponent;
class USplineComponent;

class AMotorsportGameModeBase;

UCLASS()
class MOTORSPORT_API ARouteActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARouteActor();

private:
	
//	FVector RouteBoundOrigin;
//	FVector RouteBoundExtent;
//	float DefaultMargin = 200.f;

	FVector RouteBounds;

	UPROPERTY()
	USceneComponent* Root = nullptr;

	UPROPERTY()
	AMotorsportGameModeBase* GameMode = nullptr;

//	void GetBoundsForRoute(AActor* Ground);
	void GetBoundsForRoute();

protected:
	
	UPROPERTY(BlueprintReadOnly)
	USplineComponent* RouteSpline = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	float RouteBoundMargin = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	float TangentMin = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	float TangentMax = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	float NodeDistanceMin = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	float NodeDistanceMax = 300.f;


	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	// Не знаю, а нужно ли менять границу в рантайме вообще?
	//UFUNCTION(BlueprintCallable)
	//void SetRouteBoundsMargin(float Margin);

	//UFUNCTION(BlueprintCallable)
	//float GetRouteBoundsMargin();

	UFUNCTION(BlueprintCallable)
	USplineComponent* GetRouteSpline() const;

	UFUNCTION(BlueprintCallable)
	void BuildRoute(AActor* Ground, int32 NodesAmount);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
