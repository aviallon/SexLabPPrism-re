// PrismExport addr=0x18002dad0 name=g3_7 ghidra=FUN_18002dad0 status=clean lines=96

longlong * FUN_18002dad0(longlong param_1,longlong *param_2,longlong *param_3)

{
  byte *pbVar1;
  undefined8 *puVar2;
  longlong lVar3;
  undefined8 *puVar4;
  undefined8 *puVar5;
  ulonglong uVar6;
  ulonglong uVar7;
  longlong *_Memory;
  longlong *plVar8;
  longlong *plVar9;
  longlong *plVar10;
  longlong *plVar11;
  
  if (param_2 == param_3) {
    return param_3;
  }
  plVar10 = param_2 + 2;
  puVar2 = *(undefined8 **)(param_1 + 8);
  lVar3 = *(longlong *)(param_1 + 0x18);
  puVar4 = (undefined8 *)param_2[1];
  if (0xf < (ulonglong)param_2[5]) {
    plVar10 = (longlong *)*plVar10;
  }
  uVar6 = 0;
  uVar7 = 0xcbf29ce484222325;
  if (param_2[4] != 0) {
    do {
      pbVar1 = (byte *)((longlong)plVar10 + uVar6);
      uVar6 = uVar6 + 1;
      uVar7 = (uVar7 ^ *pbVar1) * 0x100000001b3;
    } while (uVar6 < (ulonglong)param_2[4]);
  }
  plVar11 = (longlong *)((*(ulonglong *)(param_1 + 0x30) & uVar7) * 0x10 + lVar3);
  plVar10 = (longlong *)*plVar11;
  plVar9 = (longlong *)plVar11[1];
  _Memory = param_2;
  do {
    plVar8 = (longlong *)*_Memory;
    FUN_180019420(_Memory + 2);
    free(_Memory);
    *(longlong *)(param_1 + 0x10) = *(longlong *)(param_1 + 0x10) + -1;
    if (_Memory == plVar9) {
      puVar5 = puVar4;
      if (plVar10 == param_2) {
        *plVar11 = (longlong)puVar2;
        puVar5 = puVar2;
      }
      plVar11[1] = (longlong)puVar5;
      while (plVar8 != param_3) {
        plVar10 = plVar8 + 2;
        if (0xf < (ulonglong)plVar8[5]) {
          plVar10 = (longlong *)*plVar10;
        }
        uVar7 = 0;
        uVar6 = 0xcbf29ce484222325;
        if (plVar8[4] != 0) {
          do {
            pbVar1 = (byte *)(uVar7 + (longlong)plVar10);
            uVar7 = uVar7 + 1;
            uVar6 = (uVar6 ^ *pbVar1) * 0x100000001b3;
          } while (uVar7 < (ulonglong)plVar8[4]);
        }
        plVar11 = (longlong *)((*(ulonglong *)(param_1 + 0x30) & uVar6) * 0x10 + lVar3);
        plVar10 = (longlong *)plVar11[1];
        plVar9 = plVar8;
        while( true ) {
          plVar8 = (longlong *)*plVar9;
          FUN_180019420(plVar9 + 2);
          free(plVar9);
          *(longlong *)(param_1 + 0x10) = *(longlong *)(param_1 + 0x10) + -1;
          if (plVar9 == plVar10) break;
          plVar9 = plVar8;
          if (plVar8 == param_3) goto LAB_18002dbca;
        }
        *plVar11 = (longlong)puVar2;
        plVar11[1] = (longlong)puVar2;
      }
      goto LAB_18002dbce;
    }
    _Memory = plVar8;
  } while (plVar8 != param_3);
  if (plVar10 == param_2) {
LAB_18002dbca:
    *plVar11 = (longlong)plVar8;
  }
LAB_18002dbce:
  *puVar4 = plVar8;
  plVar8[1] = (longlong)puVar4;
  return param_3;
}

