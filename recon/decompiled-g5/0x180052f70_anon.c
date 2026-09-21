// PrismExport addr=0x180052f70 name=anon ghidra=FUN_180052f70 status=clean lines=12

void FUN_180052f70(undefined8 param_1,longlong param_2)

{
  if ((*(uint *)(param_2 + 0x90) & 2) != 0) {
    *(uint *)(param_2 + 0x90) = *(uint *)(param_2 + 0x90) & 0xfffffffd;
    thunk_FUN_180019420((longlong *)(param_2 + 0x98));
  }
  return;
}

