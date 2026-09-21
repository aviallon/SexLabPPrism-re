// PrismExport addr=0x18002e000 name=guess ghidra=FUN_18002e000 status=clean lines=47

void FUN_18002e000(undefined8 *param_1,void *param_2,size_t param_3,undefined8 param_4)

{
  void *_Src;
  uint uVar1;
  undefined *_Dst;
  uint uVar2;
  ulonglong uVar3;
  float fVar4;
  
  if ((int)param_2 == 0) {
    uVar3 = 4;
  }
  else {
    fVar4 = ceilf((float)((ulonglong)param_2 & 0xffffffff) +
                  (float)((ulonglong)param_2 & 0xffffffff));
    uVar3 = (ulonglong)fVar4;
    if ((int)uVar3 == 0) {
      _Dst = (undefined *)0x0;
      uVar2 = 0;
      goto LAB_18002e051;
    }
  }
  uVar2 = (uint)uVar3;
  param_2 = (void *)((uVar3 & 0xffffffff) << 3);
  _Dst = RE__BSTArrayHeapAllocator__allocate(param_1,(size_t)param_2,param_3,param_4);
LAB_18002e051:
  _Src = (void *)*param_1;
  if (_Src != (void *)0x0) {
    if (_Dst != (undefined *)0x0) {
      uVar1 = *(uint *)(param_1 + 1);
      if (uVar2 < *(uint *)(param_1 + 1)) {
        uVar1 = uVar2;
      }
      param_3 = (ulonglong)uVar1 << 3;
      param_2 = _Src;
      memcpy(_Dst,_Src,param_3);
    }
    FUN_18002df30(_Src,param_2,param_3,param_4);
  }
  *(uint *)(param_1 + 1) = uVar2;
  *param_1 = _Dst;
  return;
}

