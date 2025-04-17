// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "KimyuminDemoCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AKimyuminDemoCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	//flare child actor
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ChildActor", meta = (AllowPrivateAccess = "true"))
	//class UChildActorComponent* FlareChildActor;

//기본 변수 생성
private:

	UPROPERTY()
	class UMyGameInstance* myGameInstance;

	float DeltaSeconds;

//기본 변수 생성
public:
	//UI
	bool isUiOpen;
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> WB_UIClass;
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> WB_UIPictorialBook;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UUserWidget* pictorialBook;


	//조명탄
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flare")
	int flareNum;
	UPROPERTY(EditAnywhere, Category = "Flare")
	float flareSpeed;
	UPROPERTY(EditAnywhere, Category = "Flare")
	TSubclassOf<AActor> BP_Flare;
	UFUNCTION(BlueprintCallable)
	void SpawnFlare();


	//산소 감소
	//UFUNCTION()
	//void DecreaseOxygen();

public:
	AKimyuminDemoCharacter();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};

