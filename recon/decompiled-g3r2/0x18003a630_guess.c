// PrismExport addr=0x18003a630 name=guess ghidra=FUN_18003a630 status=clean lines=24

longlong * FUN_18003a630(longlong *param_1,void *param_2,size_t param_3)

{
  longlong lVar1;
  longlong *plVar2;
  byte local_res8;
  
  lVar1 = param_1[2];
  if ((ulonglong)(param_1[3] - lVar1) < param_3) {
    plVar2 = (longlong *)FUN_18003cd70(param_1,param_3,(ulonglong)local_res8,param_2,param_3);
    return plVar2;
  }
  param_1[2] = lVar1 + param_3;
  plVar2 = param_1;
  if (0xf < (ulonglong)param_1[3]) {
    plVar2 = (longlong *)*param_1;
  }
  memmove((void *)(lVar1 + (longlong)plVar2),param_2,param_3);
  *(undefined1 *)(lVar1 + (longlong)plVar2 + param_3) = 0;
  return param_1;
}

