// PrismExport addr=0x1800158d0 name=.?AV_Node_base@std@@::vtable[0] ghidra=FUN_1800158d0 status=clean lines=20

void FUN_1800158d0(longlong param_1)

{
  undefined8 *puVar1;
  
  puVar1 = (undefined8 *)FUN_180050418(0x20);
  if (puVar1 != (undefined8 *)0x0) {
    puVar1[2] = 0;
    *puVar1 = std::_Node_base::vftable;
    puVar1[3] = 0;
    puVar1[1] = 8;
    FUN_180018170(param_1,(longlong)puVar1);
    return;
  }
  FUN_180018170(param_1,0);
  return;
}

