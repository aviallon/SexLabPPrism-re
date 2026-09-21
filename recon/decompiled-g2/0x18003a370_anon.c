// PrismExport addr=0x18003a370 name=anon ghidra=FUN_18003a370 status=degraded lines=36

void FUN_18003a370(undefined8 param_1,undefined8 param_2,ulonglong param_3,ulonglong param_4)

{
  longlong lVar1;
  ulonglong uVar2;
  undefined *puVar3;
  ulonglong uVar4;
  code *UNRECOVERED_JUMPTABLE;
  ulonglong *puVar5;
  undefined8 uVar6;
  ulonglong uVar7;
  ulonglong local_28 [4];
  
  local_28[0] = 0x17dd2;
  puVar5 = local_28;
  local_28[1] = 0x19824;
  local_28[2] = 0x17dd2;
  uVar6 = param_2;
  uVar4 = param_3;
  uVar7 = param_4;
  uVar2 = REL__IDDatabase__id2offset(puVar5,param_2,param_3,param_4);
  UNRECOVERED_JUMPTABLE = (code *)0x0;
  if (uVar2 != 0) {
    puVar3 = FUN_18001bc40(puVar5,uVar6,uVar4,uVar7);
    lVar1 = *(longlong *)(puVar3 + 0x110);
    uVar4 = REL__IDDatabase__id2offset(local_28,uVar6,uVar4,uVar7);
    UNRECOVERED_JUMPTABLE = (code *)(uVar4 + lVar1);
  }
                    /* WARNING: Could not recover jumptable at 0x00018003a3fb. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)(param_1,param_2,param_3 & 0xffffffff,param_4 & 0xffffffff);
  return;
}

