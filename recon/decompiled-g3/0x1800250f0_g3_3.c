// PrismExport addr=0x1800250f0 name=g3_3 ghidra=FUN_1800250f0 status=clean lines=227

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_1800250f0(undefined8 param_1,undefined8 param_2,longlong param_3,undefined8 param_4)

{
  ulonglong uVar1;
  char *pcVar2;
  size_t sVar3;
  int iVar4;
  char *****pppppcVar5;
  char ****ppppcVar6;
  ulonglong uVar7;
  undefined8 **_Src;
  undefined8 **_Src_00;
  undefined1 auStackY_f8 [32];
  byte local_c8;
  char ****local_b8;
  undefined8 uStack_b0;
  ulonglong local_a8;
  ulonglong uStack_a0;
  char ****local_98;
  undefined8 uStack_90;
  ulonglong local_88;
  ulonglong uStack_80;
  char ***local_78;
  char ***pppcStack_70;
  char ***local_68;
  char ***pppcStack_60;
  char ***local_58;
  char ***pppcStack_50;
  char ***local_48;
  char ***pppcStack_40;
  ulonglong local_38;
  
  local_38 = DAT_180095900 ^ (ulonglong)auStackY_f8;
  _anonymous_namespace___ApplyPresentation(param_1,param_2,param_3,param_4);
  iVar4 = _Mtx_lock(&DAT_180095130);
  sVar3 = DAT_18009c1a0;
  if (iVar4 != 0) {
                    /* WARNING: Subroutine does not return */
    std::_Throw_Cpp_error(5);
  }
  if (DAT_18009517c != 0x7fffffff) {
    if (0x7fffffffffffffff - DAT_18009c1a0 < 0x11) {
                    /* WARNING: Subroutine does not return */
      FUN_180019860();
    }
    _Src_00 = &DAT_18009c190;
    _Src = &DAT_18009c190;
    if (0xf < DAT_18009c1a8) {
      _Src = (undefined8 **)DAT_18009c190;
    }
    local_b8 = (char ****)0x0;
    uStack_b0 = 0;
    local_a8 = 0;
    uStack_a0 = 0;
    uVar1 = DAT_18009c1a0 + 0x11;
    uVar7 = 0xf;
    pppppcVar5 = &local_b8;
    if (0xf < uVar1) {
      uVar7 = uVar1 | 0xf;
      if (uVar7 < 0x8000000000000000) {
        if (uVar7 < 0x16) {
          uVar7 = 0x16;
        }
      }
      else {
        uVar7 = 0x7fffffffffffffff;
      }
      pppppcVar5 = (char *****)FUN_1800013a0(uVar7 + 1);
      local_b8 = (char ****)pppppcVar5;
    }
    local_a8 = uVar1;
    uStack_a0 = uVar7;
    *pppppcVar5 = (char ****)s_window_slppState__18005d058._0_8_;
    pppppcVar5[1] = (char ****)s_window_slppState__18005d058._8_8_;
    *(char *)(pppppcVar5 + 2) = s_window_slppState__18005d058[0x10];
    memcpy((char *)((longlong)pppppcVar5 + 0x11),_Src,sVar3);
    *(char *)((longlong)pppppcVar5 + uVar1) = '\0';
    uVar1 = local_a8;
    if (uStack_a0 - local_a8 < 2) {
      pppppcVar5 = (char *****)FUN_180007720(&local_b8,2,(ulonglong)local_c8,&DAT_18005d050,2);
    }
    else {
      local_a8 = local_a8 + 2;
      pppppcVar5 = &local_b8;
      if (0xf < uStack_a0) {
        pppppcVar5 = (char *****)local_b8;
      }
      pcVar2 = (char *)((longlong)pppppcVar5 + uVar1);
      pcVar2[0] = ')';
      pcVar2[1] = ';';
      *(char *)((longlong)pppppcVar5 + uVar1 + 2) = '\0';
      pppppcVar5 = &local_b8;
    }
    local_78 = (char ***)0x0;
    pppcStack_70 = (char ***)0x0;
    local_68 = (char ***)0x0;
    pppcStack_60 = (char ***)0x0;
    local_78 = (char ***)*pppppcVar5;
    pppcStack_70 = (char ***)pppppcVar5[1];
    local_68 = (char ***)pppppcVar5[2];
    pppcStack_60 = (char ***)pppppcVar5[3];
    *(undefined1 *)pppppcVar5 = 0;
    pppppcVar5[2] = (char ****)0x0;
    pppppcVar5[3] = (char ****)0xf;
    _anonymous_namespace___InvokeOn(DAT_18009c1c0,&DAT_18009c1c8,&local_78);
    if ((char ****)0xf < pppcStack_60) {
      ppppcVar6 = (char ****)local_78;
      if ((0xfff < (longlong)pppcStack_60 + 1U) &&
         (ppppcVar6 = (char ****)local_78[-1],
         0x1f < (ulonglong)((longlong)local_78 + (-8 - (longlong)ppppcVar6)))) {
                    /* WARNING: Subroutine does not return */
        _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
      }
      free(ppppcVar6);
    }
    local_68 = (char ***)_DAT_18005c8e0;
    pppcStack_60 = (char ***)_UNK_18005c8e8;
    local_78 = (char ***)((ulonglong)local_78 & 0xffffffffffffff00);
    if (0xf < uStack_a0) {
      pppppcVar5 = (char *****)local_b8;
      if ((0xfff < uStack_a0 + 1) &&
         (pppppcVar5 = (char *****)local_b8[-1],
         (char *)0x1f < (char *)((longlong)local_b8 + (-8 - (longlong)pppppcVar5)))) {
                    /* WARNING: Subroutine does not return */
        _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
      }
      free(pppppcVar5);
    }
    sVar3 = DAT_18009c1a0;
    if (0x7fffffffffffffff - DAT_18009c1a0 < 0x12) {
                    /* WARNING: Subroutine does not return */
      FUN_180019860();
    }
    if (0xf < DAT_18009c1a8) {
      _Src_00 = (undefined8 **)DAT_18009c190;
    }
    local_98 = (char ****)0x0;
    uStack_90 = 0;
    local_88 = 0;
    uStack_80 = 0;
    uVar1 = DAT_18009c1a0 + 0x12;
    uVar7 = 0xf;
    pppppcVar5 = &local_98;
    if (0xf < uVar1) {
      uVar7 = uVar1 | 0xf;
      if (uVar7 < 0x8000000000000000) {
        if (uVar7 < 0x16) {
          uVar7 = 0x16;
        }
      }
      else {
        uVar7 = 0x7fffffffffffffff;
      }
      pppppcVar5 = (char *****)FUN_1800013a0(uVar7 + 1);
      local_98 = (char ****)pppppcVar5;
    }
    local_88 = uVar1;
    uStack_80 = uVar7;
    *pppppcVar5 = (char ****)s_window_slppVitals__18005d070._0_8_;
    pppppcVar5[1] = (char ****)s_window_slppVitals__18005d070._8_8_;
    *(undefined2 *)(pppppcVar5 + 2) = s_window_slppVitals__18005d070._16_2_;
    memcpy((char *)((longlong)pppppcVar5 + 0x12),_Src_00,sVar3);
    *(char *)((longlong)pppppcVar5 + uVar1) = '\0';
    uVar1 = local_88;
    if (uStack_80 - local_88 < 2) {
      pppppcVar5 = (char *****)FUN_180007720(&local_98,2,(ulonglong)local_c8,&DAT_18005d050,2);
    }
    else {
      local_88 = local_88 + 2;
      pppppcVar5 = &local_98;
      if (0xf < uStack_80) {
        pppppcVar5 = (char *****)local_98;
      }
      pcVar2 = (char *)((longlong)pppppcVar5 + uVar1);
      pcVar2[0] = ')';
      pcVar2[1] = ';';
      *(char *)((longlong)pppppcVar5 + uVar1 + 2) = '\0';
      pppppcVar5 = &local_98;
    }
    local_58 = (char ***)0x0;
    pppcStack_50 = (char ***)0x0;
    local_48 = (char ***)0x0;
    pppcStack_40 = (char ***)0x0;
    local_58 = (char ***)*pppppcVar5;
    pppcStack_50 = (char ***)pppppcVar5[1];
    local_48 = (char ***)pppppcVar5[2];
    pppcStack_40 = (char ***)pppppcVar5[3];
    *(undefined1 *)pppppcVar5 = 0;
    pppppcVar5[2] = (char ****)0x0;
    pppppcVar5[3] = (char ****)0xf;
    _anonymous_namespace___InvokeOn(DAT_18009c1c0,&DAT_18009c1c8,&local_58);
    if ((char ****)0xf < pppcStack_40) {
      ppppcVar6 = (char ****)local_58;
      if ((0xfff < (longlong)pppcStack_40 + 1U) &&
         (ppppcVar6 = (char ****)local_58[-1],
         0x1f < (ulonglong)((longlong)local_58 + (-8 - (longlong)ppppcVar6)))) {
                    /* WARNING: Subroutine does not return */
        _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
      }
      free(ppppcVar6);
    }
    local_48 = (char ***)_DAT_18005c8e0;
    pppcStack_40 = (char ***)_UNK_18005c8e8;
    local_58 = (char ***)((ulonglong)local_58 & 0xffffffffffffff00);
    if (0xf < uStack_80) {
      pppppcVar5 = (char *****)local_98;
      if ((0xfff < uStack_80 + 1) &&
         (pppppcVar5 = (char *****)local_98[-1],
         (char *)0x1f < (char *)((longlong)local_98 + (-8 - (longlong)pppppcVar5)))) {
                    /* WARNING: Subroutine does not return */
        _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
      }
      free(pppppcVar5);
    }
    _Mtx_unlock(&DAT_180095130);
    return;
  }
  DAT_18009517c = 0x7ffffffe;
                    /* WARNING: Subroutine does not return */
  std::_Throw_Cpp_error(6);
}

