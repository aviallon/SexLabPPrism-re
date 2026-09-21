// PrismExport addr=0x180052f30 name=g3_30 ghidra=FUN_180052f30 status=clean lines=12

void FUN_180052f30(undefined8 param_1,longlong param_2)

{
  if ((*(uint *)(param_2 + 0x30) & 8) != 0) {
    *(uint *)(param_2 + 0x30) = *(uint *)(param_2 + 0x30) & 0xfffffff7;
    thunk_FUN_180019420((longlong *)(param_2 + 0x60));
  }
  return;
}

