// PrismExport addr=0x18003a500 name=anon ghidra=FUN_18003a500 status=clean lines=66

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

longlong * FUN_18003a500(longlong *param_1,longlong *param_2,uint param_3)

{
  undefined1 uVar1;
  undefined8 *puVar2;
  ulonglong uVar3;
  undefined1 *puVar4;
  size_t _Size;
  undefined1 *puVar5;
  uint uVar6;
  ulonglong uVar7;
  ulonglong *_Src;
  undefined1 auStack_78 [32];
  undefined1 local_58 [24];
  undefined1 local_40 [31];
  undefined1 local_21;
  ulonglong local_20;
  
  local_20 = DAT_180095900 ^ (ulonglong)auStack_78;
  puVar2 = (undefined8 *)*param_1;
  puVar4 = local_58;
  if ((int)param_3 < 0) {
    local_58[0] = 0x2d;
    puVar4 = local_58 + 1;
  }
  _Src = &local_20;
  uVar6 = -param_3;
  if ((int)-param_3 < 0) {
    uVar6 = param_3;
  }
  uVar7 = (ulonglong)uVar6;
  do {
    _Src = (ulonglong *)((longlong)_Src + -1);
    uVar3 = uVar7 / 10;
    *(char *)_Src = (char)uVar7 + (char)uVar3 * -10 + '0';
    uVar7 = uVar3;
  } while ((int)uVar3 != 0);
  _Size = (longlong)&local_20 - (longlong)_Src;
  if ((longlong)(local_40 + -(longlong)puVar4) < (longlong)_Size) {
    puVar4 = local_40;
    puVar5 = local_58;
  }
  else {
    memcpy(puVar4,_Src,_Size);
    puVar4 = puVar4 + _Size;
    puVar5 = local_58;
    if (local_58 == puVar4) goto LAB_18003a600;
  }
  do {
    uVar1 = *puVar5;
    if ((ulonglong)puVar2[3] < puVar2[2] + 1) {
      (**(code **)*puVar2)(puVar2);
    }
    puVar5 = puVar5 + 1;
    *(undefined1 *)(puVar2[2] + puVar2[1]) = uVar1;
    puVar2[2] = puVar2[2] + 1;
  } while (puVar5 != puVar4);
LAB_18003a600:
  *param_2 = (longlong)puVar2;
  return param_2;
}

