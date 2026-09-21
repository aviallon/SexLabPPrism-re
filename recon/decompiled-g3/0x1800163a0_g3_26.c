// PrismExport addr=0x1800163a0 name=g3_26 ghidra=FUN_1800163a0 status=clean lines=16

undefined8 * FUN_1800163a0(longlong param_1)

{
  undefined8 *puVar1;
  
  puVar1 = (undefined8 *)FUN_180050418(0x30);
  *puVar1 = std::
            _Func_impl_no_alloc<`void___cdecl_FocusRecovery::_anon_A6DA2F39::SendMenuMessage(std::basic_string_view<char,std::char_traits<char>_>,RE::UI_MESSAGE_TYPE)'::`2'::<lambda_1>,void>
            ::vftable;
  FUN_18000fca0(puVar1 + 1,(undefined8 *)(param_1 + 8));
  *(undefined4 *)(puVar1 + 5) = *(undefined4 *)(param_1 + 0x28);
  return puVar1;
}

