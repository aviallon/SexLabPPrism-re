// PrismExport addr=0x1800246b0 name=PublishCatalogToUi_lambda1 ghidra=_anonymous_namespace_::PublishCatalogToUi::<lambda_1>::opera status=clean lines=298

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void _anonymous_namespace___PublishCatalogToUi__<lambda_1>__opera(longlong *param_1)

{
  ulonglong _Size;
  undefined8 uVar1;
  char ***pppcVar2;
  char ***pppcVar3;
  int iVar4;
  char *****pppppcVar5;
  longlong *plVar6;
  void *pvVar7;
  char ****ppppcVar8;
  char ****ppppcVar9;
  char *****_Memory;
  char *pcVar10;
  char ****ppppcVar11;
  char ****ppppcVar12;
  char ****ppppcVar13;
  undefined1 auStackY_158 [32];
  byte local_118;
  char ***local_108 [2];
  char ****local_f8;
  longlong *plStack_f0;
  longlong *local_e8;
  char *local_e0;
  char ***local_d8;
  char ***local_c8;
  char **ppcStack_c0;
  char *local_b8;
  char ***local_a8;
  undefined *local_a0;
  char ****local_98;
  char **ppcStack_90;
  char ***local_88;
  char ***local_80;
  char *local_78;
  char ***pppcStack_70;
  void *local_68;
  undefined8 uStack_60;
  undefined8 local_58;
  ulonglong uStack_50;
  longlong local_48;
  char ***local_40;
  ulonglong local_38;
  
  local_38 = DAT_180095900 ^ (ulonglong)auStackY_158;
  ppppcVar11 = (char ****)0x0;
  local_a0 = &DAT_1800951f0;
  local_e8 = param_1;
  iVar4 = _Mtx_lock(&DAT_1800951f0);
  if (iVar4 != 0) {
                    /* WARNING: Subroutine does not return */
    std::_Throw_Cpp_error(5);
  }
  if (DAT_18009523c == 0x7fffffff) {
    DAT_18009523c = 0x7ffffffe;
                    /* WARNING: Subroutine does not return */
    std::_Throw_Cpp_error(6);
  }
  ppppcVar13 = (char ****)(DAT_18009c1e0 - DAT_18009c1d8 >> 7);
  local_e0 = (char *)0x0;
  local_d8 = (char ***)0x0;
  local_108[0] = (char ***)0x0;
  local_78 = "window.slppCatalogReset({});";
  pppcStack_70 = (char ***)0x1c;
  local_48 = 0x4000000000000000;
  local_f8 = (char ****)0x1;
  plStack_f0 = &local_48;
  uStack_60 = 0;
  local_58 = _DAT_18005c8e0;
  uStack_50 = _UNK_18005c8e8;
  local_68 = (void *)0x0;
  local_c8 = (char ***)ppppcVar13;
  local_a8 = (char ***)ppppcVar13;
  local_40 = (char ***)ppppcVar13;
  FUN_18001e160(&local_68,0x24);
  local_98 = local_f8;
  ppcStack_90 = (char **)plStack_f0;
  FUN_18000f650(&local_f8,&local_68,(longlong *)&local_78,&local_98);
  _anonymous_namespace___InvokeOn(DAT_18009c1c0,&DAT_18009c1c8,&local_68);
  if (0xf < uStack_50) {
    pvVar7 = local_68;
    if ((0xfff < uStack_50 + 1) &&
       (pvVar7 = *(void **)((longlong)local_68 + -8),
       0x1f < (ulonglong)((longlong)local_68 + (-8 - (longlong)pvVar7)))) {
                    /* WARNING: Subroutine does not return */
      _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
    }
    free(pvVar7);
  }
  local_f8 = (char ****)0x0;
  ppppcVar9 = ppppcVar11;
  if (ppppcVar13 != (char ****)0x0) {
    do {
      local_98 = (char ****)0x0;
      ppcStack_90 = (char **)0x0;
      local_88 = (char ***)0x0;
      local_80 = (char ***)0x0;
      local_98 = (char ****)FUN_1800013a0(0x20);
      uVar1 = s_window_slppCatalogChunk___18005d100._8_8_;
      local_88 = (char ***)0x19;
      local_80 = (char ***)0x1f;
      *local_98 = (char ***)s_window_slppCatalogChunk___18005d100._0_8_;
      local_98[1] = (char ***)uVar1;
      local_98[2] = (char ***)s_window_slppCatalogChunk___18005d100._16_8_;
      *(char *)(local_98 + 3) = s_window_slppCatalogChunk___18005d100[0x18];
      *(char *)((longlong)local_98 + 0x19) = '\0';
      ppppcVar12 = (char ****)((longlong)ppppcVar11 + *param_1);
      if (ppppcVar13 < (char ****)((longlong)ppppcVar11 + *param_1)) {
        ppppcVar12 = ppppcVar13;
      }
      local_108[0] = (char ***)ppppcVar12;
      if (ppppcVar11 < ppppcVar12) {
        local_48 = (longlong)ppppcVar11 << 7;
        ppppcVar8 = ppppcVar11;
        do {
          pppcVar3 = local_80;
          pppcVar2 = local_88;
          if (ppppcVar8 < ppppcVar11) {
            if (local_88 < local_80) {
              pppppcVar5 = &local_98;
              if ((char ****)0xf < local_80) {
                pppppcVar5 = (char *****)local_98;
              }
              pcVar10 = (char *)((longlong)pppppcVar5 + (longlong)local_88);
              local_88 = (char ***)((longlong)local_88 + 1);
              pcVar10[0] = ',';
              pcVar10[1] = '\0';
            }
            else {
              if ((char ****)local_88 == (char ****)0x7fffffffffffffff) {
                    /* WARNING: Subroutine does not return */
                FUN_180019860();
              }
              ppppcVar13 = (char ****)((longlong)local_88 + 1);
              ppppcVar9 = (char ****)((ulonglong)ppppcVar13 | 0xf);
              if (ppppcVar9 < (char ****)0x8000000000000000) {
                if ((char ****)(0x7fffffffffffffff - ((ulonglong)local_80 >> 1)) < local_80) {
                  ppppcVar9 = (char ****)0x7fffffffffffffff;
                }
                else {
                  ppppcVar12 = (char ****)((longlong)local_80 + ((ulonglong)local_80 >> 1));
                  if (ppppcVar9 < ppppcVar12) {
                    ppppcVar9 = ppppcVar12;
                  }
                }
              }
              else {
                ppppcVar9 = (char ****)0x7fffffffffffffff;
              }
              pppppcVar5 = (char *****)FUN_1800013a0((longlong)ppppcVar9 + 1);
              ppppcVar12 = local_98;
              local_88 = (char ***)ppppcVar13;
              local_80 = (char ***)ppppcVar9;
              if (pppcVar3 < (char ****)0x10) {
                memcpy(pppppcVar5,&local_98,(size_t)pppcVar2);
                ((char *)((longlong)pppppcVar5 + (longlong)pppcVar2))[0] = ',';
                ((char *)((longlong)pppppcVar5 + (longlong)pppcVar2))[1] = '\0';
                ppppcVar12 = (char ****)local_108[0];
                local_98 = (char ****)pppppcVar5;
              }
              else {
                memcpy(pppppcVar5,local_98,(size_t)pppcVar2);
                ((char *)((longlong)pppppcVar5 + (longlong)pppcVar2))[0] = ',';
                ((char *)((longlong)pppppcVar5 + (longlong)pppcVar2))[1] = '\0';
                _Memory = (char *****)ppppcVar12;
                if ((0xfff < (longlong)pppcVar3 + 1U) &&
                   (_Memory = (char *****)ppppcVar12[-1],
                   (char *)0x1f < (char *)((longlong)ppppcVar12 + (-8 - (longlong)_Memory)))) {
                    /* WARNING: Subroutine does not return */
                  _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
                }
                free(_Memory);
                ppppcVar12 = (char ****)local_108[0];
                local_98 = (char ****)pppppcVar5;
              }
            }
          }
          plVar6 = FUN_1800273a0((longlong *)&local_68,(void **)(local_48 + DAT_18009c1d8));
          pppcVar2 = local_88;
          _Size = plVar6[2];
          if (0xf < (ulonglong)plVar6[3]) {
            plVar6 = (longlong *)*plVar6;
          }
          if ((ulonglong)((longlong)local_80 - (longlong)local_88) < _Size) {
            FUN_180007720(&local_98,_Size,(ulonglong)local_118,plVar6,_Size);
          }
          else {
            pppppcVar5 = &local_98;
            if ((char ****)0xf < local_80) {
              pppppcVar5 = (char *****)local_98;
            }
            pcVar10 = (char *)((longlong)pppppcVar5 + (longlong)local_88);
            local_88 = (char ***)((longlong)local_88 + _Size);
            memmove(pcVar10,plVar6,_Size);
            *(char *)((longlong)pppppcVar5 + (longlong)pppcVar2 + _Size) = '\0';
          }
          if (0xf < uStack_50) {
            pvVar7 = local_68;
            if ((0xfff < uStack_50 + 1) &&
               (pvVar7 = *(void **)((longlong)local_68 + -8),
               0x1f < (ulonglong)((longlong)local_68 + (-8 - (longlong)pvVar7)))) {
                    /* WARNING: Subroutine does not return */
              _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
            }
            free(pvVar7);
          }
          ppppcVar11 = (char ****)((longlong)ppppcVar11 + 1);
          local_48 = local_48 + 0x80;
          ppppcVar8 = local_f8;
          ppppcVar9 = (char ****)local_d8;
          ppppcVar13 = (char ****)local_c8;
        } while (ppppcVar11 < ppppcVar12);
      }
      ppppcVar11 = ppppcVar12;
      if ((ulonglong)((longlong)local_80 - (longlong)local_88) < 3) {
        FUN_180007720(&local_98,3,(ulonglong)local_118,&DAT_18005d11c,3);
      }
      else {
        pppppcVar5 = &local_98;
        if ((char ****)0xf < local_80) {
          pppppcVar5 = (char *****)local_98;
        }
        pcVar10 = (char *)((longlong)pppppcVar5 + (longlong)local_88);
        local_88 = (char ***)((longlong)local_88 + 3);
        memmove(pcVar10,&DAT_18005d11c,3);
        pcVar10[3] = '\0';
      }
      local_e0 = local_e0 + (longlong)local_88;
      if (ppppcVar9 < local_88) {
        ppppcVar9 = (char ****)local_88;
      }
      local_108[0] = (char ***)ppppcVar9;
      local_d8 = (char ***)ppppcVar9;
      _anonymous_namespace___InvokeOn(DAT_18009c1c0,&DAT_18009c1c8,&local_98);
      local_f8 = ppppcVar11;
      if ((char ****)0xf < local_80) {
        pppppcVar5 = (char *****)local_98;
        if ((0xfff < (longlong)local_80 + 1U) &&
           (pppppcVar5 = (char *****)local_98[-1],
           (char *)0x1f < (char *)((longlong)local_98 + (-8 - (longlong)pppppcVar5)))) {
                    /* WARNING: Subroutine does not return */
          _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
        }
        free(pppppcVar5);
      }
      param_1 = local_e8;
    } while (ppppcVar11 < ppppcVar13);
  }
  plVar6 = FUN_18001d7d0((longlong *)&local_e8);
  local_e8 = (longlong *)((*plVar6 - _DAT_18009c238) / 1000000);
  local_f8 = (char ****)0x18005d120;
  plStack_f0 = (longlong *)0x1b;
  local_78 = (char *)0x4000000000000000;
  local_c8 = (char ***)0x1;
  ppcStack_c0 = &local_78;
  uStack_60 = 0;
  local_58 = _DAT_18005c8e0;
  uStack_50 = _UNK_18005c8e8;
  local_68 = (void *)0x0;
  pppcStack_70 = (char ***)ppppcVar13;
  FUN_18001e160(&local_68,0x23);
  local_98 = (char ****)local_c8;
  ppcStack_90 = ppcStack_c0;
  FUN_18000f650(&local_c8,&local_68,(longlong *)&local_f8,&local_98);
  _anonymous_namespace___InvokeOn(DAT_18009c1c0,&DAT_18009c1c8,&local_68);
  if (0xf < uStack_50) {
    pvVar7 = local_68;
    if ((0xfff < uStack_50 + 1) &&
       (pvVar7 = *(void **)((longlong)local_68 + -8),
       0x1f < (ulonglong)((longlong)local_68 + (-8 - (longlong)pvVar7)))) {
                    /* WARNING: Subroutine does not return */
      _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
    }
    free(pvVar7);
  }
  plVar6 = (longlong *)FUN_180047e20();
  local_98 = (char ****)0x18005cbe8;
  ppcStack_90 = (char **)CONCAT44(ppcStack_90._4_4_,0x164);
  local_88 = (char ***)0x18005d140;
  local_78 = 
  "Catalog published to UI: {} scenes, {} JSON bytes ({} max per Invoke), {} ms since build start";
  pppcStack_70 = (char ***)0x5e;
  local_c8 = (char ***)0x18005cbe8;
  ppcStack_c0 = ppcStack_90;
  local_b8 = 
  "auto __cdecl `anonymous-namespace\'::PublishCatalogToUi::<lambda_1>::operator ()(void) const";
  FUN_180022be0(plVar6,(longlong *)&local_c8,2,(longlong *)&local_78,&local_a8,&local_e0,local_108,
                &local_e8);
  _Mtx_unlock(&DAT_1800951f0);
  return;
}

