// PrismExport addr=0x18004fb6c name=g3_18 ghidra=FUN_18004fb6c status=clean lines=35

undefined4 FUN_18004fb6c(longlong param_1,longlong param_2,uint param_3,char param_4)

{
  BOOL BVar1;
  DWORD DVar2;
  ULONGLONG UVar3;
  ULONGLONG UVar4;
  undefined4 uVar5;
  
  UVar3 = 0;
  if (param_4 != '\0') {
    UVar3 = GetTickCount64();
  }
  *(int *)(param_2 + 0x4c) = *(int *)(param_2 + 0x4c) + -1;
  *(undefined4 *)(param_2 + 0x48) = 0xffffffff;
  BVar1 = SleepConditionVariableSRW
                    ((PCONDITION_VARIABLE)(param_1 + 8),(PSRWLOCK)(param_2 + 0x10),param_3,0);
  uVar5 = 0;
  if (BVar1 == 0) {
    if (param_4 != '\0') {
      UVar4 = GetTickCount64();
      uVar5 = 0;
      if (UVar4 - UVar3 < (ulonglong)param_3) goto LAB_18004fbdc;
    }
    uVar5 = 2;
  }
LAB_18004fbdc:
  DVar2 = GetCurrentThreadId();
  *(int *)(param_2 + 0x4c) = *(int *)(param_2 + 0x4c) + 1;
  *(DWORD *)(param_2 + 0x48) = DVar2;
  return uVar5;
}

