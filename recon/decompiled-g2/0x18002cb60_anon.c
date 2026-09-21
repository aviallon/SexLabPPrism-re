// PrismExport addr=0x18002cb60 name=anon ghidra=FUN_18002cb60 status=clean lines=12

void FUN_18002cb60(void *param_1,char param_2)

{
  FUN_180019420((longlong *)((longlong)param_1 + 0x18));
  if (param_2 != '\0') {
    free(param_1);
  }
  return;
}

