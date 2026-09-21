// PrismExport addr=0x180013a20 name=anon ghidra=FUN_180013a20 status=clean lines=215

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

void FUN_180013a20(longlong param_1,longlong *param_2)

{
  longlong *plVar1;
  longlong *plVar2;
  int *_Memory;
  int iVar3;
  longlong lVar4;
  longlong lVar5;
  longlong *plVar6;
  longlong lVar7;
  longlong *plVar8;
  ulonglong uVar9;
  longlong *plVar10;
  longlong lVar11;
  undefined1 auStack_f8 [32];
  longlong *local_d8;
  longlong local_c8 [7];
  longlong *local_90;
  longlong local_88 [7];
  longlong *local_50;
  ulonglong local_48;
  
  local_48 = DAT_180095900 ^ (ulonglong)auStack_f8;
  local_d8 = param_2;
LAB_180013a70:
  if ((*param_2 != 0) && ((*(uint *)(*param_2 + 4) & 1) != 0)) goto LAB_180013eb3;
  local_50 = (longlong *)0x0;
  iVar3 = _Mtx_lock(param_1);
  if (iVar3 != 0) {
                    /* WARNING: Subroutine does not return */
    std::_Throw_Cpp_error(5);
  }
  if (*(int *)(param_1 + 0x4c) == 0x7fffffff) {
    *(undefined4 *)(param_1 + 0x4c) = 0x7ffffffe;
                    /* WARNING: Subroutine does not return */
    std::_Throw_Cpp_error(6);
  }
  if (*(longlong *)(param_1 + 0x98) == *(longlong *)(param_1 + 0xa0)) {
    while (((*param_2 == 0 || ((*(uint *)(*param_2 + 4) & 1) == 0)) &&
           (*(longlong *)(param_1 + 0x98) == *(longlong *)(param_1 + 0xa0)))) {
      _Cnd_wait(param_1 + 0x50,param_1);
    }
    if ((*param_2 != 0) && ((*(uint *)(*param_2 + 4) & 1) != 0)) {
      _Mtx_unlock(param_1);
      if (local_50 != (longlong *)0x0) {
        (**(code **)(*local_50 + 0x20))(local_50,local_50 != local_88);
        local_50 = (longlong *)0x0;
      }
LAB_180013eb3:
      _Memory = (int *)*param_2;
      if (_Memory != (int *)0x0) {
        LOCK();
        iVar3 = *_Memory;
        *_Memory = *_Memory + -1;
        UNLOCK();
        if (iVar3 == 1) {
          free(_Memory);
        }
      }
      return;
    }
  }
  plVar8 = *(longlong **)(param_1 + 0x98);
  plVar10 = plVar8;
  if (plVar8 != *(longlong **)(param_1 + 0xa0)) {
    while (plVar10 = plVar10 + 9, plVar10 != *(longlong **)(param_1 + 0xa0)) {
      if ((*plVar10 != *plVar8) && (*plVar10 < *plVar8)) {
        plVar8 = plVar10;
      }
    }
  }
  lVar7 = *plVar8;
  do {
    lVar4 = _Query_perf_frequency();
    lVar5 = _Query_perf_counter();
    if (lVar4 == 10000000) {
      lVar5 = lVar5 * 100;
    }
    else {
      if (lVar4 == 24000000) {
        lVar4 = lVar5 + SUB168(SEXT816(-0x4d0b03f86b6f730d) * SEXT816(lVar5),8);
        lVar11 = (lVar4 >> 0x18) - (lVar4 >> 0x3f);
        lVar5 = lVar5 + lVar11 * -24000000;
        lVar4 = SUB168(SEXT816(-0x4d0b03f86b6f730d) * SEXT816(lVar5 * 1000000000),8) +
                lVar5 * 1000000000;
        lVar5 = (lVar4 >> 0x18) - (lVar4 >> 0x3f);
      }
      else {
        lVar11 = lVar5 / lVar4;
        lVar5 = ((lVar5 % lVar4) * 1000000000) / lVar4;
      }
      lVar5 = lVar5 + lVar11 * 1000000000;
    }
    if ((lVar7 == lVar5) || (lVar7 < lVar5)) {
      plVar8 = *(longlong **)(param_1 + 0x98);
      plVar10 = *(longlong **)(param_1 + 0xa0);
      if (plVar8 != plVar10) {
        plVar6 = plVar8 + 9;
        if (plVar6 != plVar10) goto LAB_180013c80;
        goto LAB_180013c9a;
      }
      break;
    }
    lVar5 = lVar7 - lVar5;
    if ((lVar5 == 86400000000000) || (lVar5 < 86400000000000)) {
      uVar9 = lVar5 / 1000000;
      if ((longlong)(uVar9 * 1000000) < lVar5) {
        uVar9 = (ulonglong)((int)uVar9 + 1);
      }
    }
    else {
      uVar9 = 86400000;
    }
    iVar3 = FUN_18004fc04(param_1 + 0x50,param_1,(uint)uVar9);
  } while (iVar3 != 0);
  goto LAB_180013c38;
LAB_180013c80:
  do {
    if ((*plVar6 != *plVar8) && (*plVar6 < *plVar8)) {
      plVar8 = plVar6;
    }
    plVar6 = plVar6 + 9;
  } while (plVar6 != plVar10);
  if (plVar8 == plVar10) {
LAB_180013c38:
    _Mtx_unlock(param_1);
    if (local_50 != (longlong *)0x0) {
      (**(code **)(*local_50 + 0x20))(local_50,local_50 != local_88);
    }
    goto LAB_180013a70;
  }
LAB_180013c9a:
  lVar7 = _Query_perf_frequency();
  lVar4 = _Query_perf_counter();
  if (lVar7 == 10000000) {
    lVar4 = lVar4 * 100;
  }
  else {
    if (lVar7 == 24000000) {
      lVar7 = lVar4 + SUB168(SEXT816(-0x4d0b03f86b6f730d) * SEXT816(lVar4),8);
      lVar5 = (lVar7 >> 0x18) - (lVar7 >> 0x3f);
      lVar4 = lVar4 + lVar5 * -24000000;
      lVar7 = SUB168(SEXT816(-0x4d0b03f86b6f730d) * SEXT816(lVar4 * 1000000000),8) +
              lVar4 * 1000000000;
      lVar4 = (lVar7 >> 0x18) - (lVar7 >> 0x3f);
    }
    else {
      lVar5 = lVar4 / lVar7;
      lVar4 = ((lVar4 % lVar7) * 1000000000) / lVar7;
    }
    lVar4 = lVar4 + lVar5 * 1000000000;
  }
  if ((*plVar8 != lVar4) && (lVar4 <= *plVar8)) goto LAB_180013c38;
  if (local_88 != plVar8 + 1) {
    if (local_50 != (longlong *)0x0) {
      (**(code **)(*local_50 + 0x20))(local_50,local_50 != local_88);
      local_50 = (longlong *)0x0;
    }
    FUN_180018cc0((longlong)local_88,plVar8 + 1);
  }
  plVar6 = *(longlong **)(param_1 + 0xa0);
  plVar10 = plVar8 + 9;
  if (plVar10 != plVar6) {
    plVar8 = plVar8 + 1;
    do {
      plVar8[-1] = *plVar10;
      plVar1 = plVar8 + 9;
      if (plVar8 != plVar1) {
        plVar2 = (longlong *)plVar8[7];
        if (plVar2 != (longlong *)0x0) {
          (**(code **)(*plVar2 + 0x20))(plVar2,plVar2 != plVar8);
          plVar8[7] = 0;
        }
        FUN_180018cc0((longlong)plVar8,plVar1);
      }
      plVar10 = plVar10 + 9;
      plVar8 = plVar1;
    } while (plVar10 != plVar6);
  }
  lVar7 = *(longlong *)(param_1 + 0xa0);
  plVar8 = *(longlong **)(lVar7 + -8);
  if (plVar8 != (longlong *)0x0) {
    (**(code **)(*plVar8 + 0x20))(plVar8,plVar8 != (longlong *)(lVar7 + -0x40));
    *(undefined8 *)(lVar7 + -8) = 0;
  }
  *(longlong *)(param_1 + 0xa0) = *(longlong *)(param_1 + 0xa0) + -0x48;
  _Mtx_unlock(param_1);
  lVar7 = FUN_1800382c0();
  if ((lVar7 == 0) || (local_50 == (longlong *)0x0)) goto LAB_180013e6f;
  local_90 = (longlong *)0x0;
  if (local_50 == local_88) {
    local_90 = (longlong *)(**(code **)(*local_50 + 8))(local_50,local_c8);
    if (local_50 != (longlong *)0x0) {
      (**(code **)(*local_50 + 0x20))(local_50,local_50 != local_88);
      goto LAB_180013e5b;
    }
  }
  else {
    local_90 = local_50;
LAB_180013e5b:
    local_50 = (longlong *)0x0;
  }
  FUN_180037df0(lVar7,local_c8);
LAB_180013e6f:
  if (local_50 != (longlong *)0x0) {
    (**(code **)(*local_50 + 0x20))(local_50,local_50 != local_88);
  }
  goto LAB_180013a70;
}

