// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CableComponent.h"
#include "Powergrid.generated.h"


class USplineComponent;
class USplineMeshComponent;

USTRUCT(BlueprintType)
struct FCableInfo
{
	GENERATED_BODY()

	FCableInfo(){};
	
	FCableInfo(UCableComponent* CableRefToSet, float SplinePositionToSet, FName CableSocketToSet, float CableLengthToSet, bool bEnableCableStiffnessToSet, FVector EndPointToSet)
	:CableRef(CableRefToSet),
	SplinePosition(SplinePositionToSet),
	CableSocket(CableSocketToSet),
	CableLength(CableLengthToSet),
	bEnableCableStiffness(bEnableCableStiffnessToSet),
	EndPoint(EndPointToSet)
	{};

	UPROPERTY(EditDefaultsOnly)
	UCableComponent* CableRef;

	UPROPERTY(EditDefaultsOnly)
	float SplinePosition = 0.0f;

	UPROPERTY(EditDefaultsOnly, meta = (ToolTip = "The PowerlineMesh socket name"))
	FName CableSocket = NAME_None;

	UPROPERTY(EditDefaultsOnly)
	float CableLength = 500.0f;
    
	UPROPERTY(EditDefaultsOnly)
	bool bEnableCableStiffness = false;
    
	UPROPERTY(EditDefaultsOnly)
	FVector EndPoint = FVector::ZeroVector;
};

UCLASS()
class POWERLINES_API APowergrid : public AActor
{
	GENERATED_BODY()
	
public:	

	APowergrid();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USplineComponent* Spline;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TArray<UStaticMeshComponent*> PowerPoleMeshes;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TArray<UCableComponent*> Cables;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Power Grid")
	UStaticMesh* PowerlineMesh = nullptr;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Power Grid")
	int32 InitialPolesNum = 5;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Power Grid")
	float CableLength = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Power Grid")
	TArray<FCableInfo> CableInfos;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Power Grid")
	bool bInitialPolesSpawned = false;
    
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Power Grid")
	float RemainingLineLength = 0.0f;

public:
	
	UFUNCTION(Category = "Power Grid")
	void AddSplinePoints(const FVector& LineStart, const FVector& LineEnd);

	UFUNCTION(Category = "Power Grid")
	void UpdatePowerlines();

	UFUNCTION(Category = "Power Grid")
	void SetRemainingLineLength(float LengthToSet);

	UFUNCTION(Category = "Power Grid")
	void InitializePowerlineParams(UStaticMesh* PowerlineMeshToSet, int PolesNumToSet, float CableLengthToSet);
	
private:

	UFUNCTION(Category = "Power Grid")
	TArray<UStaticMeshComponent*> SpawnPowerlineMeshesAlongSpline();
    
	UFUNCTION(Category = "Power Grid")
	void SetCableProperties(FCableInfo &CableInfoToSet);
    
	UFUNCTION(Category = "Power Grid")
	void DestroyPowerlinesAndCables();

	UFUNCTION(Category = "Power Grid")
	UCableComponent* AddCableComponent(UStaticMeshComponent* MeshComponent, FName AttachSocketName);

	UFUNCTION(Category = "Power Grid")
	USplineMeshComponent* AddPowerlineMeshToSpline(int SplineIndex);

};
