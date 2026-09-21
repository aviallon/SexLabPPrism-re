// PrismExport addr=0x180024450 name=Window_slppSetSearchQuery ghidra=FUN_180024450 status=clean lines=120

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_180024450(undefined8 *param_1)

{
  longlong lVar1;
  undefined8 *puVar2;
  undefined8 *******pppppppuVar3;
  undefined8 *******_Memory;
  ulonglong uVar4;
  undefined *puVar5;
  undefined1 auStackY_c8 [32];
  byte local_98;
  undefined8 *local_88;
  undefined8 local_80;
  undefined8 ******local_78;
  undefined8 uStack_70;
  longlong local_68;
  ulonglong uStack_60;
  undefined8 ******local_58;
  undefined8 *****pppppuStack_50;
  undefined8 *****local_48;
  undefined8 *****pppppuStack_40;
  undefined8 ******local_38 [3];
  ulonglong local_20;
  ulonglong local_18;
  
  local_18 = DAT_180095900 ^ (ulonglong)auStackY_c8;
  local_88 = param_1;
  if (0xf < (ulonglong)param_1[3]) {
    local_88 = (undefined8 *)*param_1;
  }
  local_80 = param_1[2];
  puVar2 = FUN_18002bee0(local_38,(longlong *)&local_88);
  puVar5 = (undefined *)0x1a;
  puVar2 = FUN_18001bee0(puVar2,0,"window.slppSetSearchQuery(",0x1a);
  local_78 = (undefined8 ******)*puVar2;
  uStack_70 = puVar2[1];
  lVar1 = puVar2[2];
  uStack_60 = puVar2[3];
  *(undefined1 *)puVar2 = 0;
  puVar2[2] = 0;
  puVar2[3] = 0xf;
  if (uStack_60 - lVar1 < 2) {
    puVar5 = &DAT_18005d050;
    local_68 = lVar1;
    pppppppuVar3 = (undefined8 *******)
                   FUN_180007720(&local_78,2,(ulonglong)local_98,&DAT_18005d050,2);
  }
  else {
    local_68 = lVar1 + 2;
    pppppppuVar3 = &local_78;
    if (0xf < uStack_60) {
      pppppppuVar3 = (undefined8 *******)local_78;
    }
    *(undefined2 *)(lVar1 + (longlong)pppppppuVar3) = 0x3b29;
    *(undefined1 *)(lVar1 + 2 + (longlong)pppppppuVar3) = 0;
    pppppppuVar3 = &local_78;
  }
  local_58 = (undefined8 ******)0x0;
  pppppuStack_50 = (undefined8 *****)0x0;
  local_48 = (undefined8 *****)0x0;
  pppppuStack_40 = (undefined8 *****)0x0;
  local_58 = *pppppppuVar3;
  pppppuStack_50 = pppppppuVar3[1];
  local_48 = pppppppuVar3[2];
  pppppuStack_40 = pppppppuVar3[3];
  *(undefined1 *)pppppppuVar3 = 0;
  pppppppuVar3[2] = (undefined8 ******)0x0;
  pppppppuVar3[3] = (undefined8 ******)0xf;
  pppppppuVar3 = &local_58;
  _Memory = (undefined8 *******)DAT_18009c1c0;
  _anonymous_namespace___InvokeOn((longlong)DAT_18009c1c0,&DAT_18009c1c8,pppppppuVar3);
  if ((undefined8 ******)0xf < pppppuStack_40) {
    _Memory = (undefined8 *******)local_58;
    if ((0xfff < (longlong)pppppuStack_40 + 1U) &&
       (_Memory = (undefined8 *******)local_58[-1],
       0x1f < (ulonglong)((longlong)local_58 + (-8 - (longlong)_Memory)))) {
                    /* WARNING: Subroutine does not return */
      _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
    }
    free(_Memory);
  }
  local_48 = (undefined8 *****)_DAT_18005c8e0;
  pppppuStack_40 = (undefined8 *****)_UNK_18005c8e8;
  local_58 = (undefined8 ******)((ulonglong)local_58 & 0xffffffffffffff00);
  if (0xf < uStack_60) {
    _Memory = (undefined8 *******)local_78;
    if ((0xfff < uStack_60 + 1) &&
       (_Memory = (undefined8 *******)local_78[-1],
       0x1f < (ulonglong)((longlong)local_78 + (-8 - (longlong)_Memory)))) {
                    /* WARNING: Subroutine does not return */
      _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
    }
    free(_Memory);
  }
  local_68 = _DAT_18005c8e0;
  uStack_60 = _UNK_18005c8e8;
  local_78 = (undefined8 ******)((ulonglong)local_78 & 0xffffffffffffff00);
  uVar4 = local_20;
  if (0xf < local_20) {
    uVar4 = local_20 + 1;
    _Memory = (undefined8 *******)local_38[0];
    if (0xfff < uVar4) {
      uVar4 = local_20 + 0x28;
      _Memory = (undefined8 *******)local_38[0][-1];
      if (0x1f < (ulonglong)((longlong)local_38[0] + (-8 - (longlong)_Memory))) {
                    /* WARNING: Subroutine does not return */
        _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
      }
    }
    free(_Memory);
  }
  _anonymous_namespace___ApplyPresentation(_Memory,uVar4,(longlong)pppppppuVar3,puVar5);
  return;
}

