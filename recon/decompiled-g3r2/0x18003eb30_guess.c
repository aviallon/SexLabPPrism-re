// PrismExport addr=0x18003eb30 name=guess ghidra=FUN_18003eb30 status=clean lines=88

void FUN_18003eb30(undefined8 *param_1,undefined8 *param_2,int param_3,int *param_4,char param_5,
                  undefined8 *param_6)

{
  undefined1 uVar1;
  int iVar2;
  undefined1 *puVar3;
  int iVar4;
  char cVar5;
  undefined1 *puVar6;
  undefined1 *puVar7;
  ulonglong uVar8;
  int iVar9;
  
  iVar2 = *param_4;
  cVar5 = *(char *)((longlong)param_4 + 9);
  if (*(char *)((longlong)param_4 + 9) == '\0') {
    cVar5 = param_5;
  }
  iVar4 = 0;
  iVar9 = 0;
  if (param_3 < iVar2) {
    if (cVar5 == '\x01') {
      uVar8 = (ulonglong)*(byte *)((longlong)param_4 + 0xe);
      iVar9 = iVar2 - param_3;
      goto LAB_18003ebfb;
    }
    iVar9 = 0;
    if (cVar5 == '\x02') {
      iVar4 = iVar2 - param_3;
    }
    else {
      iVar4 = 0;
      if (cVar5 == '\x03') {
        iVar4 = (iVar2 - param_3) / 2;
        iVar9 = (iVar2 - iVar4) - param_3;
      }
    }
  }
  uVar8 = (ulonglong)*(byte *)((longlong)param_4 + 0xe);
  puVar6 = (undefined1 *)((longlong)param_4 + 0xf);
  if (0 < iVar4) {
    puVar3 = puVar6;
    do {
      for (; puVar3 != puVar6 + uVar8; puVar3 = puVar3 + 1) {
        uVar1 = *puVar3;
        if ((ulonglong)param_2[3] < param_2[2] + 1) {
          (**(code **)*param_2)(param_2);
        }
        *(undefined1 *)(param_2[2] + param_2[1]) = uVar1;
        param_2[2] = param_2[2] + 1;
      }
      iVar4 = iVar4 + -1;
      puVar3 = puVar6;
    } while (0 < iVar4);
  }
LAB_18003ebfb:
  puVar7 = (undefined1 *)((longlong)param_4 + 0xf);
  puVar3 = (undefined1 *)param_6[2];
  for (puVar6 = (undefined1 *)*param_6; puVar6 != puVar3; puVar6 = puVar6 + 1) {
    uVar1 = *puVar6;
    if ((ulonglong)param_2[3] < param_2[2] + 1) {
      (**(code **)*param_2)(param_2);
    }
    *(undefined1 *)(param_2[2] + param_2[1]) = uVar1;
    param_2[2] = param_2[2] + 1;
  }
  if (0 < iVar9) {
    puVar6 = puVar7;
    do {
      for (; puVar6 != puVar7 + uVar8; puVar6 = puVar6 + 1) {
        uVar1 = *puVar6;
        if ((ulonglong)param_2[3] < param_2[2] + 1) {
          (**(code **)*param_2)(param_2);
        }
        *(undefined1 *)(param_2[2] + param_2[1]) = uVar1;
        param_2[2] = param_2[2] + 1;
      }
      iVar9 = iVar9 + -1;
      puVar6 = puVar7;
    } while (0 < iVar9);
  }
  *param_1 = param_2;
  return;
}

