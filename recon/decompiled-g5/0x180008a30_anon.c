// PrismExport addr=0x180008a30 name=anon ghidra=FUN_180008a30 status=clean lines=26

longlong * FUN_180008a30(longlong *param_1,longlong *param_2,longlong *param_3)

{
  longlong *plVar1;
  longlong lVar2;
  longlong *plVar3;
  longlong lVar4;
  
  if (param_1 != param_2) {
    lVar4 = (longlong)param_3 - (longlong)param_1;
    plVar3 = param_1 + 1;
    do {
      *param_3 = plVar3[-1];
      lVar2 = lVar4 + (longlong)plVar3;
      *(undefined8 *)(lVar2 + 0x38) = 0;
      FUN_180018cc0(lVar2,plVar3);
      param_3 = param_3 + 9;
      plVar1 = plVar3 + 8;
      plVar3 = plVar3 + 9;
    } while (plVar1 != param_2);
  }
  return param_3;
}

