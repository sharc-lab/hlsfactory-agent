#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>
using std::min;
using std::max;

// CUDA thread/block dimension constants (adjust for your workload)
#ifndef BLOCK_DIM_X
#define BLOCK_DIM_X 256
#endif
#ifndef GRID_DIM_X
#define GRID_DIM_X 1
#endif

// --- from bondsEngine.cu ---
//bondsEngine.cu
//Scott Grauer-Gray sgrauerg@gmail.com
//Contains main function for running bonds application on a GPU

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h> 

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))









// --- from bondsKernelsGpu.cu ---
//bondsKernelsGpu.cu
//Scott Grauer-Gray
//Bonds kernels to run on the GPU

#include <hip/hip_runtime.h>















































// --- from bondsKernelsGpu.h ---
//bondsKernelsGpu.cuh
//Scott Grauer-Gray
//Header for bonds kernels to run on the GPU

#ifndef BONDS_KERNELS_GPU
#define BONDS_KERNELS_GPU

#include <stdbool.h>

int monthLengthKernelGpu(int month, bool leapYear);

int monthOffsetKernelGpu(int m, bool leapYear);

int yearOffsetKernelGpu(int y);

bool isLeapKernelGpu(int y);

bondsDateStruct intializeDateKernelGpu(int d, int m, int y);

dataType yearFractionGpu(bondsDateStruct d1, bondsDateStruct d2, int dayCounter);

int dayCountGpu(bondsDateStruct d1, bondsDateStruct d2, int dayCounter);

dataType couponNotionalGpu();

dataType bondNotionalGpu();

dataType fixedRateCouponNominalGpu();

bool eventHasOccurredGpu(bondsDateStruct currDate, bondsDateStruct eventDate);

bool cashFlowHasOccurredGpu(bondsDateStruct refDate, bondsDateStruct eventDate);

bondsDateStruct advanceDateGpu(bondsDateStruct date, int numMonthsAdvance);

int getNumCashFlowsGpu(inArgsStruct inArgs, int bondNum);

void setCashFlowsGpu(inArgsStruct inArgs, int bondNum);

dataType getDirtyPriceGpu(inArgsStruct inArgs, int bondNum, cashFlowsStruct cashFlows, int numLegs);

dataType getAccruedAmountGpu(inArgsStruct inArgs, bondsDateStruct date, int bondNum, cashFlowsStruct cashFlows, int numLegs);

dataType discountingBondEngineCalculateSettlementValueGpu(inArgsStruct inArgs, int bondNum, 
    cashFlowsStruct cashFlows, int numLegs);

dataType bondAccruedAmountGpu(inArgsStruct inArgs, bondsDateStruct date, int bondNum, cashFlowsStruct cashFlows, int numLegs);

dataType bondFunctionsAccruedAmountGpu(inArgsStruct inArgs, bondsDateStruct date, int bondNum, 
    cashFlowsStruct cashFlows, int numLegs);

dataType cashFlowsAccruedAmountGpu(cashFlowsStruct cashFlows,
    bool includeSettlementDateFlows,
    bondsDateStruct settlementDate,
    int numLegs, inArgsStruct inArgs, int bondNum);

dataType fixedRateCouponAccruedAmountGpu(cashFlowsStruct cashFlows, int numLeg, bondsDateStruct d,
    inArgsStruct inArgs, int bondNum);

dataType cashFlowsNpvGpu(cashFlowsStruct cashFlows,
    bondsYieldTermStruct discountCurve,
    bool includeSettlementDateFlows,
    bondsDateStruct settlementDate,
    bondsDateStruct npvDate,
    int numLegs);

dataType bondsYieldTermStructureDiscountGpu(bondsYieldTermStruct ytStruct, bondsDateStruct t);

dataType flatForwardDiscountImplGpu(intRateStruct intRate, dataType t);

dataType interestRateDiscountFactorGpu(intRateStruct intRate, dataType t);

dataType interestRateCompoundFactorGpuTwoArgs(intRateStruct intRate, dataType t);

dataType fixedRateCouponAmountGpu(cashFlowsStruct cashFlows, int numLeg);

dataType interestRateCompoundFactorGpu(intRateStruct intRate, bondsDateStruct d1,
    bondsDateStruct d2, int dayCounter);

dataType fixedRateBondForwardSpotIncomeGpu(inArgsStruct inArgs, int bondNum, cashFlowsStruct cashFlows, int numLegs);

dataType getImpliedYieldGpu(inArgsStruct inArgs, dataType forwardValue, 
    dataType underlyingSpotValue, dataType spotIncomeIncDiscCurve, int bondNum);

dataType interestRateImpliedRateGpu(dataType compound,                                        
    int comp,
    dataType freq,
    dataType t);

dataType getMarketRepoRateGpu(bondsDateStruct d,
    int comp,
    dataType freq,
    bondsDateStruct referenceDate,
    inArgsStruct inArgs, int bondNum);

couponStruct cashFlowsNextCashFlowGpu(cashFlowsStruct cashFlows,
    bondsDateStruct settlementDate,
    int numLegs);

int cashFlowsNextCashFlowNumGpu(cashFlowsStruct cashFlows,
    bondsDateStruct settlementDate,
    int numLegs);

