// PrismExport addr=0x18004dfa0 name=g3_16 ghidra=FUN_18004dfa0 status=clean lines=43

undefined4 * FUN_18004dfa0(undefined4 *param_1,longlong param_2)

{
  undefined8 *puVar1;
  undefined8 uVar2;
  
  *param_1 = 5;
  param_1[1] = 10;
  *(undefined8 *)(param_1 + 2) = 0;
  *(undefined8 *)(param_1 + 4) = 0;
  *(undefined8 *)(param_1 + 6) = 0;
  *(undefined8 *)(param_1 + 8) = 0;
  *(undefined8 *)(param_1 + 10) = 0xf;
  *(undefined1 *)(param_1 + 4) = 0;
  *(undefined8 *)(param_1 + 0x1a) = 0;
  puVar1 = *(undefined8 **)(param_2 + 0x38);
  if (puVar1 != (undefined8 *)0x0) {
    uVar2 = (**(code **)*puVar1)(puVar1,param_1 + 0xc);
    *(undefined8 *)(param_1 + 0x1a) = uVar2;
  }
  *(undefined8 *)(param_1 + 0x2a) = 0;
  puVar1 = *(undefined8 **)(param_2 + 0x78);
  if (puVar1 != (undefined8 *)0x0) {
    uVar2 = (**(code **)*puVar1)(puVar1,param_1 + 0x1c);
    *(undefined8 *)(param_1 + 0x2a) = uVar2;
  }
  *(undefined8 *)(param_1 + 0x3a) = 0;
  puVar1 = *(undefined8 **)(param_2 + 0xb8);
  if (puVar1 != (undefined8 *)0x0) {
    uVar2 = (**(code **)*puVar1)(puVar1,param_1 + 0x2c);
    *(undefined8 *)(param_1 + 0x3a) = uVar2;
  }
  *(undefined8 *)(param_1 + 0x4a) = 0;
  puVar1 = *(undefined8 **)(param_2 + 0xf8);
  if (puVar1 != (undefined8 *)0x0) {
    uVar2 = (**(code **)*puVar1)(puVar1,param_1 + 0x3c);
    *(undefined8 *)(param_1 + 0x4a) = uVar2;
  }
  return param_1;
}

