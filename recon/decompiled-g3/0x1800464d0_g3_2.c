// PrismExport addr=0x1800464d0 name=g3_2 ghidra=FUN_1800464d0 status=clean lines=90

undefined8 * FUN_1800464d0(longlong *param_1,undefined8 *param_2,undefined8 *param_3)

{
  longlong *plVar1;
  undefined1 uVar2;
  _Locimp *p_Var3;
  facet *pfVar4;
  undefined8 *puVar5;
  int iVar6;
  undefined1 *puVar7;
  longlong lVar8;
  undefined1 *puVar9;
  undefined1 local_58 [8];
  _Locimp *local_50;
  longlong local_48;
  longlong local_40;
  
  if (*(char *)(param_1[1] + 10) == '\x01') {
    if ((ulonglong)param_3[3] < param_3[2] + 1) {
      (**(code **)*param_3)(param_3);
    }
    *(undefined1 *)(param_3[1] + param_3[2]) = 0x2b;
  }
  else {
    if (*(char *)(param_1[1] + 10) != '\x03') goto LAB_180046540;
    if ((ulonglong)param_3[3] < param_3[2] + 1) {
      (**(code **)*param_3)(param_3);
    }
    *(undefined1 *)(param_3[1] + param_3[2]) = 0x20;
  }
  param_3[2] = param_3[2] + 1;
LAB_180046540:
  puVar7 = *(undefined1 **)param_1[3];
  puVar9 = puVar7 + ((undefined8 *)param_1[3])[1];
  for (; puVar7 != puVar9; puVar7 = puVar7 + 1) {
    uVar2 = *puVar7;
    if ((ulonglong)param_3[3] < param_3[2] + 1) {
      (**(code **)*param_3)(param_3);
    }
    *(undefined1 *)(param_3[2] + param_3[1]) = uVar2;
    param_3[2] = param_3[2] + 1;
  }
  if (*(char *)param_1[4] != '\0') {
    if ((*(int *)param_1[5] < *(int *)param_1[1]) &&
       (iVar6 = *(int *)param_1[1] - *(int *)param_1[5], lVar8 = (longlong)iVar6, 0 < iVar6)) {
      do {
        if ((ulonglong)param_3[3] < param_3[2] + 1) {
          (**(code **)*param_3)(param_3);
        }
        *(undefined1 *)(param_3[2] + param_3[1]) = 0x30;
        param_3[2] = param_3[2] + 1;
        lVar8 = lVar8 + -1;
      } while (0 < lVar8);
    }
  }
  iVar6 = *(int *)param_1[6];
  if (iVar6 < 1) {
    FUN_18003e380(param_2,*(undefined1 **)param_1[7],(undefined1 *)*param_1,param_3);
  }
  else {
    if (*(longlong *)param_1[9] == 0) {
      p_Var3 = std::locale::_Init(true);
      local_50 = p_Var3;
    }
    else {
      p_Var3 = *(_Locimp **)(*(longlong *)param_1[9] + 8);
      local_50 = p_Var3;
      (**(code **)(*(longlong *)p_Var3 + 8))(p_Var3);
    }
    pfVar4 = FUN_18000f1a0((longlong)local_58);
    uVar2 = (**(code **)(*(longlong *)pfVar4 + 0x20))(pfVar4);
    plVar1 = (longlong *)param_1[8];
    local_48 = (longlong)plVar1;
    if (0xf < (ulonglong)plVar1[3]) {
      local_48 = *plVar1;
    }
    local_40 = plVar1[2];
    FUN_180040330(param_2,*(undefined1 **)param_1[7],*param_1,&local_48,uVar2,iVar6,param_3);
    if ((p_Var3 != (_Locimp *)0x0) &&
       (puVar5 = (undefined8 *)(**(code **)(*(longlong *)p_Var3 + 0x10))(p_Var3),
       puVar5 != (undefined8 *)0x0)) {
      (**(code **)*puVar5)(puVar5,1);
      return param_2;
    }
  }
  return param_2;
}

