// PrismExport addr=0x18004a450 name=g3_9 ghidra=FUN_18004a450 status=clean lines=70

void FUN_18004a450(longlong param_1,longlong param_2,undefined8 param_3,longlong *param_4)

{
  int iVar1;
  longlong lVar2;
  ulonglong uVar3;
  ulonglong uVar4;
  uint uVar5;
  uint uVar6;
  
  lVar2 = *(longlong *)(param_2 + 0x18) - *(longlong *)(param_1 + 0x18);
  *(longlong *)(param_1 + 0x18) = *(longlong *)(param_2 + 0x18);
  iVar1 = 1;
  if (lVar2 == 0) {
    uVar4 = 0;
  }
  else {
    if (lVar2 < 0) {
      lVar2 = 0;
    }
    uVar4 = lVar2 / 10000;
    for (uVar3 = uVar4; 9 < uVar3; uVar3 = uVar3 / 10000) {
      if (uVar3 < 100) {
        iVar1 = iVar1 + 1;
        break;
      }
      if (uVar3 < 1000) {
        iVar1 = iVar1 + 2;
        break;
      }
      if (uVar3 < 10000) {
        iVar1 = iVar1 + 3;
        break;
      }
      iVar1 = iVar1 + 4;
    }
  }
  uVar5 = *(int *)(param_1 + 8) - iVar1;
  if (0 < (int)uVar5) {
    if (*(int *)(param_1 + 0x10) == 0) {
      FUN_18003a630(param_4,"                                                                ",
                    (longlong)(int)uVar5);
      uVar5 = 0;
    }
    else if (*(int *)(param_1 + 0x10) == 2) {
      FUN_18003a630(param_4,"                                                                ",
                    (longlong)((int)uVar5 / 2));
      uVar5 = (uVar5 & 1) + (int)uVar5 / 2;
    }
  }
  FUN_1800407b0(uVar4,param_4);
  if ((int)uVar5 < 0) {
    if (*(char *)(param_1 + 0x14) != '\0') {
      uVar5 = (int)param_4[2] + uVar5;
      uVar6 = 0;
      if (-1 < (int)uVar5) {
        uVar6 = uVar5;
      }
      FUN_18001e280(param_4,(longlong)(int)uVar6,'\0');
    }
  }
  else {
    FUN_18003a630(param_4,"                                                                ",
                  (longlong)(int)uVar5);
  }
  return;
}

