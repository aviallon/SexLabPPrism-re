// PrismExport addr=0x180014428 name=FocusRecovery::`anonymous-namespace'::StartConsolePulse ghidra=FocusRecovery::_anonymous_namespace_::StartConsolePulse status=clean lines=103

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FocusRecovery___anonymous_namespace___StartConsolePulse
               (longlong param_1,undefined8 param_2,longlong param_3,undefined8 param_4)

{
  longlong lVar1;
  undefined8 uVar2;
  longlong *plVar3;
  undefined *puVar4;
  undefined ***pppuVar5;
  bool bVar6;
  undefined1 local_res10 [8];
  char *local_78;
  undefined8 uStack_70;
  char *local_68;
  undefined4 uStack_60;
  undefined4 uStack_5c;
  char *local_58;
  undefined **local_48;
  undefined8 uStack_40;
  char *local_38;
  char local_20;
  undefined ***local_10;
  
  pppuVar5 = &local_48;
  lVar1 = param_1;
  FUN_180013920((longlong *)pppuVar5,param_1);
  if (local_20 != '\0') {
    local_78 = PTR_s_Console_180056a10;
    uStack_70 = _UNK_180056a18;
    lVar1 = FUN_180037840(pppuVar5,lVar1,param_3,param_4);
    if ((lVar1 != 0) &&
       (uVar2 = FUN_1800378a0(lVar1,&local_78,param_3,param_4), (char)uVar2 != '\0')) {
      plVar3 = (longlong *)FUN_180047e20();
      uStack_60 = 200;
      local_68 = "src\\FocusRecovery.cpp";
      uStack_5c = uStack_40._4_4_;
      uStack_40 = CONCAT44(uStack_40._4_4_,200);
      uStack_70 = 0x48;
      local_58 = 
      "void __cdecl FocusRecovery::`anonymous-namespace\'::StartConsolePulse(unsigned __int64)";
      local_78 = "FocusRecovery: console fallback skipped because Console was already open";
      local_48 = (undefined **)0x18005c0a0;
      local_38 = 
      "void __cdecl FocusRecovery::`anonymous-namespace\'::StartConsolePulse(unsigned __int64)";
      FUN_18000daf0(plVar3,(longlong *)&local_48,2,(longlong *)&local_78);
      return;
    }
    LOCK();
    bVar6 = DAT_18009c0b0 == '\0';
    if (bVar6) {
      DAT_18009c0b0 = '\x01';
    }
    UNLOCK();
    if (bVar6) {
      local_res10[0] = (**(code **)(*local_48 + 0xa0))();
      plVar3 = (longlong *)FUN_180047e20();
      uStack_60 = 0xd1;
      local_68 = "src\\FocusRecovery.cpp";
      uStack_5c = uStack_40._4_4_;
      uStack_40 = CONCAT44(uStack_40._4_4_,0xd1);
      uStack_70 = 0x5b;
      local_58 = 
      "void __cdecl FocusRecovery::`anonymous-namespace\'::StartConsolePulse(unsigned __int64)";
      local_78 = 
      "FocusRecovery: pulsing Console to rebuild Skyrim mouse/menu input state (anyPrismaFocus={})";
      local_48 = (undefined **)0x18005c0a0;
      local_38 = 
      "void __cdecl FocusRecovery::`anonymous-namespace\'::StartConsolePulse(unsigned __int64)";
      FUN_18000e940(plVar3,&local_48,3,&local_78,local_res10);
      local_78 = PTR_s_Console_180056a10;
      uStack_70 = _UNK_180056a18;
      FUN_180014140(&local_78,1);
      puVar4 = FUN_1800140c0();
      local_48 = std::
                 _Func_impl_no_alloc<`void___cdecl_FocusRecovery::_anon_A6DA2F39::StartConsolePulse(unsigned___int64)'::`2'::<lambda_1>,void>
                 ::vftable;
      local_10 = &local_48;
      uStack_40 = param_1;
      FUN_180013f20((longlong)puVar4,0xb4,(longlong *)&local_48);
    }
    else {
      plVar3 = (longlong *)FUN_180047e20();
      uStack_60 = 0xce;
      local_68 = "src\\FocusRecovery.cpp";
      uStack_5c = uStack_40._4_4_;
      uStack_40 = CONCAT44(uStack_40._4_4_,0xce);
      uStack_70 = 0x30;
      local_58 = 
      "void __cdecl FocusRecovery::`anonymous-namespace\'::StartConsolePulse(unsigned __int64)";
      local_78 = "FocusRecovery: console pulse already in progress";
      local_48 = (undefined **)0x18005c0a0;
      local_38 = 
      "void __cdecl FocusRecovery::`anonymous-namespace\'::StartConsolePulse(unsigned __int64)";
      FUN_18000daf0(plVar3,(longlong *)&local_48,2,(longlong *)&local_78);
    }
  }
  return;
}

