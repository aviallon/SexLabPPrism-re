// PrismExport addr=0x180022be0 name=g3_8 ghidra=FUN_180022be0 status=clean lines=89

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_180022be0(longlong *param_1,longlong *param_2,int param_3,longlong *param_4,
                  undefined8 *param_5,undefined8 *param_6,undefined8 *param_7,undefined8 *param_8)

{
  longlong lVar1;
  char cVar2;
  undefined8 ****_Memory;
  undefined1 auStackY_198 [32];
  undefined8 ***local_168;
  undefined8 *local_160;
  longlong *local_158;
  longlong *local_150;
  longlong local_148;
  longlong lStack_140;
  longlong local_138;
  longlong *local_118;
  longlong local_110;
  undefined8 local_108 [12];
  undefined8 ***local_a8;
  undefined8 uStack_a0;
  undefined8 local_98;
  ulonglong uStack_90;
  undefined8 local_88;
  undefined8 local_80;
  undefined8 local_78;
  undefined8 local_70;
  undefined8 local_68;
  undefined8 local_60;
  undefined8 local_58;
  undefined8 local_50;
  ulonglong local_48;
  
  local_48 = DAT_180095900 ^ (ulonglong)auStackY_198;
  lVar1 = param_1[8];
  local_158 = param_1;
  local_150 = param_2;
  cVar2 = FUN_1800480e0((longlong)(param_1 + 0x11));
  if (((int)lVar1 <= param_3) || (cVar2 != '\0')) {
    uStack_a0 = 0;
    local_98 = _DAT_18005c8e0;
    uStack_90 = _UNK_18005c8e8;
    local_a8 = (undefined8 ****)0x0;
    local_68 = *param_5;
    local_88 = 0x4000000000000000;
    local_60 = *param_6;
    local_80 = 0x4000000000000008;
    local_58 = *param_7;
    local_78 = 0x4000000000000010;
    local_50 = *param_8;
    local_70 = 0x3000000000000018;
    local_168 = (undefined8 ***)0x4;
    local_160 = &local_88;
    local_148 = *param_4;
    lStack_140 = param_4[1];
    FUN_18000f650(&local_118,&local_a8,&local_148,&local_168);
    local_168 = &local_a8;
    if (0xf < uStack_90) {
      local_168 = local_a8;
    }
    local_118 = param_1 + 1;
    if (0xf < (ulonglong)param_1[4]) {
      local_118 = (longlong *)param_1[1];
    }
    local_160 = (undefined8 *)local_98;
    local_110 = param_1[3];
    local_148 = *param_2;
    lStack_140 = param_2[1];
    local_138 = param_2[2];
    FUN_180044c00((undefined4 *)local_108,&local_148,(undefined4 *)&local_118,param_3,&local_168);
    FUN_18004d1f0(param_1,local_108,(int)lVar1 <= param_3,cVar2);
    if (0xf < uStack_90) {
      _Memory = (undefined8 ****)local_a8;
      if ((0xfff < uStack_90 + 1) &&
         (_Memory = (undefined8 ****)local_a8[-1],
         0x1f < (ulonglong)((longlong)local_a8 + (-8 - (longlong)_Memory)))) {
                    /* WARNING: Subroutine does not return */
        _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
      }
      free(_Memory);
    }
  }
  return;
}

