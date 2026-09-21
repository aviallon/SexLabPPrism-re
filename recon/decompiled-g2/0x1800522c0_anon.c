// PrismExport addr=0x1800522c0 name=anon ghidra=FUN_1800522c0 status=clean lines=12

void FUN_1800522c0(undefined8 param_1,longlong param_2)

{
  if ((*(uint *)(param_2 + 0x50) & 8) != 0) {
    *(uint *)(param_2 + 0x50) = *(uint *)(param_2 + 0x50) & 0xfffffff7;
    FUN_180010690(*(longlong **)(param_2 + 0x40));
  }
  return;
}

