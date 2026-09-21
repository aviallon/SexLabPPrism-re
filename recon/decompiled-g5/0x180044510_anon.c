// PrismExport addr=0x180044510 name=anon ghidra=FUN_180044510 status=clean lines=59

void FUN_180044510(uint param_1,longlong *param_2)

{
  ulonglong uVar1;
  longlong *plVar2;
  char cVar3;
  char cVar4;
  byte local_res8;
  
  if (999 < param_1) {
    FUN_1800405d0(param_1,param_2);
    return;
  }
  uVar1 = param_2[2];
  cVar3 = (char)(param_1 / 100) + '0';
  if (uVar1 < (ulonglong)param_2[3]) {
    param_2[2] = uVar1 + 1;
    plVar2 = param_2;
    if (0xf < (ulonglong)param_2[3]) {
      plVar2 = (longlong *)*param_2;
    }
    *(char *)(uVar1 + (longlong)plVar2) = cVar3;
    *(undefined1 *)(uVar1 + 1 + (longlong)plVar2) = 0;
  }
  else {
    FUN_180007dd0(param_2,1,(ulonglong)local_res8,cVar3);
  }
  uVar1 = param_2[2];
  cVar3 = (char)((param_1 % 100) / 10);
  cVar4 = cVar3 + '0';
  if (uVar1 < (ulonglong)param_2[3]) {
    param_2[2] = uVar1 + 1;
    plVar2 = param_2;
    if (0xf < (ulonglong)param_2[3]) {
      plVar2 = (longlong *)*param_2;
    }
    *(char *)(uVar1 + (longlong)plVar2) = cVar4;
    *(undefined1 *)(uVar1 + 1 + (longlong)plVar2) = 0;
  }
  else {
    FUN_180007dd0(param_2,1,(ulonglong)local_res8,cVar4);
  }
  uVar1 = param_2[2];
  cVar3 = (char)(param_1 % 100) + cVar3 * -10 + '0';
  if (uVar1 < (ulonglong)param_2[3]) {
    param_2[2] = uVar1 + 1;
    if (0xf < (ulonglong)param_2[3]) {
      param_2 = (longlong *)*param_2;
    }
    *(char *)(uVar1 + (longlong)param_2) = cVar3;
    *(undefined1 *)(uVar1 + 1 + (longlong)param_2) = 0;
    return;
  }
  FUN_180007dd0(param_2,1,(ulonglong)local_res8,cVar3);
  return;
}

