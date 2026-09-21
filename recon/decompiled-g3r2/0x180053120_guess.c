// PrismExport addr=0x180053120 name=guess ghidra=FUN_180053120 status=clean lines=12

void FUN_180053120(undefined8 param_1,longlong param_2)

{
  if ((*(uint *)(param_2 + 0x40) & 1) != 0) {
    *(uint *)(param_2 + 0x40) = *(uint *)(param_2 + 0x40) & 0xfffffffe;
    thunk_FUN_180019420(*(longlong **)(param_2 + 0xb0));
  }
  return;
}

