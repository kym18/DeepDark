// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseMineralTag.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Kismet/GameplayStatics.h"
#include "BaseMineral.h"

void UBaseMineralTag::NativeConstruct()
{
    Super::NativeConstruct();

    // 위젯 초기화하고 이름과 HP 표시
    if (ABaseMineral* Base = Cast<ABaseMineral>(UGameplayStatics::GetActorOfClass(this, ABaseMineral::StaticClass())))
    {
        // 오류인듯?
        if (MineralName)
        {
            // MineralName->SetText(FText::FromName(Base->UnderMineralName));
        }

        // 소수점 없이 정수 형태로 표시
        if (HPBar)
        {
            HPBar->SetPercent(GetPercent());
        }
    }
}

// GetPercent 관련 함수인데...
float UBaseMineralTag::GetPercent() const
{
    if (ABaseMineral* Base = Cast<ABaseMineral>(UGameplayStatics::GetActorOfClass(this, ABaseMineral::StaticClass())))
    {
        const float MaxHP = 100.0f;
        return FMath::Clamp(static_cast<float>(Base->HitPoints) / MaxHP, 0.0f, 1.0f);
    }
    return 0.0f;
}
