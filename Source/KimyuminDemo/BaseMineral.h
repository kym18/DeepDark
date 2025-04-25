// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h"
#include "BaseMineral.generated.h"

// Forward-declare your UMG widget class
class UWidgetComponent;
class UBaseMineralTag;
class UNiagaraSystem;
class UStaticMeshComponent;

UCLASS()
class KIMYUMINDEMO_API ABaseMineral : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABaseMineral();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// ºÎµúÈú¶§
	UFUNCTION()
	void OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// HP Bar À§Á¬
	UPROPERTY(EditAnywhere, Category = "UI")
	UWidgetComponent* HPBarWidgetComponent;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	UBaseMineralTag* WB_HPBar;

	// ¶³¾îÁö´Â ±¤¹° ÀÌ¸§
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mineral")
	FName UnderMineralName;

	// ¶³¾îÁö´Â ±¤¹° BluePrint
	UPROPERTY(EditDefaultsOnly, Category = "Mineral")
	TSubclassOf<AActor> UnderMineralBP;

	// ¶³¾îÁö´Â ±¤¹° Niagara
	UPROPERTY(EditDefaultsOnly, Category = "Mineral")
	UNiagaraSystem* UnderMineralNS;

	// ±¤¹° MIÀÇ Dissolve °ª ´ã´Â °÷
	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInstanceDynamic* DMI_Dissolved;

	// ±¤¹° Mesh
	UPROPERTY(EditAnywhere, Category = "Mesh")
	UPrimitiveComponent* MineralMesh;

	// ±¤¹° Ã¼·Â
	UPROPERTY(EditAnywhere, Category = "Mineral")
	double InitialHitPoints = 100.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mineral")
	double HitPoints;

	// ±¤¹° »ç¶óÁö°Ô
	UFUNCTION()
	void HandleDissolveStep();

	FTimerHandle DissolveTimerHandle;
	float CurrentVal = 0.0f;
};
