// PrismExport addr=0x180013760 name=FocusRecovery::`anonymous-namespace'::Finalize ghidra=FocusRecovery::_anonymous_namespace_::Finalize status=clean lines=101

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FocusRecovery___anonymous_namespace___Finalize
               (longlong param_1,undefined8 param_2,longlong param_3,undefined8 param_4)

{
  longlong *plVar1;
  undefined8 uVar2;
  longlong lVar3;
  longlong lVar4;
  longlong *plVar5;
  char **ppcVar6;
  byte local_res10 [8];
  byte local_res18 [8];
  undefined1 local_res20 [8];
  undefined1 local_78;
  undefined1 local_77;
  undefined1 local_76 [2];
  undefined4 local_74 [3];
  char *local_68;
  undefined8 uStack_60;
  char *local_58;
  char *local_48;
  undefined8 uStack_40;
  longlong *local_38;
  undefined8 uStack_30;
  char *local_28;
  char local_10;
  
  FUN_180013920((longlong *)&local_38,param_1);
  if (local_10 == '\0') {
    FocusRecovery___anonymous_namespace___CloseOwnedConsole();
    return;
  }
  ppcVar6 = uStack_30;
  local_76[0] = (**(code **)(*local_38 + 0x20))(local_38);
  local_77 = (**(code **)(*local_38 + 0xa0))();
  local_48 = PTR_s_PrismaUI_FocusMenu_180056a00;
  uStack_40 = _UNK_180056a08;
  plVar1 = (longlong *)FUN_180037840(local_38,ppcVar6,param_3,param_4);
  plVar5 = local_38;
  if (plVar1 == (longlong *)0x0) {
LAB_1800137e1:
    local_78 = 0;
  }
  else {
    ppcVar6 = &local_48;
    uVar2 = FUN_1800378a0((longlong)plVar1,ppcVar6,param_3,param_4);
    plVar5 = plVar1;
    if ((char)uVar2 == '\0') goto LAB_1800137e1;
    local_78 = 1;
  }
  local_68 = PTR_s_Console_180056a10;
  uStack_60 = _UNK_180056a18;
  plVar1 = (longlong *)FUN_180037840(plVar5,ppcVar6,param_3,param_4);
  if (plVar1 != (longlong *)0x0) {
    ppcVar6 = &local_68;
    uVar2 = FUN_1800378a0((longlong)plVar1,ppcVar6,param_3,param_4);
    plVar5 = plVar1;
    if ((char)uVar2 != '\0') {
      local_res20[0] = 1;
      goto LAB_180013811;
    }
  }
  local_res20[0] = 0;
LAB_180013811:
  lVar3 = FUN_180037490(plVar5,ppcVar6,param_3,param_4);
  lVar4 = FUN_1800374f0(plVar5,ppcVar6,param_3,param_4);
  if (lVar4 == 0) {
    local_74[0] = 0xfffffc19;
  }
  else {
    lVar4 = FUN_1800374f0(plVar5,ppcVar6,param_3,param_4);
    local_74[0] = *(undefined4 *)(lVar4 + 0x2c);
  }
  if (lVar3 == 0) {
    local_res10[0] = 0;
    local_res18[0] = 0;
  }
  else {
    local_res10[0] = *(byte *)(lVar3 + 0x118) & 1;
    local_res18[0] = (byte)(*(uint *)(lVar3 + 0x118) >> 1) & 1;
  }
  plVar5 = (longlong *)FUN_180047e20();
  local_68 = "src\\FocusRecovery.cpp";
  uStack_60 = CONCAT44(uStack_30._4_4_,0xb0);
  uStack_40 = 0x6a;
  local_58 = "void __cdecl FocusRecovery::`anonymous-namespace\'::Finalize(unsigned __int64)";
  local_48 = 
  "FocusRecovery: final ownFocus={} anyFocus={} focusMenu={} console={} looking={} movement={} cursorCount={}"
  ;
  local_38 = (longlong *)0x18005c0a0;
  uStack_30 = (char **)uStack_60;
  local_28 = "void __cdecl FocusRecovery::`anonymous-namespace\'::Finalize(unsigned __int64)";
  FUN_18000e310(plVar5,&local_38,2,&local_48,local_76,&local_77,&local_78,local_res20,local_res18,
                local_res10,local_74);
  return;
}

