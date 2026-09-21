// PrismExport addr=0x180010aa0 name=guess ghidra=FUN_180010aa0 status=clean lines=15

void FUN_180010aa0(longlong param_1)

{
  longlong *plVar1;
  
  plVar1 = *(longlong **)(param_1 + 0x40);
  if (plVar1 != (longlong *)0x0) {
    (**(code **)(*plVar1 + 0x20))(plVar1,plVar1 != (longlong *)(param_1 + 8));
    *(undefined8 *)(param_1 + 0x40) = 0;
  }
  return;
}

