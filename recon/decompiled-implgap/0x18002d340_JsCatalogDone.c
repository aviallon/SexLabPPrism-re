// PrismExport addr=0x18002d340 name=JsCatalogDone ghidra=FUN_18002d340 status=clean lines=53

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_18002d340(longlong param_1)

{
  void *_Memory;
  undefined1 auStackY_b8 [32];
  char *local_78;
  undefined8 uStack_70;
  undefined8 local_68;
  undefined8 *puStack_60;
  undefined8 local_58;
  undefined8 *puStack_50;
  void *local_48;
  undefined8 uStack_40;
  undefined8 local_38;
  ulonglong uStack_30;
  undefined8 local_28;
  undefined8 local_20;
  ulonglong local_18;
  
  local_18 = DAT_180095900 ^ (ulonglong)auStackY_b8;
  local_78 = "window.slppCatalogDone({});";
  uStack_70 = 0x1b;
  local_20 = *(undefined8 *)(param_1 + 8);
  local_28 = 0x4000000000000000;
  local_68 = 1;
  puStack_60 = &local_28;
  uStack_40 = 0;
  local_38 = _DAT_18005c8e0;
  uStack_30 = _UNK_18005c8e8;
  local_48 = (void *)0x0;
  FUN_18001e160(&local_48,0x23);
  local_58 = local_68;
  puStack_50 = puStack_60;
  FUN_18000f650(&local_68,&local_48,(longlong *)&local_78,&local_58);
  _anonymous_namespace___InvokeOn(DAT_18009c1c0,&DAT_18009c1c8,&local_48);
  if (0xf < uStack_30) {
    _Memory = local_48;
    if ((0xfff < uStack_30 + 1) &&
       (_Memory = *(void **)((longlong)local_48 + -8),
       0x1f < (ulonglong)((longlong)local_48 + (-8 - (longlong)_Memory)))) {
                    /* WARNING: Subroutine does not return */
      _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
    }
    free(_Memory);
  }
  return;
}

