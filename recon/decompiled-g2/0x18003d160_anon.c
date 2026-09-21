// PrismExport addr=0x18003d160 name=anon ghidra=FUN_18003d160 status=clean lines=103

longlong * FUN_18003d160(float *param_1,longlong *param_2,undefined8 *param_3)

{
  undefined8 *puVar1;
  size_t _Size;
  longlong lVar2;
  int iVar3;
  undefined8 *puVar4;
  ulonglong uVar5;
  undefined8 *_Buf1;
  undefined8 *puVar6;
  undefined8 *puVar7;
  ulonglong uVar8;
  float fVar9;
  undefined8 *local_58;
  longlong local_50;
  
  puVar7 = param_3;
  if (0xf < (ulonglong)param_3[3]) {
    puVar7 = (undefined8 *)*param_3;
  }
  uVar8 = 0xcbf29ce484222325;
  uVar5 = 0;
  if (param_3[2] != 0) {
    do {
      uVar8 = (uVar8 ^ *(byte *)((longlong)puVar7 + uVar5)) * 0x100000001b3;
      uVar5 = uVar5 + 1;
    } while (uVar5 < (ulonglong)param_3[2]);
  }
  FUN_18003aca0((longlong)param_1,(longlong *)&local_58,param_3,uVar8);
  if (local_50 != 0) {
    *param_2 = local_50;
    *(undefined1 *)(param_2 + 1) = 0;
    return param_2;
  }
  if (*(longlong *)(param_1 + 4) == 0x3ffffffffffffff) {
                    /* WARNING: Subroutine does not return */
    std::_Xlength_error("unordered_map/set too long");
  }
  puVar4 = (undefined8 *)FUN_180050418(0x40);
  FUN_18000fca0(puVar4 + 2,param_3);
  puVar4[6] = 0;
  puVar4[7] = 0;
  fVar9 = (float)(*(longlong *)(param_1 + 4) + 1) / (float)*(ulonglong *)(param_1 + 0xe);
  puVar7 = local_58;
  if (*param_1 <= fVar9 && fVar9 != *param_1) {
    FUN_180047500(param_1);
    puVar6 = *(undefined8 **)
              (*(longlong *)(param_1 + 6) + 8 + (uVar8 & *(ulonglong *)(param_1 + 0xc)) * 0x10);
    puVar7 = *(undefined8 **)(param_1 + 2);
    if (puVar6 != *(undefined8 **)(param_1 + 2)) {
      puVar1 = *(undefined8 **)
                (*(longlong *)(param_1 + 6) + (uVar8 & *(ulonglong *)(param_1 + 0xc)) * 0x10);
      _Size = puVar4[4];
      puVar7 = puVar6;
      while( true ) {
        puVar6 = puVar7 + 2;
        if (0xf < (ulonglong)puVar7[5]) {
          puVar6 = (undefined8 *)*puVar6;
        }
        if ((ulonglong)puVar4[5] < 0x10) {
          _Buf1 = puVar4 + 2;
        }
        else {
          _Buf1 = (undefined8 *)puVar4[2];
        }
        if ((_Size == puVar7[4]) &&
           ((_Size == 0 || (iVar3 = memcmp(_Buf1,puVar6,_Size), iVar3 == 0)))) break;
        if (puVar7 == puVar1) goto LAB_18003d32b;
        puVar7 = (undefined8 *)puVar7[1];
      }
      puVar7 = (undefined8 *)*puVar7;
    }
  }
LAB_18003d32b:
  puVar6 = (undefined8 *)puVar7[1];
  *(longlong *)(param_1 + 4) = *(longlong *)(param_1 + 4) + 1;
  *puVar4 = puVar7;
  puVar4[1] = puVar6;
  *puVar6 = puVar4;
  puVar7[1] = puVar4;
  lVar2 = *(longlong *)(param_1 + 6);
  uVar8 = *(ulonglong *)(param_1 + 0xc) & uVar8;
  puVar1 = *(undefined8 **)(lVar2 + uVar8 * 0x10);
  if (puVar1 == *(undefined8 **)(param_1 + 2)) {
    *(undefined8 **)(lVar2 + uVar8 * 0x10) = puVar4;
  }
  else {
    if (puVar1 == puVar7) {
      *(undefined8 **)(lVar2 + uVar8 * 0x10) = puVar4;
      goto LAB_18003d375;
    }
    if (*(undefined8 **)(lVar2 + 8 + uVar8 * 0x10) != puVar6) goto LAB_18003d375;
  }
  *(undefined8 **)(lVar2 + 8 + uVar8 * 0x10) = puVar4;
LAB_18003d375:
  *param_2 = (longlong)puVar4;
  *(undefined1 *)(param_2 + 1) = 1;
  return param_2;
}

