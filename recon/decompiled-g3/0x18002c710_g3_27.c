// PrismExport addr=0x18002c710 name=g3_27 ghidra=FUN_18002c710 status=clean lines=18

void FUN_18002c710(ulonglong *param_1,ulonglong param_2)

{
  ulonglong uVar1;
  
  if (param_2 < 0x1000000000000000) {
    uVar1 = FUN_1800013a0(param_2 * 0x10);
    *param_1 = uVar1;
    param_1[1] = uVar1;
    param_1[2] = uVar1 + param_2 * 0x10;
    return;
  }
                    /* WARNING: Subroutine does not return */
  FUN_180019880();
}

