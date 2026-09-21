// PrismExport addr=0x180039120 name=guess ghidra=FUN_180039120 status=clean lines=29

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 FUN_180039120(undefined8 param_1,undefined8 param_2,longlong param_3,undefined8 param_4)

{
  longlong lVar1;
  ulonglong uVar2;
  undefined *puVar3;
  ulonglong *puVar4;
  ulonglong local_28;
  undefined8 uStack_20;
  undefined8 local_18;
  
  puVar4 = &local_28;
  local_28 = _DAT_1800615b8;
  uStack_20 = _UNK_1800615c0;
  local_18 = DAT_1800615c8;
  uVar2 = REL__IDDatabase__id2offset(puVar4,param_2,param_3,param_4);
  if (uVar2 != 0) {
    puVar3 = FUN_18001bc40(puVar4,param_2,param_3,param_4);
    lVar1 = *(longlong *)(puVar3 + 0x110);
    uVar2 = REL__IDDatabase__id2offset(&local_28,param_2,param_3,param_4);
    return *(undefined8 *)(uVar2 + lVar1);
  }
  return uRam0000000000000000;
}

