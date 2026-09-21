// PrismExport addr=0x180047470 name=guess ghidra=FUN_180047470 status=clean lines=26

void FUN_180047470(longlong param_1,undefined1 *param_2,undefined1 *param_3)

{
  undefined1 uVar1;
  undefined8 *puVar2;
  
  puVar2 = *(undefined8 **)(param_1 + 0x20);
  if (param_2 == param_3) {
    *(undefined8 **)(param_1 + 0x20) = puVar2;
    return;
  }
  do {
    uVar1 = *param_2;
    if ((ulonglong)puVar2[3] < puVar2[2] + 1) {
      (**(code **)*puVar2)(puVar2);
    }
    param_2 = param_2 + 1;
    *(undefined1 *)(puVar2[2] + puVar2[1]) = uVar1;
    puVar2[2] = puVar2[2] + 1;
  } while (param_2 != param_3);
  *(undefined8 **)(param_1 + 0x20) = puVar2;
  return;
}

