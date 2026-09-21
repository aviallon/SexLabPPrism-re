// PrismExport addr=0x180023c10 name=anon ghidra=FUN_180023c10 status=clean lines=20

void FUN_180023c10(longlong *param_1,undefined8 param_2,longlong param_3,undefined8 param_4)

{
  longlong lVar1;
  longlong lVar2;
  
  lVar1 = *param_1;
  if (lVar1 != 0) {
    for (lVar2 = *(longlong *)(lVar1 + -8); lVar2 != 0; lVar2 = lVar2 + -1) {
      FUN_180010460(lVar1,param_2,param_3,param_4);
      lVar1 = lVar1 + 0x10;
    }
    FUN_18002df30(*param_1 + -8,param_2,param_3,param_4);
    *param_1 = 0;
  }
  return;
}

