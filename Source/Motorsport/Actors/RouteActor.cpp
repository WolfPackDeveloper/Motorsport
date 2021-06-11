// Fill out your copyright notice in the Description page of Project Settings.


#include "RouteActor.h"
#include "MotorsportGameModeBase.h"

#include "Components/SplineComponent.h"
//#include "Math/Box.h" // Route bounds
#include "Engine/Engine.h" // Debug strings
#include "Kismet/GameplayStatics.h" // GetGameMode

// Sets default values
ARouteActor::ARouteActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//RouteBoundMargin = DefaultMargin;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	RouteSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RouteSpline"));
	RouteSpline->SetupAttachment(Root);
	RouteSpline->ScaleVisualizationWidth = 50.f;
}

//void ARouteActor::GetBoundsForRoute(AActor* Ground)
//{
//	Ground->GetActorBounds(true, RouteBoundOrigin, RouteBoundExtent);
//
//	// Причёсываем границы
//	// Какая-то ситуация - он что берёт половину размера от центра (root)?
//	// В таком случае вектор начала вообще не нужет - берём +- Extent
//	//RouteBoundOrigin.X += RouteBoundMargin;
//	//RouteBoundOrigin.Y += RouteBoundMargin;
//	//RouteBoundOrigin.Z = RouteBoundExtent.Z;
//
//	RouteBoundExtent.X -= RouteBoundMargin;
//	RouteBoundExtent.Y -= RouteBoundMargin;
//
//	//Debug
//	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Bound Origin %s"), *RouteBoundOrigin.ToString()));
//	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Bound Extent %s"), *RouteBoundExtent.ToString()));
//}



void ARouteActor::GetBoundsForRoute()
{
	if (GameMode)
	{
		RouteBounds = GameMode->GetGroundBounds();
		RouteBounds.X -= RouteBoundMargin;
		RouteBounds.Y -= RouteBoundMargin;
	}

	// Debug
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Bound Extent %s"), *RouteBounds.ToString()));
}

// Called when the game starts or when spawned
void ARouteActor::BeginPlay()
{
	Super::BeginPlay();
	
	GameMode = Cast<AMotorsportGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	GetBoundsForRoute();
}

//void ARouteActor::SetRouteBoundsMargin(float Margin)
//{
//	if (Margin <= DefaultMargin)
//	{
//		RouteBoundMargin = DefaultMargin;
//		return;
//	}
//
//	RouteBoundMargin = Margin;
//}

//float ARouteActor::GetRouteBoundsMargin()
//{
//	return RouteBoundMargin;
//}

// Called every frame
void ARouteActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

USplineComponent* ARouteActor::GetRouteSpline() const
{
	return RouteSpline;
}

void ARouteActor::BuildRoute(AActor* Ground, int32 NodesAmount)
{
	if (!IsValid(Ground)) return;
	
	// Очищаем старый маршрут
	RouteSpline->ClearSplinePoints();
	// Если длина меньше узла - оставляем пустой маршрут. Для движения без маршрута.
	if (NodesAmount < 1) return;

	// Строим маршрут
	//GetBoundsForRoute(); // Имеет ли смысл? В BeginPlay уже есть.
	
	// И двигаем положение актора в первую точку маршрута. Можно привязать начало маршрута к положению Лэндрейдера.
	// Например, его положение, или +100 по X.

}

