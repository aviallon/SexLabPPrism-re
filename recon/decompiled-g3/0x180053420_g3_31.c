// PrismExport addr=0x180053420 name=g3_31 ghidra=FUN_180053420 status=clean lines=12

void FUN_180053420(undefined8 param_1,longlong param_2)

{
  if ((*(uint *)(param_2 + 0xa0) & 2) != 0) {
    *(uint *)(param_2 + 0xa0) = *(uint *)(param_2 + 0xa0) & 0xfffffffd;
    thunk_FUN_180019420((longlong *)(param_2 + 0x178));
  }
  return;
}

