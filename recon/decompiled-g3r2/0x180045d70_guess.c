// PrismExport addr=0x180045d70 name=guess ghidra=FUN_180045d70 status=clean lines=24

void FUN_180045d70(longlong *param_1)

{
  int iVar1;
  int iVar2;
  
  iVar2 = (int)param_1[2];
  if (-1 < iVar2) {
    FUN_18003a630((longlong *)param_1[1],(void *)param_1[3],(longlong)iVar2);
    return;
  }
  if (*(char *)(*param_1 + 0xc) != '\0') {
    iVar2 = iVar2 + *(int *)((undefined8 *)param_1[1] + 2);
    iVar1 = 0;
    if (-1 < iVar2) {
      iVar1 = iVar2;
    }
    FUN_18001e280((undefined8 *)param_1[1],(longlong)iVar1,'\0');
  }
  return;
}

