// PrismExport addr=0x180046f00 name=g3_20 ghidra=FUN_180046f00 status=clean lines=41

void FUN_180046f00(ulonglong *param_1,ulonglong param_2)

{
  code *pcVar1;
  longlong lVar2;
  ulonglong uVar3;
  longlong lVar4;
  ulonglong uVar5;
  
  if (param_2 < 0x200000000000000) {
    uVar5 = param_2 * 0x80;
    if (uVar5 == 0) {
      uVar3 = 0;
    }
    else if (uVar5 < 0x1000) {
      uVar3 = FUN_180050418(uVar5);
    }
    else {
      if (uVar5 + 0x27 <= uVar5) goto LAB_180046f81;
      lVar2 = FUN_180050418(uVar5 + 0x27);
      lVar4 = lVar2;
      if (lVar2 == 0) {
        lVar4 = 5;
        pcVar1 = (code *)swi(0x29);
        lVar2 = (*pcVar1)();
      }
      uVar3 = lVar2 + 0x27U & 0xffffffffffffffe0;
      *(longlong *)(uVar3 - 8) = lVar4;
    }
    *param_1 = uVar3;
    param_1[1] = uVar3;
    param_1[2] = uVar3 + uVar5;
    return;
  }
LAB_180046f81:
                    /* WARNING: Subroutine does not return */
  FUN_1800191d0();
}

