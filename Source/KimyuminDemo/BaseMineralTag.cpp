// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseMineralTag.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Kismet/GameplayStatics.h"
#include "BaseMineral.h"

void UBaseMineralTag::NativeConstruct()
{
    Super::NativeConstruct();

    // ���� �ʱ�ȭ�ϰ� �̸��� HP ǥ��
    if (ABaseMineral* Base = Cast<ABaseMineral>(UGameplayStatics::GetActorOfClass(this, ABaseMineral::StaticClass())))
    {
        // �����ε�?
        if (MineralName)
        {
            // MineralName->SetText(FText::FromName(Base->UnderMineralName));
        }

        // �Ҽ��� ���� ���� ���·� ǥ��
        if (HPBar)
        {
            HPBar->SetPercent(GetPercent());
        }
    }
}

// GetPercent ���� �Լ��ε�...
float UBaseMineralTag::GetPercent() const
{
    if (ABaseMineral* Base = Cast<ABaseMineral>(UGameplayStatics::GetActorOfClass(this, ABaseMineral::StaticClass())))
    {
        const float MaxHP = 100.0f;
        return FMath::Clamp(static_cast<float>(Base->HitPoints) / MaxHP, 0.0f, 1.0f);
    }
    return 0.0f;
}