dataType getBondYieldGpu(dataType cleanPrice,
    int dc,
    int comp,
    dataType freq,
    bondsDateStruct settlement,
    dataType accuracy,
    int maxEvaluations,
    inArgsStruct currInArgs, int bondNum, cashFlowsStruct cashFlows, int numLegs);

dataType getBondFunctionsYieldGpu(dataType cleanPrice,
    int dc,
    int comp,
    dataType freq,
    bondsDateStruct settlement,
    dataType accuracy,
    int maxEvaluations,
    inArgsStruct currInArgs, int bondNum, cashFlowsStruct cashFlows, int numLegs);

dataType solverSolveGpu(solverStruct solver,
    irrFinderStruct f,
    dataType accuracy,
    dataType guess,
    dataType step,
    cashFlowsStruct cashFlows,
    int numLegs);

dataType cashFlowsNpvYieldGpu(cashFlowsStruct cashFlows,
    intRateStruct y,
    bool includeSettlementDateFlows,
    bondsDateStruct settlementDate,
    bondsDateStruct npvDate,
    int numLegs);

dataType fOpGpu(irrFinderStruct f, dataType y, cashFlowsStruct cashFlows, int numLegs);

dataType fDerivativeGpu(irrFinderStruct f, dataType y, cashFlowsStruct cashFlows, int numLegs);

bool closeGpu(dataType x, dataType y);

bool closeGpuThreeArgs(dataType x, dataType y, int n);

dataType enforceBoundsGpu(dataType x);

dataType solveImplGpu(solverStruct solver, irrFinderStruct f,
    dataType xAccuracy, cashFlowsStruct cashFlows, int numLegs);

dataType modifiedDurationGpu(cashFlowsStruct cashFlows,
    intRateStruct y,
    bool includeSettlementDateFlows,
    bondsDateStruct settlementDate,
    bondsDateStruct npvDate,
    int numLegs);

dataType getCashFlowsYieldGpu(cashFlowsStruct cashFlows,
    dataType npv,
    int dayCounter,
    int compounding,
    dataType frequency,
    bool includeSettlementDateFlows,
    bondsDateStruct settlementDate,
    bondsDateStruct npvDate,
    int numLegs,
    dataType accuracy/* = 1.0e-10*/,
    int maxIterations/* = 100*/,
    dataType guess/* = 0.05f*/);

#endif //BONDS_KERNELS_GPU



// --- from bondsStructs.h ---
//bondsStructs.cuh
//Scott Grauer-Gray
//Structs for running the bonds application

#ifndef BONDS_STRUCTS_CUH
#define BONDS_STRUCTS_CUH

typedef float dataType;

#include <stdlib.h>
#include <math.h>

#define SIMPLE_INTEREST 0
#define COMPOUNDED_INTEREST 1
#define CONTINUOUS_INTEREST 2
#define SIMPLE_THEN_COMPOUNDED_INTEREST 3

#define ANNUAL_FREQ 1
#define SEMIANNUAL_FREQ 2

#define USE_EXACT_DAY 0
#define USE_SERIAL_NUMS 1

#define QL_EPSILON_GPU 0.000000000000000001f 

#define COMPUTE_AMOUNT -1

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#define ACCURACY 1.0e-8

typedef struct
{
  int month;
  int day;
  int year;
  int dateSerialNum;
} bondsDateStruct;

typedef struct
{
  bondsDateStruct startDate;
  bondsDateStruct maturityDate;
  float rate;
} bondStruct;

typedef struct
{
  dataType rate;
  dataType freq;
  int comp;
  int dayCounter;
} intRateStruct;

typedef struct
{
  dataType forward;
  dataType compounding;
  dataType frequency;
  intRateStruct intRate;
  bondsDateStruct refDate;
  bondsDateStruct calDate;
  int dayCounter;
} bondsYieldTermStruct;

typedef struct
{
  bondsDateStruct paymentDate;
  bondsDateStruct accrualStartDate;
  bondsDateStruct accrualEndDate;
  dataType amount;  
} couponStruct;

typedef struct
{
  couponStruct* legs;
  intRateStruct intRate;
  int nominal;
  int dayCounter;
} cashFlowsStruct;

typedef struct
{
  dataType* dirtyPrice;
  dataType* accruedAmountCurrDate;
  dataType* cleanPrice;
  dataType* bondForwardVal;
} resultsStruct;

typedef struct
{
  bondsYieldTermStruct* discountCurve;
  bondsYieldTermStruct* repoCurve;
  bondsDateStruct* currDate;
  bondsDateStruct* maturityDate;
  dataType* bondCleanPrice;
  bondStruct* bond;
  dataType* dummyStrike;
} inArgsStruct;

typedef struct
{
  dataType npv;
  int dayCounter;
  int comp;
  dataType freq;
  bool includecurrDateFlows;
  bondsDateStruct currDate;
  bondsDateStruct npvDate;

} irrFinderStruct;

typedef struct
{
  dataType root_;
  dataType xMin_;
  dataType xMax_;
  dataType fxMin_;
  dataType fxMax_;
  int maxEvaluations_;
  int evaluationNumber_;
  dataType lowerBound_;
  dataType upperBound_;
  bool lowerBoundEnforced_;
  bool upperBoundEnforced_;
} solverStruct;

#endif //BONDS_STRUCTS_CUH
