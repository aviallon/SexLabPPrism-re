// PrismExport addr=0x180044ca0 name=anon ghidra=FUN_180044ca0 status=clean lines=93

undefined8 * FUN_180044ca0(undefined8 *param_1,longlong param_2)

{
  int *piVar1;
  ulonglong *puVar2;
  undefined8 *puVar3;
  longlong *plVar4;
  int iVar5;
  undefined8 uVar6;
  undefined8 *puVar7;
  ulonglong uVar8;
  undefined8 *puVar9;
  
  *param_1 = spdlog::logger::vftable;
  FUN_18000fca0(param_1 + 1,(undefined8 *)(param_2 + 8));
  param_1[5] = 0;
  param_1[6] = 0;
  param_1[7] = 0;
  uVar8 = *(longlong *)(param_2 + 0x30) - *(longlong *)(param_2 + 0x28) >> 4;
  if (uVar8 != 0) {
    FUN_18002c710(param_1 + 5,uVar8);
    puVar3 = *(undefined8 **)(param_2 + 0x30);
    puVar7 = (undefined8 *)param_1[5];
    for (puVar9 = *(undefined8 **)(param_2 + 0x28); puVar9 != puVar3; puVar9 = puVar9 + 2) {
      *puVar7 = 0;
      puVar7[1] = 0;
      if (puVar9[1] != 0) {
        LOCK();
        piVar1 = (int *)(puVar9[1] + 8);
        *piVar1 = *piVar1 + 1;
        UNLOCK();
      }
      *puVar7 = *puVar9;
      puVar7[1] = puVar9[1];
      puVar7 = puVar7 + 2;
    }
    param_1[6] = puVar7;
  }
  *(undefined4 *)(param_1 + 8) = *(undefined4 *)(param_2 + 0x40);
  *(undefined4 *)((longlong)param_1 + 0x44) = *(undefined4 *)(param_2 + 0x44);
  param_1[0x10] = 0;
  puVar3 = *(undefined8 **)(param_2 + 0x80);
  if (puVar3 != (undefined8 *)0x0) {
    uVar6 = (**(code **)*puVar3)(puVar3,param_1 + 9);
    param_1[0x10] = uVar6;
  }
  param_1[0x14] = 0;
  param_1[0x15] = 0;
  param_1[0x16] = 0;
  param_1[0x17] = 0;
  param_1[0x18] = 0;
  param_1[0x19] = 0;
  param_1[0x12] = 0;
  param_1[0x13] = 0;
  *(undefined4 *)(param_1 + 0x1a) = 0xffffffff;
  *(undefined4 *)(param_1 + 0x11) = 2;
  *(undefined4 *)((longlong)param_1 + 0xd4) = 0;
  *(undefined1 *)(param_1 + 0x1b) = 0;
  param_1[0x1c] = 0;
  param_1[0x1d] = 0;
  param_1[0x1e] = 0;
  param_1[0x1f] = 0;
  puVar2 = param_1 + 0x20;
  *puVar2 = 0;
  param_1[0x21] = 0;
  param_1[0x22] = 0;
  iVar5 = _Mtx_lock(param_2 + 0x88);
  if (iVar5 == 0) {
    if (*(int *)(param_2 + 0xd4) != 0x7fffffff) {
      LOCK();
      *(undefined1 *)(param_1 + 0x1b) = *(undefined1 *)(param_2 + 0xd8);
      UNLOCK();
      param_1[0x1c] = *(undefined8 *)(param_2 + 0xe0);
      param_1[0x1d] = *(undefined8 *)(param_2 + 0xe8);
      param_1[0x1e] = *(undefined8 *)(param_2 + 0xf0);
      param_1[0x1f] = *(undefined8 *)(param_2 + 0xf8);
      if (puVar2 != (ulonglong *)(param_2 + 0x100)) {
        plVar4 = *(longlong **)(param_2 + 0x100);
        FUN_18003a830(puVar2,plVar4,*(longlong *)(param_2 + 0x108) - (longlong)plVar4 >> 7);
      }
      _Mtx_unlock(param_2 + 0x88);
      return param_1;
    }
    *(undefined4 *)(param_2 + 0xd4) = 0x7ffffffe;
                    /* WARNING: Subroutine does not return */
    std::_Throw_Cpp_error(6);
  }
                    /* WARNING: Subroutine does not return */
  std::_Throw_Cpp_error(5);
}

