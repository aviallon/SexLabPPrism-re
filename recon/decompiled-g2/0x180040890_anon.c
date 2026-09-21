// PrismExport addr=0x180040890 name=anon ghidra=FUN_180040890 status=clean lines=29

void FUN_180040890(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3)

{
  undefined8 uVar1;
  ulonglong uVar2;
  ulonglong uVar3;
  undefined8 *puVar4;
  
  uVar3 = (ulonglong)((longlong)param_2 + (7 - (longlong)param_1)) >> 3;
  if (param_2 < param_1) {
    uVar3 = 0;
  }
  if ((1 < uVar3) && ((param_3 < param_1 || (param_1 + (uVar3 - 1) < param_3)))) {
    uVar1 = *param_3;
    puVar4 = param_1;
    for (uVar2 = uVar3 & 0x1ffffffffffffffe; uVar2 != 0; uVar2 = uVar2 - 1) {
      *puVar4 = uVar1;
      puVar4 = puVar4 + 1;
    }
    param_1 = param_1 + (uVar3 & 0xfffffffffffffffe);
  }
  for (; param_1 != param_2; param_1 = param_1 + 1) {
    *param_1 = *param_3;
  }
  return;
}

