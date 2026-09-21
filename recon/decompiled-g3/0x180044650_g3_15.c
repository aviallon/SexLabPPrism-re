// PrismExport addr=0x180044650 name=g3_15 ghidra=FUN_180044650 status=clean lines=49

void FUN_180044650(ulonglong param_1,uint param_2,longlong *param_3)

{
  ulonglong uVar1;
  uint uVar2;
  byte local_res10;
  
  uVar2 = 1;
  uVar1 = param_1;
  do {
    if (uVar1 < 10) {
joined_r0x0001800446bb:
      for (; uVar2 < param_2; uVar2 = uVar2 + 1) {
        uVar1 = param_3[2];
        if (uVar1 < (ulonglong)param_3[3]) {
          param_3[2] = uVar1 + 1;
          if ((ulonglong)param_3[3] < 0x10) {
            *(undefined2 *)(uVar1 + (longlong)param_3) = 0x30;
          }
          else {
            *(undefined2 *)(uVar1 + *param_3) = 0x30;
          }
        }
        else {
          FUN_180007dd0(param_3,1,(ulonglong)local_res10,0x30);
        }
      }
      FUN_1800407b0(param_1,param_3);
      return;
    }
    if (uVar1 < 100) {
      uVar2 = uVar2 + 1;
      goto joined_r0x0001800446bb;
    }
    if (uVar1 < 1000) {
      uVar2 = uVar2 + 2;
      goto joined_r0x0001800446bb;
    }
    if (uVar1 < 10000) {
      uVar2 = uVar2 + 3;
      goto joined_r0x0001800446bb;
    }
    uVar2 = uVar2 + 4;
    uVar1 = uVar1 / 10000;
  } while( true );
}

