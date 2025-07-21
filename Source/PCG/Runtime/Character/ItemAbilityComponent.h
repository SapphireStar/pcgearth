// ItemAbilityComponent.h - 更新后的版本
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemAbilityComponent.generated.h"


UENUM(BlueprintType)
enum class EAbilityType : uint8
{
	None,
	TerrainBuild,
	TerrainDig,
	GetResource,
	// 可以继续添加其他能力类型
	MAX UMETA(Hidden)
};

// 委托声明 - 当能力切换时通知
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityChanged, EAbilityType, OldAbility, EAbilityType, NewAbility);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityActivated, EAbilityType, AbilityType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityDeactivated, EAbilityType, AbilityType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityStatusChanged, class UItemAbilityComponent*, AbilityComponent);

UCLASS(Blueprintable)
class PCG_API UItemAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UItemAbilityComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Ability", meta = (CallInEditor = "true"))
	void ReceiveOnInitializeAbility();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ability", meta = (CallInEditor = "true"))
	void ReceiveOnActivateAbility();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ability", meta = (CallInEditor = "true"))
	void ReceiveOnTickAbility();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ability", meta = (CallInEditor = "true"))
	void ReceiveOnDeactivateAbility();
	
	UFUNCTION(BlueprintCallable)
	virtual void OnInitializeAbility();

	UFUNCTION(BlueprintCallable)
	virtual void OnActivateAbility();

	UFUNCTION(BlueprintCallable)
	virtual void OnTickAbility();

	UFUNCTION(BlueprintCallable)
	virtual void OnDeactivateAbility();

	// 能力状态查询
	UFUNCTION(BlueprintPure)
	bool IsActivated() const { return bIsActivated; }

	UFUNCTION(BlueprintPure)
	bool IsInitialized() const { return bIsInitialized; }

	// 能力信息
	UFUNCTION(BlueprintPure)
	FString GetAbilityName() const { return AbilityName; }

	UFUNCTION(BlueprintPure)
	FString GetAbilityDescription() const { return AbilityDescription; }

	UFUNCTION(BlueprintCallable)
	virtual void OnStartUseAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera);

	UFUNCTION(BlueprintCallable)
	virtual void OnKeepUsingAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera);

	UFUNCTION(BlueprintCallable)
	virtual void OnCompleteUseAbility(UPrimitiveComponent* TraceStartComp, UCameraComponent* Camera);

public:
	// 委托事件
	UPROPERTY(BlueprintAssignable)
	FOnAbilityStatusChanged OnAbilityActivated;

	UPROPERTY(BlueprintAssignable)
	FOnAbilityStatusChanged OnAbilityDeactivated;

	// 能力状态
	UPROPERTY(BlueprintReadOnly)
	bool bIsActivated = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsInitialized = false;

	UPROPERTY(BlueprintReadOnly)
	EAbilityType AbilityType = EAbilityType::None;

protected:
	// 能力基本信息
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Info")
	FString AbilityName = TEXT("Base Ability");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Info", meta = (MultiLine = true))
	FString AbilityDescription = TEXT("Base ability component");

	// 能力图标（可选）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Info")
	TObjectPtr<class UTexture2D> AbilityIcon;
};