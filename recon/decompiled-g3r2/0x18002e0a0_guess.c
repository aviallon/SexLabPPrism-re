// PrismExport addr=0x18002e0a0 name=guess ghidra=FUN_18002e0a0 status=clean lines=45

undefined * FUN_18002e0a0(undefined8 param_1,undefined8 param_2,longlong param_3,undefined8 param_4)

{
  longlong lVar1;
  ulonglong uVar2;
  undefined *puVar3;
  longlong lVar4;
  ulonglong *puVar6;
  code *pcVar7;
  ulonglong local_28 [4];
  code *pcVar5;
  
  local_28[0] = 0x2b25;
  puVar6 = local_28;
  local_28[1] = 0x2b85;
  local_28[2] = 0x2b25;
  uVar2 = REL__IDDatabase__id2offset(puVar6,param_2,param_3,param_4);
  pcVar7 = (code *)0x0;
  pcVar5 = pcVar7;
  if (uVar2 != 0) {
    puVar3 = FUN_18001bc40(puVar6,param_2,param_3,param_4);
    lVar4 = *(longlong *)(puVar3 + 0x110);
    uVar2 = REL__IDDatabase__id2offset(local_28,param_2,param_3,param_4);
    pcVar5 = (code *)(uVar2 + lVar4);
  }
  lVar4 = (*pcVar5)();
  if (lVar4 != 0) {
    puVar6 = local_28;
    local_28[0] = 0x1052b;
    local_28[1] = 0x10a13;
    local_28[2] = 0x1052b;
    uVar2 = REL__IDDatabase__id2offset(puVar6,param_2,param_3,param_4);
    if (uVar2 != 0) {
      puVar3 = FUN_18001bc40(puVar6,param_2,param_3,param_4);
      lVar1 = *(longlong *)(puVar3 + 0x110);
      uVar2 = REL__IDDatabase__id2offset(local_28,param_2,param_3,param_4);
      pcVar7 = (code *)(lVar1 + uVar2);
    }
    pcVar7 = (code *)(*pcVar7)(lVar4,param_1,0,0);
  }
  return pcVar7;
}

