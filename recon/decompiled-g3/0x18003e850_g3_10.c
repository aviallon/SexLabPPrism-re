// PrismExport addr=0x18003e850 name=g3_10 ghidra=FUN_18003e850 status=clean lines=85

longlong *
FUN_18003e850(longlong *param_1,undefined8 *param_2,int param_3,int *param_4,char param_5,
             longlong *param_6)

{
  undefined1 *puVar1;
  undefined1 uVar2;
  int iVar3;
  undefined8 *puVar4;
  undefined1 *puVar5;
  int iVar6;
  ulonglong uVar7;
  longlong *plVar8;
  char cVar9;
  int iVar10;
  undefined1 *puVar11;
  undefined8 local_res8;
  
  iVar3 = *param_4;
  cVar9 = *(char *)((longlong)param_4 + 9);
  if (*(char *)((longlong)param_4 + 9) == '\0') {
    cVar9 = param_5;
  }
  iVar6 = 0;
  iVar10 = 0;
  if (param_3 < iVar3) {
    if (cVar9 != '\x01') {
      iVar10 = 0;
      if (cVar9 == '\x02') {
        iVar6 = iVar3 - param_3;
      }
      else {
        iVar6 = 0;
        if (cVar9 == '\x03') {
          iVar6 = (iVar3 - param_3) / 2;
          iVar10 = (iVar3 - iVar6) - param_3;
        }
      }
      goto LAB_18003e8b9;
    }
    uVar7 = (ulonglong)*(byte *)((longlong)param_4 + 0xe);
    iVar10 = iVar3 - param_3;
  }
  else {
LAB_18003e8b9:
    uVar7 = (ulonglong)*(byte *)((longlong)param_4 + 0xe);
    puVar1 = (undefined1 *)((longlong)param_4 + 0xf);
    if (0 < iVar6) {
      puVar11 = puVar1 + uVar7;
      puVar5 = puVar1;
      do {
        for (; puVar5 != puVar11; puVar5 = puVar5 + 1) {
          uVar2 = *puVar5;
          if ((ulonglong)param_2[3] < param_2[2] + 1) {
            (**(code **)*param_2)(param_2);
          }
          *(undefined1 *)(param_2[1] + param_2[2]) = uVar2;
          param_2[2] = param_2[2] + 1;
        }
        iVar6 = iVar6 + -1;
        puVar5 = puVar1;
      } while (0 < iVar6);
      goto LAB_18003e93b;
    }
  }
  puVar11 = (undefined1 *)((longlong)param_4 + uVar7 + 0xf);
LAB_18003e93b:
  plVar8 = FUN_1800466d0(param_6,&local_res8,param_2);
  puVar4 = (undefined8 *)*plVar8;
  for (; puVar1 = (undefined1 *)((longlong)param_4 + 0xf), 0 < iVar10; iVar10 = iVar10 + -1) {
    for (; puVar1 != puVar11; puVar1 = puVar1 + 1) {
      uVar2 = *puVar1;
      if ((ulonglong)puVar4[3] < puVar4[2] + 1) {
        (**(code **)*puVar4)(puVar4);
      }
      *(undefined1 *)(puVar4[1] + puVar4[2]) = uVar2;
      puVar4[2] = puVar4[2] + 1;
    }
  }
  *param_1 = (longlong)puVar4;
  return param_1;
}

