// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Flare.generated.h"

UCLASS()
class KIMYUMINDEMO_API AFlare : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	class UCapsuleComponent* Capsule;


	//UPROPERTY(VisibleAnywhere)
	//class UPointLightComponent* PointLight;
	
	class AKimyuminDemoCharacter* my_character;

public:	
	// Sets default values for this actor's properties
	AFlare();

	UPROPERTY(EditAnywhere)
	float flareLifeTime;
	UFUNCTION()
	void OnFlareExpired();

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* FlareMesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
