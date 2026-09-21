// PrismExport addr=0x18002cfd0 name=g3_13 ghidra=_anonymous_namespace_::DispatchAction::<lambda_1>::operator status=clean lines=89

void _anonymous_namespace___DispatchAction__<lambda_1>__operator
               (longlong param_1,undefined8 param_2,longlong param_3,undefined8 param_4)

{
  undefined8 *puVar1;
  ulonglong uVar2;
  undefined *puVar3;
  code *pcVar4;
  longlong *plVar5;
  char **ppcVar6;
  char *pcVar7;
  char **ppcVar8;
  undefined8 *puVar9;
  longlong lVar10;
  char *local_78;
  undefined8 local_70;
  char *local_68;
  undefined8 uStack_60;
  char *local_58;
  char *local_48;
  undefined8 uStack_40;
  char *local_38;
  undefined8 local_28;
  undefined8 local_20;
  undefined4 local_18;
  undefined8 local_10;
  
  puVar1 = (undefined8 *)(param_1 + 8);
  local_28 = 0;
  local_68 = (char *)0x108eb;
  uStack_60 = 0x10e29;
  local_58 = (char *)0x108eb;
  ppcVar6 = &local_68;
  uVar2 = REL__IDDatabase__id2offset((ulonglong *)ppcVar6,param_2,param_3,param_4);
  pcVar4 = (code *)0x0;
  if (uVar2 != 0) {
    puVar3 = FUN_18001bc40(ppcVar6,param_2,param_3,param_4);
    lVar10 = *(longlong *)(puVar3 + 0x110);
    uVar2 = REL__IDDatabase__id2offset((ulonglong *)&local_68,param_2,param_3,param_4);
    pcVar4 = (code *)(uVar2 + lVar10);
  }
  pcVar7 = "SLPPPrism_Action";
  (*pcVar4)(&local_28);
  local_20 = 0;
  puVar9 = puVar1;
  if (0xf < *(ulonglong *)(param_1 + 0x20)) {
    puVar9 = (undefined8 *)*puVar1;
  }
  if (*(longlong *)(param_1 + 0x18) != 0) {
    local_68 = (char *)0x108eb;
    uStack_60 = 0x10e29;
    local_58 = (char *)0x108eb;
    ppcVar6 = &local_68;
    uVar2 = REL__IDDatabase__id2offset((ulonglong *)ppcVar6,pcVar7,param_3,param_4);
    pcVar4 = (code *)0x0;
    if (uVar2 != 0) {
      puVar3 = FUN_18001bc40(ppcVar6,pcVar7,param_3,param_4);
      lVar10 = *(longlong *)(puVar3 + 0x110);
      uVar2 = REL__IDDatabase__id2offset((ulonglong *)&local_68,pcVar7,param_3,param_4);
      pcVar4 = (code *)(uVar2 + lVar10);
    }
    (*pcVar4)(&local_20,puVar9);
  }
  local_18 = 0;
  local_10 = 0;
  plVar5 = (longlong *)FUN_180038270();
  FUN_18002c460(plVar5,&local_28,param_3,param_4);
  plVar5 = (longlong *)FUN_180047e20();
  local_68 = "src\\main.cpp";
  uStack_60 = CONCAT44(uStack_40._4_4_,0x27b);
  local_58 = 
  "auto __cdecl `anonymous-namespace\'::DispatchAction::<lambda_1>::operator ()(void) const";
  local_78 = "UI action dispatched to Papyrus: {}";
  local_70 = 0x23;
  local_48 = "src\\main.cpp";
  uStack_40 = uStack_60;
  local_38 = 
  "auto __cdecl `anonymous-namespace\'::DispatchAction::<lambda_1>::operator ()(void) const";
  ppcVar6 = &local_78;
  lVar10 = 2;
  ppcVar8 = &local_48;
  FUN_180021ca0(plVar5,ppcVar8,2,(longlong *)ppcVar6,puVar1);
  FUN_180010460(&local_20,ppcVar8,lVar10,ppcVar6);
  FUN_180010460(&local_28,ppcVar8,lVar10,ppcVar6);
  return;
}

