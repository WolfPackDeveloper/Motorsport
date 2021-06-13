// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
//#include "Engine/SplineMeshActor.h"
#include "RouteActor.generated.h"

class USceneComponent;
class USplineComponent;
class USplineMeshComponent;
class AMotorsportGameModeBase;

UENUM()
enum class ERouteType : uint8
{
	Snake
};

UENUM()
enum class EDirection : uint8
{
	Up,
	Down,
	Right,
	Left
};

UENUM()
enum class ESectionTemplate : uint8
{
	Snake,
	Turn
};

UCLASS()
class MOTORSPORT_API ARouteActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARouteActor();

private:
	
	// ������ ��� ���������� ��������
	// ������� ��������
	FVector RouteBounds;
	// ����� ������ ��������
	FVector RouteOrigin;

	// ��� ��������� �������� (���������).
	float TangentIn = 0.f;
	float TangentOut = 0.f;

	UPROPERTY()
	USceneComponent* Root = nullptr;

	UPROPERTY()
	AMotorsportGameModeBase* GameMode = nullptr;

	// TODO: �������� ������ ���������� �� SplineMeshComponent ��� ����, ����� ��� ��������� �������� ����� ���� ������� ������.
	TArray<USplineMeshComponent*> MeshesOnTheRoad;
	// �������� �� ���� ���� ������� "�����" � ���������� ������� ��������.
	void GetBoundsForRoute();

	// �������� ���������� �� ���� �� ��������� ������ ����.
	FVector GetDistanceToBounds(EDirection UpDown, EDirection RightLeft);

	// ��������� �������� ������ ��������.
	EDirection BuildTemplateSnake(EDirection InForwardDirection, float ForvardStep, float LagCosinus); // ������
	EDirection BuildTemplateTurn(EDirection InForwardDirection, EDirection InTurnDirection, float ForvardStep, float LagCosinus); // ������� 180 ��������.
	EDirection BuildTemplateFiller(EDirection InForwardDirection, float ForvardStep, float LagCosinus); // ���� �� ������� ����� �� ����������� ������, ������ ����� Forvard.

	void UpdateMesh(int StartPointIndex, int EndPointIndex);

	void BuildRouteSnake(EDirection InForwardDirection, EDirection InTurnDirection);

protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USplineComponent* RouteSpline = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	UStaticMesh* RoadMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	float RouteBoundMargin = 200.f;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

//	void OnConstruction(const FTransform& Transform) override;
	
	UFUNCTION(BlueprintCallable)
	USplineComponent* GetRouteSpline() const;

	// ���� �� ���  ��� ����� � ��������� ���������, �� � ��� � ������...
	UFUNCTION(BlueprintCallable)
	void BuildRoute(EDirection InForwardDirection, EDirection InTurnDirection, ERouteType RouteType);
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
