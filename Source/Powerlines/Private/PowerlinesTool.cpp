// Fill out your copyright notice in the Description page of Project Settings.


#include "PowerlinesTool.h"
#include "Drawing/ScriptableToolLineSet.h"
#include "Drawing/ScriptableToolLine.h"
#include "Drawing/ScriptableToolPointSet.h"
#include "Drawing/ScriptableToolPoint.h"
#include "Kismet/KismetMathLibrary.h"
#include"UObject/UnrealType.h"

UPowerlinesTool::UPowerlinesTool()
{

}

void UPowerlinesTool::Setup()
{
    UScriptableModularBehaviorTool::Setup();
    
    int CapturePriorityL =0;

     CanBeginClickSequence.BindDynamic(this, &UPowerlinesTool::CanBeginSequence);
    
     OnBeginSequencePreview.BindDynamic(this, &UPowerlinesTool::ShowLinePreview);
     OnNextSequencePreview.BindDynamic(this, &UPowerlinesTool::ShowLinePreview);

     OnBeginClickSequence.BindDynamic(this, &UPowerlinesTool::ShowLineAndUpdatePowerlines);
     OnNextSequenceClick.BindDynamic(this, &UPowerlinesTool::ShowLineAndCreateOrUpdatePowerlines);
     OnTerminateClickSequence.BindDynamic(this, &UPowerlinesTool::RemoveLinesAndPoints);
    
    AddMultiClickSequenceBehavior(OnBeginSequencePreview,  
    CanBeginClickSequence,  
    OnBeginClickSequence,  
    OnNextSequencePreview,  
    OnNextSequenceClick,  
    OnTerminateClickSequence,  
    RequestAbortClickSequence,  
     CaptureCheck,  
    HoverCaptureCheck,  
    CapturePriorityL,  
    EScriptableToolMouseButton::LeftButton);

    LineSet = AddLineSet();
    PointSet = AddPointSet();

}

bool UPowerlinesTool::CanBeginSequence(FInputDeviceRay ClickPos, EScriptableToolMouseButton MouseButton)
{
    return true;
}

void UPowerlinesTool::ShowLinePreview(FInputDeviceRay ClickPos, FScriptableToolModifierStates Modifiers,
                                      EScriptableToolMouseButton MouseButton)
{
    FVector CursorLocationL = GetLocationUnderCursor(ClickPos);
    
    if (CurrentLinePreview)
    {
        CurrentLinePreview->SetLineEnd(CursorLocationL);
        CurrentLinePreview->SetLineColor(LinePreviewColor);
    }

}

void UPowerlinesTool::ShowLineAndUpdatePowerlines(FInputDeviceRay ClickPos, FScriptableToolModifierStates Modifiers,
    EScriptableToolMouseButton MouseButton)
{
    FVector CursorLocationL = GetLocationUnderCursor(ClickPos);
    AddLineSegment(CursorLocationL);

    FVector LineStartL, LineEndL;
    GetLastTwoRoutePoints(LineStartL, LineEndL);
    if (SpawnedPowerlines)
    {
        SpawnedPowerlines->AddSplinePoints(LineStartL, LineEndL);
        SpawnedPowerlines->UpdatePowerlines();
    }
}

bool UPowerlinesTool::ShowLineAndCreateOrUpdatePowerlines(FInputDeviceRay ClickPos,
    FScriptableToolModifierStates Modifiers, EScriptableToolMouseButton MouseButton)
{
    FVector CursorLocationL = GetLocationUnderCursor(ClickPos);
    AddLineSegment(CursorLocationL);
    
    FVector LineStartL, LineEndL;
    GetLastTwoRoutePoints(LineStartL, LineEndL);
    if (SpawnedPowerlines)
    {
        SpawnedPowerlines->AddSplinePoints(LineStartL, LineEndL);
        SpawnedPowerlines->UpdatePowerlines();
    }
    else
    {
        SpawnedPowerlines = SpawnPowerlines(LineStartL, LineEndL);
        SpawnedPowerlines->UpdatePowerlines();
    }

    return true;
}

void UPowerlinesTool::Shutdown(EToolShutdownType ShutdownType)
{
    UScriptableModularBehaviorTool::Shutdown(ShutdownType);
}


void UPowerlinesTool::RemoveLinesAndPoints(FScriptableToolModifierStates Modifiers, EScriptableToolMouseButton MouseButton)
{
    LineSet->RemoveAllLines();
    PointSet->RemoveAllPoints();
}

void UPowerlinesTool::AddLineSegment(const FVector& PositionToAdd)
{
    SplinePoints.Add(PositionToAdd);
    UScriptableToolPoint * PointL = PointSet->AddPoint();
    
    PointL->SetPointSize(10.0f);
    PointL->SetPointPosition(PositionToAdd);

    FVector OffSetPositionL = PositionToAdd + FVector(0.0f, 0.0f, 10.0f);
    if (CurrentLinePreview)
    {
        CurrentLinePreview->SetLineEnd(OffSetPositionL);
        CurrentLinePreview->SetLineColor(LineLockedColor);
    }

    CurrentLinePreview = LineSet->AddLine();
    CurrentLinePreview->SetLineEndPoints(OffSetPositionL,OffSetPositionL);
    CurrentLinePreview->SetLineColor(LinePreviewColor);
    CurrentLinePreview->SetLineThickness(5.0f);

}

FVector UPowerlinesTool::GetLocationUnderCursor(FInputDeviceRay Ray) const
{
    UWorld* WorldL = GetWorld();
    if (!WorldL) return  FVector::ZeroVector;

    float RayDistanceL = 1000000.0f;
    FVector StartL = Ray.WorldRay.Origin;
    FVector EndL = StartL + Ray.WorldRay.Direction * RayDistanceL;
    FHitResult HitResultL;
    
    bool bHitL = WorldL->LineTraceSingleByChannel(
        HitResultL,  
        StartL,      
        EndL,        
        ECC_Visibility, 
        FCollisionQueryParams::DefaultQueryParam, 
        FCollisionResponseParams::DefaultResponseParam 
    );
	
    if (bHitL)
    {
        return HitResultL.Location;
    }
    return FVector::ZeroVector;
}

APowergrid * UPowerlinesTool::SpawnPowerlines(const FVector &SpawnLocation, const FVector &DestinationLocation) const
{
    float LineLengthL = FVector::Dist(SpawnLocation, DestinationLocation);
    int PolesNumL = FMath::Floor(LineLengthL * (1/CableLength)) + 1;
    
    FActorSpawnParameters SpawnParamsL;
    SpawnParamsL.bDeferConstruction = true;
    SpawnParamsL.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    FRotator SpawnRotationL = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, DestinationLocation);
    
    if (LineLengthL > CableLength)
    {
        APowergrid * SpawnedPowergridL = GetWorld()->SpawnActor<APowergrid>(APowergrid::StaticClass(),SpawnLocation, SpawnRotationL, SpawnParamsL);

        if (SpawnedPowergridL)
        {
            SpawnedPowergridL->InitializePowerlineParams(PowerlinesMesh, PolesNumL, CableLength);
            SpawnedPowergridL->FinishSpawning(FTransform(SpawnRotationL, SpawnLocation));
        }
        return SpawnedPowergridL;
    }
    
    return nullptr;
}

void UPowerlinesTool::GetLastTwoRoutePoints(FVector& SecondToLast, FVector& Last) const
{
    if (SplinePoints.Num() > 1)
    {
        Last = SplinePoints[SplinePoints.Num() - 1];
        SecondToLast = SplinePoints[SplinePoints.Num() - 2];
    }
}
