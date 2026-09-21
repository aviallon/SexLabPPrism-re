// PrismExport addr=0x180025ce0 name=g3_5 ghidra=FUN_180025ce0 status=clean lines=115

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_180025ce0(longlong *param_1,uint param_2)

{
  byte bVar1;
  undefined8 ****ppppuVar2;
  longlong lVar3;
  ulonglong uVar4;
  undefined1 auStackY_c8 [32];
  char *local_88;
  char **ppcStack_80;
  char *local_78;
  char **ppcStack_70;
  undefined8 ***local_68;
  undefined8 uStack_60;
  size_t local_58;
  ulonglong uStack_50;
  char *local_48;
  undefined8 uStack_40;
  char *local_38;
  char **ppcStack_30;
  uint local_28;
  uint local_24;
  longlong alStack_20 [3];
  
  alStack_20[1] = DAT_180095900 ^ (ulonglong)auStackY_c8;
  if (param_2 < 0x10000) {
    local_78 = "\\u{:04X}";
    ppcStack_70 = (char **)0x8;
    uStack_40 = CONCAT44(uStack_40._4_4_,param_2);
    local_48 = (char *)0x2000000000000000;
    local_88 = (char *)0x1;
    ppcStack_80 = &local_48;
    uStack_60 = 0;
    local_58 = _DAT_18005c8e0;
    uStack_50 = _UNK_18005c8e8;
    local_68 = (undefined8 ****)0x0;
    FUN_18001e160(&local_68,0x10);
    local_38 = local_88;
    ppcStack_30 = ppcStack_80;
    FUN_18000f650(&local_88,&local_68,(longlong *)&local_78,&local_38);
    ppppuVar2 = &local_68;
    if (0xf < uStack_50) {
      ppppuVar2 = (undefined8 ****)local_68;
    }
    FUN_18001afb0(param_1,ppppuVar2,local_58);
    if (uStack_50 < 0x10) {
      return;
    }
    ppppuVar2 = (undefined8 ****)local_68;
    if ((0xfff < uStack_50 + 1) &&
       (ppppuVar2 = (undefined8 ****)local_68[-1],
       0x1f < (ulonglong)((longlong)local_68 + (-8 - (longlong)ppppuVar2)))) {
                    /* WARNING: Subroutine does not return */
      _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
    }
  }
  else {
    local_48 = "\\u{:04X}\\u{:04X}";
    uStack_40 = 0x10;
    local_28 = (uint)(ushort)((short)(param_2 - 0x10000 >> 10) + 0xd800);
    local_38 = (char *)0x2000000000000000;
    local_24 = (uint)(ushort)(((ushort)(param_2 - 0x10000) & 0x3ff) + 0xdc00);
    ppcStack_30 = (char **)0x2000000000000004;
    local_88 = (char *)0x2;
    ppcStack_80 = &local_38;
    uStack_60 = 0;
    local_58 = _DAT_18005c8e0;
    uStack_50 = _UNK_18005c8e8;
    local_68 = (undefined8 ****)0x0;
    lVar3 = 0;
    uVar4 = 0;
    do {
      bVar1 = (byte)((ulonglong)(&local_38)[uVar4] >> 0x3c);
      if (bVar1 == 0xc) {
        lVar3 = lVar3 + *(longlong *)
                         ((longlong)alStack_20 + ((ulonglong)(&local_38)[uVar4] & 0xfffffffffffffff)
                         );
      }
      else if (bVar1 == 0xb) {
        lVar3 = lVar3 + 0x20;
      }
      else {
        lVar3 = lVar3 + 8;
      }
      uVar4 = uVar4 + 1;
    } while (uVar4 < 2);
    FUN_18001e160(&local_68,lVar3 + 0x10);
    local_78 = local_88;
    ppcStack_70 = ppcStack_80;
    FUN_18000f650(&local_88,&local_68,(longlong *)&local_48,&local_78);
    ppppuVar2 = &local_68;
    if (0xf < uStack_50) {
      ppppuVar2 = (undefined8 ****)local_68;
    }
    FUN_18001afb0(param_1,ppppuVar2,local_58);
    if (uStack_50 < 0x10) {
      return;
    }
    ppppuVar2 = (undefined8 ****)local_68;
    if ((0xfff < uStack_50 + 1) &&
       (ppppuVar2 = (undefined8 ****)local_68[-1],
       0x1f < (ulonglong)((longlong)local_68 + (-8 - (longlong)local_68[-1])))) {
                    /* WARNING: Subroutine does not return */
      _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
    }
  }
  free(ppppuVar2);
  return;
}

