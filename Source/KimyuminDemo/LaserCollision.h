// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LaserCollision.generated.h"

UCLASS()
class KIMYUMINDEMO_API ALaserCollision : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALaserCollision();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	// LaserCollision 나오면 일정 시간 후 사라지게
	FTimerHandle DestroyTimerHandle;

	void DestroySelf();
};
