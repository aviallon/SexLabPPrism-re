// PrismExport addr=0x18004e670 name=anon ghidra=FUN_18004e670 status=clean lines=43

undefined8 * FUN_18004e670(undefined8 *param_1,undefined8 param_2,int param_3)

{
  BOOL BVar1;
  undefined8 *puVar2;
  bool bVar3;
  undefined8 *local_res8 [2];
  ulonglong local_28 [4];
  
  *(undefined4 *)(param_1 + 1) = 0;
  *param_1 = spdlog::sinks::wincolor_sink<spdlog::details::console_mutex>::vftable;
  param_1[2] = param_2;
  param_1[3] = &DAT_1800958a0;
  local_res8[0] = param_1;
  local_res8[0] = (undefined8 *)FUN_180050418(0xd8);
  puVar2 = (undefined8 *)0x0;
  if (local_res8[0] != (undefined8 *)0x0) {
    local_28[1] = 0;
    local_28[2] = 2;
    local_28[3] = 0xf;
    local_28[0] = (ulonglong)DAT_180061b64;
    puVar2 = FUN_180045040(local_res8[0],0,(longlong *)local_28);
  }
  local_res8[0] = (undefined8 *)0x0;
  param_1[5] = puVar2;
  FUN_180045bb0(local_res8);
  if (param_3 == 1) {
    BVar1 = GetConsoleMode((HANDLE)param_1[2],(LPDWORD)local_res8);
    bVar3 = BVar1 != 0;
  }
  else {
    bVar3 = param_3 == 0;
  }
  *(bool *)(param_1 + 4) = bVar3;
  *(undefined4 *)(param_1 + 6) = 0x30007;
  *(undefined4 *)((longlong)param_1 + 0x34) = 0xe0002;
  *(undefined4 *)(param_1 + 7) = 0x4f000c;
  *(undefined2 *)((longlong)param_1 + 0x3c) = 0;
  return param_1;
}

