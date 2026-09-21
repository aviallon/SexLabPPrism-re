// PrismExport addr=0x180016d20 name=anon ghidra=FUN_180016d20 status=clean lines=23

void FUN_180016d20(longlong param_1)

{
  undefined8 uVar1;
  undefined *puVar2;
  undefined **local_48;
  undefined8 local_40;
  undefined ***local_10;
  
  uVar1 = *(undefined8 *)(param_1 + 8);
  FocusRecovery___anonymous_namespace___CloseOwnedConsole();
  puVar2 = FUN_1800140c0();
  local_48 = std::
             _Func_impl_no_alloc<`void___cdecl_FocusRecovery::_anon_A6DA2F39::CloseConsolePulse(unsigned___int64)'::`2'::<lambda_1>,void>
             ::vftable;
  local_10 = &local_48;
  local_40 = uVar1;
  FUN_180013f20((longlong)puVar2,0x78,(longlong *)&local_48);
  return;
}

