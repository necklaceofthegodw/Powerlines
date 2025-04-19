// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Powergrid.h"
#include "BaseTools/ScriptableModularBehaviorTool.h"
#include "Drawing/ScriptableToolLineSet.h"
#include "PowerlinesTool.generated.h"


class UScriptableInteractiveToolPropertySet;
class APowergrid;

//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCableLengthChanged);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaticMeshChanged);

UCLASS()
class POWERLINES_API UPowerlinesTool : public UScriptableModularBehaviorTool
{

private:
	GENERATED_BODY()

public:

	UPowerlinesTool();

    // Events
	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;


	FCanBeginClickSequenceDelegate CanBeginClickSequence; 
	FOnBeginSequencePreviewDelegate OnBeginSequencePreview;
	FOnNextSequencePreviewDelegate OnNextSequencePreview; 
	FOnBeginClickSequenceDelegate OnBeginClickSequence;
	FOnNextSequenceClickDelegate OnNextSequenceClick;
	
	FOnTerminateClickSequenceDelegate OnTerminateClickSequence;
	FRequestAbortClickSequenceDelegate RequestAbortClickSequence;
	FMouseBehaviorModiferCheckDelegate CaptureCheck;  
	const FMouseBehaviorModiferCheckDelegate HoverCaptureCheck;


	//UFUNCTION()
	//bool HoverCaptureCheck1(const FInputDeviceState&, InputDeviceState);
    // UPROPERTY(BlueprintAssignable, Category = "Powerlines|Events")
    // FOnCableLengthChanged OnCableLengthChanged;
    //
    // UPROPERTY(BlueprintAssignable, Category = "Powerlines|Events")
    // FOnStaticMeshChanged OnStaticMeshChanged;
	UFUNCTION()
	bool CanBeginSequence(FInputDeviceRay ClickPos, EScriptableToolMouseButton MouseButton);

    // Drawing functions
	UFUNCTION(Category = "Powerlines|Drawing")
	void ShowLinePreview(FInputDeviceRay ClickPos, FScriptableToolModifierStates Modifiers, EScriptableToolMouseButton MouseButton);

    UFUNCTION(Category = "Powerlines|Drawing")
    void ShowLineAndUpdatePowerlines(FInputDeviceRay ClickPos, FScriptableToolModifierStates Modifiers, EScriptableToolMouseButton MouseButton);

    UFUNCTION(Category = "Powerlines|Drawing") 
    bool ShowLineAndCreateOrUpdatePowerlines(FInputDeviceRay ClickPos, FScriptableToolModifierStates Modifiers, EScriptableToolMouseButton MouseButton);

    UFUNCTION(Category = "Powerlines|Drawing")
    void RemoveLinesAndPoints(FScriptableToolModifierStates Modifiers, EScriptableToolMouseButton MouseButton);

    UFUNCTION(Category = "Powerlines|Drawing")
    void AddLineSegment(const FVector& PositionToAdd);

    // Utility functions
    UFUNCTION(Category = "Powerlines|Utility")
    FVector GetLocationUnderCursor(FInputDeviceRay Ray) const;

    UFUNCTION(Category = "Powerlines|Utility")
    APowergrid * SpawnPowerlines(const FVector &SpawnLocation, const FVector &DestinationLocation) const;

    UFUNCTION(Category = "Powerlines|Utility")
    void GetLastTwoRoutePoints(FVector& SecondToLast, FVector& Last) const;
	
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Powerlines|Variables")
    UScriptableToolLineSet *LineSet = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Powerlines|Variables")
    UScriptableToolPointSet *PointSet = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Powerlines|Variables")
    TArray<FVector> SplinePoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Powerlines|Variables")
    UScriptableToolLine *CurrentLinePreview = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Powerlines|Variables")
    FColor LineLockedColor = FColor::Red;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Powerlines|Variables")
    FColor LinePreviewColor = FColor::Purple;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Powerlines|Variables")
    UScriptableInteractiveToolPropertySet* PowerlinesPropertySet;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Powerlines|Variables")
    float CableLength;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Powerlines|Variables")
    APowergrid* SpawnedPowerlines;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Powerlines|Variables")
    UStaticMesh* PowerlinesMesh;
	
};


