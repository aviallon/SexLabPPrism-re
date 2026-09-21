// PrismExport addr=0x180055090 name=guess ghidra=FUN_180055090 status=clean lines=28

void FUN_180055090(undefined8 param_1,undefined8 param_2,longlong param_3,undefined8 param_4)

{
  longlong lVar1;
  ulonglong uVar2;
  undefined *puVar3;
  code *pcVar4;
  ulonglong *puVar5;
  ulonglong local_28 [4];
  
  local_28[0] = 0x10907;
  local_28[1] = 0x10e48;
  local_28[2] = 0x10907;
  puVar5 = local_28;
  uVar2 = REL__IDDatabase__id2offset(puVar5,param_2,param_3,param_4);
  pcVar4 = (code *)0x0;
  if (uVar2 != 0) {
    puVar3 = FUN_18001bc40(puVar5,param_2,param_3,param_4);
    lVar1 = *(longlong *)(puVar3 + 0x110);
    uVar2 = REL__IDDatabase__id2offset(local_28,param_2,param_3,param_4);
    pcVar4 = (code *)(uVar2 + lVar1);
  }
  (*pcVar4)(&DAT_18009c2b0);
  return;
}

