// PrismExport addr=0x180022340 name=anon ghidra=FUN_180022340 status=clean lines=81

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_180022340(longlong *param_1,longlong *param_2,int param_3,longlong *param_4,
                  undefined8 *param_5,undefined8 *param_6)

{
  longlong lVar1;
  char cVar2;
  undefined8 ****_Memory;
  undefined1 auStackY_178 [32];
  undefined8 ***local_148;
  undefined8 *local_140;
  longlong *local_138;
  longlong *local_130;
  longlong local_128;
  longlong lStack_120;
  longlong local_118;
  longlong *local_f8;
  longlong local_f0;
  undefined8 local_e8 [12];
  undefined8 ***local_88;
  undefined8 uStack_80;
  undefined8 local_78;
  ulonglong uStack_70;
  undefined8 local_68;
  undefined8 local_60;
  undefined8 local_58;
  undefined8 local_50;
  ulonglong local_48;
  
  local_48 = DAT_180095900 ^ (ulonglong)auStackY_178;
  lVar1 = param_1[8];
  local_138 = param_1;
  local_130 = param_2;
  cVar2 = FUN_1800480e0((longlong)(param_1 + 0x11));
  if (((int)lVar1 <= param_3) || (cVar2 != '\0')) {
    uStack_80 = 0;
    local_78 = _DAT_18005c8e0;
    uStack_70 = _UNK_18005c8e8;
    local_88 = (undefined8 ****)0x0;
    local_58 = *param_5;
    local_68 = 0x4000000000000000;
    local_50 = *param_6;
    local_60 = 0x3000000000000008;
    local_148 = (undefined8 ***)0x2;
    local_140 = &local_68;
    local_128 = *param_4;
    lStack_120 = param_4[1];
    FUN_18000f650(&local_f8,&local_88,&local_128,&local_148);
    local_148 = &local_88;
    if (0xf < uStack_70) {
      local_148 = local_88;
    }
    local_f8 = param_1 + 1;
    if (0xf < (ulonglong)param_1[4]) {
      local_f8 = (longlong *)param_1[1];
    }
    local_140 = (undefined8 *)local_78;
    local_f0 = param_1[3];
    local_128 = *param_2;
    lStack_120 = param_2[1];
    local_118 = param_2[2];
    FUN_180044c00((undefined4 *)local_e8,&local_128,(undefined4 *)&local_f8,param_3,&local_148);
    FUN_18004d1f0(param_1,local_e8,(int)lVar1 <= param_3,cVar2);
    if (0xf < uStack_70) {
      _Memory = (undefined8 ****)local_88;
      if ((0xfff < uStack_70 + 1) &&
         (_Memory = (undefined8 ****)local_88[-1],
         0x1f < (ulonglong)((longlong)local_88 + (-8 - (longlong)_Memory)))) {
                    /* WARNING: Subroutine does not return */
        _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
      }
      free(_Memory);
    }
  }
  return;
}

