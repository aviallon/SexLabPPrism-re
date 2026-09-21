// PrismExport addr=0x18003e380 name=anon ghidra=FUN_18003e380 status=clean lines=25

undefined8 *
FUN_18003e380(undefined8 *param_1,undefined1 *param_2,undefined1 *param_3,undefined8 *param_4)

{
  undefined1 uVar1;
  
  if (param_2 == param_3) {
    *param_1 = param_4;
    return param_1;
  }
  do {
    uVar1 = *param_2;
    if ((ulonglong)param_4[3] < param_4[2] + 1) {
      (**(code **)*param_4)(param_4);
    }
    param_2 = param_2 + 1;
    *(undefined1 *)(param_4[2] + param_4[1]) = uVar1;
    param_4[2] = param_4[2] + 1;
  } while (param_2 != param_3);
  *param_1 = param_4;
  return param_1;
}

