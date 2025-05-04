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

	//�ۻ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple", meta = (AllowPrivateAccess = "true"))
	USceneComponent* GrappleStartLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple", meta = (AllowPrivateAccess = "true"))
	class UCableComponent* GrappleCable;

	//���� ��� �� �� 
	UPROPERTY(EditAnywhere, Category = WeaponMode, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* LaserMesh;

	UPROPERTY(EditAnywhere, Category = WeaponMode, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UArrowComponent* LaserArrow;

	//UPROPERTY(EditAnywhere, Category = WeaponMode, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	//class UStaticMeshComponent* LaserSphere;

	UPROPERTY(EditAnywhere, Category = WeaponMode, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* DissolveLaser;

	UPROPERTY(EditAnywhere, Category = WeaponMode, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Laser;

	UPROPERTY(EditAnywhere, Category = WeaponMode, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* RifleMesh;

	//�� ī�޶�
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* MapCameraBoom;

	UPROPERTY(EditAnywhere, Category = Map, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* MapCamera;


//�⺻ ���� ����
private:

	UPROPERTY()
	class UMyGameInstance* myGameInstance;

	float DeltaSeconds;

//�⺻ ���� ����
public:

	UPROPERTY(BlueprintReadWrite)
	int CurrentCharacterIndex;

	UPROPERTY(BlueprintReadWrite)
	bool isInCave;

	// ī�޶� ����
	UPROPERTY(EditAnywhere)
	float DefaultArmLength = 400.0;
	UPROPERTY(EditAnywhere)
	float ZoomedArmLength = 150.f;

	UPROPERTY(EditAnywhere)
	FVector DefaultCameraOffset = FVector(0.f, 0.f, 0.f);  // �⺻ ��ġ
	UPROPERTY(EditAnywhere)
	FVector ZoomedCameraOffset = FVector(0.f, 50.f, 30.f); // ������+�������� �̵�


	//��� (�⺻, ����)
	UPROPERTY(BlueprintReadWrite)
	int modeNumber;
	UFUNCTION(BlueprintCallable)
	void ModeChange();
	UFUNCTION()
	void SetDefaultMode();
	UFUNCTION()
	void SetWeaponMode();

	//UI
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> WB_UIClass;
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> WB_UIPictorialBook;
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> WB_UIMapSelect;
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> WB_UIStore;

	//���� UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UUserWidget* pictorialBook;

	//�� ���� or ���� UI
	UPROPERTY(BlueprintReadWrite)
	class UUserWidget* wbMapSelect;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool isOverlapMapSelectZone{ false };
	UPROPERTY(BlueprintReadWrite)
	class UUserWidget* wbStore;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool isOverlapStore{ false };
	UFUNCTION(BlueprintCallable)
	void MapAndStoreFlipFlop();


	//����ź
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flare")
	int flareNum;
	UPROPERTY(EditAnywhere, Category = "Flare")
	float flareSpeed;
	UPROPERTY(EditAnywhere, Category = "Flare")
	TSubclassOf<AActor> BP_Flare;
	UFUNCTION(BlueprintCallable)
	void SpawnFlare();

	//���
	bool isDash;
	UFUNCTION(BlueprintCallable)
	void DashFlipFlop();

	//����
	bool isPictorialOpen;
	UFUNCTION(BlueprintCallable)
	void PictorialFlipFlop();

	//����
	UFUNCTION(BlueprintCallable)
	void LeftMouseBtnPressed();
		//1�� ����
		UPROPERTY(EditAnywhere)
		class UParticleSystem* ExplosionFX;

		//2�� ����
		FTimerHandle LaserTimerHandle;
		void StartFiringLaser();
		void FireLaserTick();
		UPROPERTY(EditAnywhere)
		class UParticleSystem* LaserImpactFX;
		
		UPROPERTY(EditAnywhere)
		TSubclassOf<AActor> LaserCollisionClass;

		//3�� ����
		bool IsHoldingF;
		UPROPERTY(EditAnywhere)
		float grappleDistance;
		UPROPERTY(EditAnywhere)
		TSubclassOf<AActor> DissolveClass;
		UPROPERTY(EditAnywhere)
		AActor* currentHole;

		UPROPERTY(EditAnywhere)
		TSubclassOf<AActor> GrappleHookClass;

		AActor* Hook;
		bool isGrappling;

		UFUNCTION()
		void ResetGrappleHook();
		void EnableGrapple();

	UFUNCTION(BlueprintCallable)
	void LeftMouseBtnReleased();

	UFUNCTION(BlueprintCallable)
	void RightMouseBtnPressed();
		FTimerHandle DissolveTimerHandle;
		UPROPERTY(EditAnywhere)
		TSubclassOf<AActor> DissolveCircleClass;
	UFUNCTION(BlueprintCallable)
	void RightMouseBtnReleased();

	//��� ����
	UFUNCTION()
	void DecreaseOxygen();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Oxygen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OxygenConsumptionRate;

	//���� ����
	UFUNCTION(BlueprintCallable)
	void OpenMap();

	bool IsMapOpen;
	bool IsMapOpenProgress;
	float MapCameraBoomLength;
	float MapCameraBoomLengthTarget;

	AActor* map_character_point;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> MapCharacterPoint;

	// üũ��
	bool bIsDissolveLaserFiring = false;

	void ResetActorPitch();

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

