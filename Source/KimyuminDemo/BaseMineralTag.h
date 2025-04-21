// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseMineralTag.generated.h"

/**
 * 
 */
UCLASS()
class KIMYUMINDEMO_API UBaseMineralTag : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// �̳׶� �̸�
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UTextBlock* MineralName;

	// ���� ü�¹�
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UProgressBar* HPBar;

protected:
	virtual void NativeConstruct() override;

public:
	// GetPercent �Լ�
	UFUNCTION(BlueprintPure, Category = "Mineral")
	float GetPercent() const;
};
