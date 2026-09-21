// PrismExport addr=0x18003ac10 name=g3_21 ghidra=FUN_18003ac10 status=clean lines=21

void FUN_18003ac10(undefined8 param_1,undefined8 param_2,longlong *param_3)

{
  char cVar1;
  longlong *plVar2;
  
  cVar1 = *(char *)((longlong)param_3 + 0x19);
  while (cVar1 == '\0') {
    FUN_18003ac10(param_1,param_2,(longlong *)param_3[2]);
    plVar2 = (longlong *)*param_3;
    FUN_180019420(param_3 + 8);
    FUN_180019420(param_3 + 4);
    free(param_3);
    param_3 = plVar2;
    cVar1 = *(char *)((longlong)plVar2 + 0x19);
  }
  return;
}

