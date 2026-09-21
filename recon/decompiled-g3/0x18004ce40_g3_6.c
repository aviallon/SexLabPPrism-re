// PrismExport addr=0x18004ce40 name=g3_6 ghidra=FUN_18004ce40 status=clean lines=89

void FUN_18004ce40(longlong param_1,longlong *param_2,longlong *param_3)

{
  char cVar1;
  int iVar2;
  int iVar3;
  longlong *plVar4;
  uint uVar5;
  longlong *plVar6;
  longlong *plVar7;
  longlong *local_res10 [3];
  
  plVar6 = (longlong *)*param_2;
  local_res10[0] = plVar6;
  local_res10[0] = FUN_180045fc0((longlong *)local_res10);
  local_res10[0] = (longlong *)*local_res10[0];
  plVar4 = (longlong *)*plVar6;
  if (plVar4 != plVar6) {
    do {
      plVar6 = plVar4 + 4;
      plVar7 = plVar4 + 8;
      uVar5 = ((*(int *)(param_1 + 8) - ((plVar4 != local_res10[0]) + 1)) - (int)plVar4[6]) -
              (int)plVar4[10];
      if (0 < (int)uVar5) {
        if (*(int *)(param_1 + 0x10) == 0) {
          FUN_18003a630(param_3,"                                                                ",
                        (longlong)(int)uVar5);
          uVar5 = 0;
        }
        else if (*(int *)(param_1 + 0x10) == 2) {
          FUN_18003a630(param_3,"                                                                ",
                        (longlong)((int)uVar5 / 2));
          uVar5 = (uVar5 & 1) + (int)uVar5 / 2;
        }
      }
      if (0xf < (ulonglong)plVar4[7]) {
        plVar6 = (longlong *)*plVar6;
      }
      FUN_18003a630(param_3,plVar6,plVar4[6]);
      FUN_18003a630(param_3,&DAT_1800624f0,1);
      if (0xf < (ulonglong)plVar4[0xb]) {
        plVar7 = (longlong *)*plVar7;
      }
      FUN_18003a630(param_3,plVar7,plVar4[10]);
      if (plVar4 != local_res10[0]) {
        FUN_18003a630(param_3,&DAT_1800624f4,1);
      }
      if ((int)uVar5 < 0) {
        if (*(char *)(param_1 + 0x14) != '\0') {
          iVar3 = (int)param_3[2] + uVar5;
          iVar2 = 0;
          if (-1 < iVar3) {
            iVar2 = iVar3;
          }
          FUN_18001e280(param_3,(longlong)iVar2,'\0');
        }
      }
      else {
        FUN_18003a630(param_3,"                                                                ",
                      (longlong)(int)uVar5);
      }
      plVar6 = (longlong *)plVar4[2];
      if (*(char *)((longlong)plVar6 + 0x19) == '\0') {
        cVar1 = *(char *)(*plVar6 + 0x19);
        plVar4 = plVar6;
        plVar6 = (longlong *)*plVar6;
        while (cVar1 == '\0') {
          cVar1 = *(char *)(*plVar6 + 0x19);
          plVar4 = plVar6;
          plVar6 = (longlong *)*plVar6;
        }
      }
      else {
        cVar1 = *(char *)(plVar4[1] + 0x19);
        plVar7 = (longlong *)plVar4[1];
        plVar6 = plVar4;
        while ((plVar4 = plVar7, cVar1 == '\0' && (plVar6 == (longlong *)plVar4[2]))) {
          cVar1 = *(char *)(plVar4[1] + 0x19);
          plVar7 = (longlong *)plVar4[1];
          plVar6 = plVar4;
        }
      }
    } while (plVar4 != (longlong *)*param_2);
  }
  return;
}

