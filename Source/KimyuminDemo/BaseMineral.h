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

	// �ε�����
	UFUNCTION()
	void OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// HP Bar ����
	UPROPERTY(EditAnywhere, Category = "UI")
	UWidgetComponent* HPBarWidgetComponent;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	UBaseMineralTag* WB_HPBar;

	// �������� ���� �̸�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mineral")
	FName UnderMineralName;

	// �������� ���� BluePrint
	UPROPERTY(EditDefaultsOnly, Category = "Mineral")
	TSubclassOf<AActor> UnderMineralBP;

	// �������� ���� Niagara
	UPROPERTY(EditDefaultsOnly, Category = "Mineral")
	UNiagaraSystem* UnderMineralNS;

	// ���� MI�� Dissolve �� ��� ��
	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInstanceDynamic* DMI_Dissolved;

	// ���� Mesh
	UPROPERTY(EditAnywhere, Category = "Mesh")
	UPrimitiveComponent* MineralMesh;

	// ���� ü��
	UPROPERTY(EditAnywhere, Category = "Mineral")
	double InitialHitPoints = 100.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mineral")
	double HitPoints;

	// ���� �������
	UFUNCTION()
	void HandleDissolveStep();

	FTimerHandle DissolveTimerHandle;
	float CurrentVal = 0.0f;
};
