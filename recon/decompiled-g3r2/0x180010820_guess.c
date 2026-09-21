// PrismExport addr=0x180010820 name=guess ghidra=FUN_180010820 status=clean lines=43

void FUN_180010820(longlong *param_1)

{
  longlong lVar1;
  longlong *plVar2;
  void *pvVar3;
  undefined8 *puVar4;
  longlong lVar5;
  void *_Memory;
  
  lVar5 = *param_1;
  if (lVar5 != 0) {
    lVar1 = param_1[1];
    if (lVar5 != lVar1) {
      puVar4 = (undefined8 *)(lVar5 + 0x40);
      do {
        plVar2 = (longlong *)*puVar4;
        if (plVar2 != (longlong *)0x0) {
          (**(code **)(*plVar2 + 0x20))(plVar2,plVar2 != puVar4 + -7);
          *puVar4 = 0;
        }
        lVar5 = lVar5 + 0x48;
        puVar4 = puVar4 + 9;
      } while (lVar5 != lVar1);
    }
    pvVar3 = (void *)*param_1;
    _Memory = pvVar3;
    if ((0xfff < (ulonglong)(((param_1[2] - (longlong)pvVar3) / 0x48) * 0x48)) &&
       (_Memory = *(void **)((longlong)pvVar3 + -8),
       0x1f < (ulonglong)((longlong)pvVar3 + (-8 - (longlong)_Memory)))) {
                    /* WARNING: Subroutine does not return */
      _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
    }
    free(_Memory);
    *param_1 = 0;
    param_1[1] = 0;
    param_1[2] = 0;
  }
  return;
}

