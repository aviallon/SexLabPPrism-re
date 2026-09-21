// PrismExport addr=0x18003a6c0 name=g3_12 ghidra=FUN_18003a6c0 status=clean lines=92

undefined8 * FUN_18003a6c0(undefined8 *param_1,void *param_2,size_t param_3)

{
  ulonglong uVar1;
  ulonglong uVar2;
  undefined1 *puVar3;
  code *pcVar4;
  longlong lVar5;
  ulonglong uVar6;
  undefined1 *_Memory;
  undefined1 *puVar7;
  undefined8 *_Dst;
  ulonglong uVar8;
  void *unaff_R14;
  undefined1 auStack_38 [8];
  undefined1 auStack_30 [24];
  
  uVar2 = param_1[3];
  if (param_3 <= uVar2) {
    _Dst = param_1;
    if (0xf < uVar2) {
      _Dst = (undefined8 *)*param_1;
    }
    param_1[2] = param_3;
    memmove(_Dst,param_2,param_3);
    *(undefined1 *)((longlong)_Dst + param_3) = 0;
    return param_1;
  }
  uVar8 = 0x7fffffffffffffff;
  if (0x7fffffffffffffff < param_3) {
                    /* WARNING: Subroutine does not return */
    FUN_180019860();
  }
  uVar6 = param_3 | 0xf;
  if ((uVar6 < 0x8000000000000000) && (uVar2 <= 0x7fffffffffffffff - (uVar2 >> 1))) {
    uVar1 = (uVar2 >> 1) + uVar2;
    uVar8 = uVar6;
    if (uVar6 < uVar1) {
      uVar8 = uVar1;
    }
    uVar1 = uVar8 + 1;
    if (uVar1 == 0) {
      unaff_R14 = (void *)0x0;
    }
    else {
      if (0xfff < uVar1) {
        uVar6 = uVar8 + 0x28;
        if (uVar6 <= uVar1) {
                    /* WARNING: Subroutine does not return */
          FUN_1800191d0();
        }
        goto LAB_18003a779;
      }
      unaff_R14 = (void *)FUN_180050418(uVar1);
    }
LAB_18003a79c:
    param_1[2] = param_3;
    param_1[3] = uVar8;
    memcpy(unaff_R14,param_2,param_3);
    *(undefined1 *)((longlong)unaff_R14 + param_3) = 0;
    if (uVar2 < 0x10) goto LAB_18003a7f6;
    puVar3 = (undefined1 *)*param_1;
    _Memory = puVar3;
    puVar7 = auStack_38;
    if ((0xfff < uVar2 + 1) &&
       (_Memory = *(undefined1 **)(puVar3 + -8), puVar7 = auStack_38,
       (undefined1 *)0x1f < puVar3 + (-8 - (longlong)_Memory))) goto LAB_18003a7e4;
  }
  else {
    uVar6 = 0x8000000000000027;
LAB_18003a779:
    lVar5 = FUN_180050418(uVar6);
    if (lVar5 != 0) {
      unaff_R14 = (void *)(lVar5 + 0x27U & 0xffffffffffffffe0);
      *(longlong *)((longlong)unaff_R14 - 8) = lVar5;
      goto LAB_18003a79c;
    }
LAB_18003a7e4:
    _Memory = &DAT_00000005;
    pcVar4 = (code *)swi(0x29);
    (*pcVar4)();
    puVar7 = auStack_30;
  }
  *(undefined8 *)(puVar7 + -8) = 0x18003a7f6;
  free(_Memory);
LAB_18003a7f6:
  *param_1 = unaff_R14;
  return param_1;
}

