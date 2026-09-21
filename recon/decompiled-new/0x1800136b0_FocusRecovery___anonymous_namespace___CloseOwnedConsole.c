// PrismExport addr=0x1800136b0 name=FocusRecovery::`anonymous-namespace'::CloseOwnedConsole ghidra=FocusRecovery::_anonymous_namespace_::CloseOwnedConsole status=clean lines=42

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FocusRecovery___anonymous_namespace___CloseOwnedConsole(void)

{
  char cVar1;
  longlong *plVar2;
  char *local_58;
  undefined8 uStack_50;
  char *local_48;
  undefined4 uStack_40;
  undefined4 uStack_3c;
  char *local_38;
  char *local_28;
  undefined8 uStack_20;
  char *local_18;
  
  cVar1 = DAT_18009c0b0;
  LOCK();
  DAT_18009c0b0 = 0;
  UNLOCK();
  if (cVar1 != '\0') {
    plVar2 = (longlong *)FUN_180047e20();
    uStack_40 = 0x98;
    local_48 = "src\\FocusRecovery.cpp";
    uStack_3c = uStack_20._4_4_;
    uStack_20 = CONCAT44(uStack_20._4_4_,0x98);
    uStack_50 = 0x27;
    local_38 = "void __cdecl FocusRecovery::`anonymous-namespace\'::CloseOwnedConsole(void)";
    local_58 = "FocusRecovery: closing recovery console";
    local_28 = "src\\FocusRecovery.cpp";
    local_18 = "void __cdecl FocusRecovery::`anonymous-namespace\'::CloseOwnedConsole(void)";
    FUN_18000daf0(plVar2,(longlong *)&local_28,2,(longlong *)&local_58);
    local_58 = PTR_s_Console_180056a10;
    uStack_50 = _UNK_180056a18;
    FUN_180014140(&local_58,3);
  }
  return;
}

