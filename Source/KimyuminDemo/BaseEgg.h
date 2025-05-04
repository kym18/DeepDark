// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseEgg.generated.h"

UCLASS()
class KIMYUMINDEMO_API ABaseEgg : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseEgg();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 데미지 처리 오버라이드
	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser
	) override;

	// Mesh
	UPROPERTY(EditAnywhere, Category = "Mesh")
	UPrimitiveComponent* EggMesh;

	// HP
	UPROPERTY(EditDefaultsOnly, Category = "HP")
	double CurrentHP = 1000.f;

	// Dissolve 값 담는 곳
	UPROPERTY(EditAnywhere, Category = "Material")
	UMaterialInstanceDynamic* DMI_Dissolved;

	UFUNCTION()

	// HP 0 이하면 호출
	void EggDeath();

	FTimerHandle DissolveTimerHandleEgg;
	float CurrentVal = 0.0f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
