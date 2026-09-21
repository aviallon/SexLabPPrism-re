// PrismExport addr=0x180022e60 name=anon ghidra=FUN_180022e60 status=clean lines=76

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_180022e60(longlong *param_1,longlong *param_2,int param_3,longlong *param_4,
                  undefined8 *param_5)

{
  longlong lVar1;
  char cVar2;
  undefined8 ****_Memory;
  undefined1 auStackY_158 [32];
  undefined8 ***local_128;
  longlong **local_120;
  longlong *local_118;
  longlong *local_110;
  longlong local_108;
  longlong lStack_100;
  longlong local_f8;
  undefined8 local_e0;
  undefined8 local_d8 [12];
  undefined8 ***local_78;
  undefined8 uStack_70;
  undefined8 local_68;
  ulonglong uStack_60;
  longlong *local_58;
  longlong local_50;
  ulonglong local_48;
  
  local_48 = DAT_180095900 ^ (ulonglong)auStackY_158;
  lVar1 = param_1[8];
  local_118 = param_1;
  local_110 = param_2;
  cVar2 = FUN_1800480e0((longlong)(param_1 + 0x11));
  if (((int)lVar1 <= param_3) || (cVar2 != '\0')) {
    uStack_70 = 0;
    local_68 = _DAT_18005c8e0;
    uStack_60 = _UNK_18005c8e8;
    local_78 = (undefined8 ****)0x0;
    local_50 = *param_5;
    local_58 = (longlong *)0xb000000000000000;
    local_128 = (undefined8 ***)0x1;
    local_120 = &local_58;
    local_108 = *param_4;
    lStack_100 = param_4[1];
    FUN_18000f650(&local_e0,&local_78,&local_108,&local_128);
    local_128 = &local_78;
    if (0xf < uStack_60) {
      local_128 = local_78;
    }
    local_58 = param_1 + 1;
    if (0xf < (ulonglong)param_1[4]) {
      local_58 = (longlong *)param_1[1];
    }
    local_120 = (longlong **)local_68;
    local_50 = param_1[3];
    local_108 = *param_2;
    lStack_100 = param_2[1];
    local_f8 = param_2[2];
    FUN_180044c00((undefined4 *)local_d8,&local_108,(undefined4 *)&local_58,param_3,&local_128);
    FUN_18004d1f0(param_1,local_d8,(int)lVar1 <= param_3,cVar2);
    if (0xf < uStack_60) {
      _Memory = (undefined8 ****)local_78;
      if ((0xfff < uStack_60 + 1) &&
         (_Memory = (undefined8 ****)local_78[-1],
         0x1f < (ulonglong)((longlong)local_78 + (-8 - (longlong)_Memory)))) {
                    /* WARNING: Subroutine does not return */
        _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
      }
      free(_Memory);
    }
  }
  return;
}

