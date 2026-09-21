// PrismExport addr=0x1800145c5 name=FocusRecovery::`anonymous-namespace'::VerifyCleanup ghidra=FUN_180014590 status=clean lines=146

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_180014590(longlong param_1,int param_2,longlong param_3,char **param_4)

{
  bool bVar1;
  char cVar2;
  char cVar3;
  longlong lVar4;
  undefined8 uVar5;
  longlong *plVar6;
  undefined *puVar7;
  undefined **ppuVar8;
  char **ppcVar9;
  char cVar10;
  int local_res20 [2];
  char *local_b8;
  undefined8 uStack_b0;
  char *local_a8;
  undefined4 uStack_a0;
  undefined4 uStack_9c;
  char *local_98;
  char *local_88;
  undefined8 uStack_80;
  char *local_78;
  undefined **local_68;
  char **local_60;
  int local_58;
  char local_54;
  undefined2 local_53;
  undefined1 local_51;
  char local_40;
  undefined ***local_30;
  
  cVar10 = (char)param_3;
  FUN_180013920((longlong *)&local_68,param_1);
  if (local_40 == '\0') {
    return;
  }
  cVar2 = (**(code **)(*local_68 + 0x20))(local_68,local_60);
  ppcVar9 = *(char ***)(*local_68 + 0xa0);
  ppuVar8 = local_68;
  cVar3 = (*(code *)ppcVar9)();
  local_b8 = PTR_s_PrismaUI_FocusMenu_180056a00;
  uStack_b0 = _UNK_180056a08;
  lVar4 = FUN_180037840(ppuVar8,ppcVar9,param_3,param_4);
  if (lVar4 == 0) {
LAB_180014633:
    bVar1 = false;
  }
  else {
    ppcVar9 = &local_b8;
    uVar5 = FUN_1800378a0(lVar4,ppcVar9,param_3,param_4);
    if ((char)uVar5 == '\0') goto LAB_180014633;
    bVar1 = true;
  }
  if (cVar2 == '\0') {
    if (cVar3 != '\0') {
      plVar6 = (longlong *)FUN_180047e20();
      uStack_9c = uStack_80._4_4_;
      param_4 = &local_b8;
      local_a8 = "src\\FocusRecovery.cpp";
      local_98 = 
      "void __cdecl FocusRecovery::`anonymous-namespace\'::VerifyCleanup(unsigned __int64,int,bool)"
      ;
      ppcVar9 = &local_88;
      local_b8 = "FocusRecovery: another Prisma view acquired focus; using Console yield";
      param_3 = 3;
      uStack_a0 = 0xec;
      uStack_80 = CONCAT44(uStack_80._4_4_,0xec);
      uStack_b0 = 0x46;
      local_78 = 
      "void __cdecl FocusRecovery::`anonymous-namespace\'::VerifyCleanup(unsigned __int64,int,bool)"
      ;
      local_88 = "src\\FocusRecovery.cpp";
      FUN_18000daf0(plVar6,(longlong *)ppcVar9,3,(longlong *)param_4);
      goto LAB_1800146ab;
    }
  }
  else if (param_2 < 3) {
    local_res20[0] = param_2 + 1;
    plVar6 = (longlong *)FUN_180047e20();
    uStack_9c = uStack_80._4_4_;
    param_4 = &local_b8;
    local_a8 = "src\\FocusRecovery.cpp";
    local_98 = 
    "void __cdecl FocusRecovery::`anonymous-namespace\'::VerifyCleanup(unsigned __int64,int,bool)";
    local_b8 = "FocusRecovery: view still focused during verification, retrying Unfocus ({})";
    param_3 = 3;
    uStack_a0 = 0xf2;
    uStack_80 = CONCAT44(uStack_80._4_4_,0xf2);
    uStack_b0 = 0x4c;
    local_88 = "src\\FocusRecovery.cpp";
    local_78 = 
    "void __cdecl FocusRecovery::`anonymous-namespace\'::VerifyCleanup(unsigned __int64,int,bool)";
    FUN_18000e690(plVar6,&local_88,3,param_4,local_res20);
    (**(code **)(*local_68 + 0x30))(local_68);
    ppcVar9 = local_60;
  }
  if (bVar1) {
    local_res20[0] = param_2 + 1;
    plVar6 = (longlong *)FUN_180047e20();
    param_4 = &local_b8;
    uStack_9c = uStack_80._4_4_;
    local_a8 = "src\\FocusRecovery.cpp";
    local_98 = 
    "void __cdecl FocusRecovery::`anonymous-namespace\'::VerifyCleanup(unsigned __int64,int,bool)";
    param_3 = 3;
    local_b8 = "FocusRecovery: FocusMenu still open, requesting ForceHide ({})";
    uStack_a0 = 0xf6;
    uStack_80 = CONCAT44(uStack_80._4_4_,0xf6);
    uStack_b0 = 0x3e;
    local_78 = 
    "void __cdecl FocusRecovery::`anonymous-namespace\'::VerifyCleanup(unsigned __int64,int,bool)";
    local_88 = "src\\FocusRecovery.cpp";
    FUN_18000e690(plVar6,&local_88,3,param_4,local_res20);
    ppcVar9 = (char **)0x4;
    local_b8 = PTR_s_PrismaUI_FocusMenu_180056a00;
    uStack_b0 = _UNK_180056a08;
    FUN_180014140(&local_b8,4);
  }
  if (((cVar2 != '\0') || (bVar1)) && (param_2 < 3)) {
    puVar7 = FUN_1800140c0();
    local_68 = std::
               _Func_impl_no_alloc<`void___cdecl_FocusRecovery::_anon_A6DA2F39::VerifyCleanup(unsigned___int64,int,bool)'::`17'::<lambda_1>,void>
               ::vftable;
    local_53 = uStack_b0._5_2_;
    local_51 = uStack_b0._7_1_;
    local_30 = &local_68;
    local_60 = (char **)param_1;
    local_58 = param_2;
    local_54 = cVar10;
    FUN_180013f20((longlong)puVar7,0x50,(longlong *)&local_68);
    return;
  }
  if (((cVar10 == '\0') && (cVar2 == '\0')) && (!bVar1)) {
    FocusRecovery___anonymous_namespace___Finalize(param_1,ppcVar9,param_3,param_4);
    return;
  }
LAB_1800146ab:
  FocusRecovery___anonymous_namespace___StartConsolePulse(param_1,ppcVar9,param_3,param_4);
  return;
}

