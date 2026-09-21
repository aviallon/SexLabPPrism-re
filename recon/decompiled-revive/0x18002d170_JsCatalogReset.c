// PrismExport addr=0x18002d170 name=JsCatalogReset ghidra=FUN_18002d170 status=clean lines=72

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_18002d170(longlong param_1)

{
  byte bVar1;
  void *_Memory;
  longlong lVar2;
  ulonglong uVar3;
  undefined1 auStackY_b8 [32];
  char *local_78;
  undefined8 uStack_70;
  undefined8 local_68;
  ulonglong *puStack_60;
  undefined8 local_58;
  ulonglong *puStack_50;
  ulonglong local_48 [2];
  undefined4 local_38;
  undefined4 local_34;
  void *local_30 [3];
  ulonglong uStack_18;
  ulonglong local_10;
  
  local_10 = DAT_180095900 ^ (ulonglong)auStackY_b8;
  local_78 = "window.slppCatalogReset({});window.slppCatalogProgress(0,{});";
  uStack_70 = 0x3d;
  local_38 = *(undefined4 *)(param_1 + 8);
  local_48[0] = 0x1000000000000000;
  local_48[1] = 0x1000000000000004;
  local_68 = 2;
  puStack_60 = local_48;
  local_30[1] = (void *)0x0;
  local_30[2] = (void *)_DAT_18005c8e0;
  uStack_18 = _UNK_18005c8e8;
  local_30[0] = (void *)0x0;
  lVar2 = 0;
  uVar3 = 0;
  do {
    bVar1 = (byte)(local_48[uVar3] >> 0x3c);
    if (bVar1 == 0xc) {
      lVar2 = lVar2 + *(longlong *)((longlong)local_30 + (local_48[uVar3] & 0xfffffffffffffff));
    }
    else if (bVar1 == 0xb) {
      lVar2 = lVar2 + 0x20;
    }
    else {
      lVar2 = lVar2 + 8;
    }
    uVar3 = uVar3 + 1;
  } while (uVar3 < 2);
  local_34 = local_38;
  FUN_18001e160(local_30,lVar2 + 0x3d);
  local_58 = local_68;
  puStack_50 = puStack_60;
  FUN_18000f650(&local_68,local_30,(longlong *)&local_78,&local_58);
  _anonymous_namespace___InvokeOn(DAT_18009c1c0,&DAT_18009c1c8,local_30);
  if (0xf < uStack_18) {
    _Memory = local_30[0];
    if ((0xfff < uStack_18 + 1) &&
       (_Memory = *(void **)((longlong)local_30[0] + -8),
       0x1f < (ulonglong)((longlong)local_30[0] + (-8 - (longlong)_Memory)))) {
                    /* WARNING: Subroutine does not return */
      _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
    }
    free(_Memory);
  }
  return;
}

