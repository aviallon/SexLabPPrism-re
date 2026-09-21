// PrismExport addr=0x180014140 name=anon ghidra=FUN_180014140 status=clean lines=104

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

void FUN_180014140(undefined8 *param_1,undefined4 param_2)

{
  undefined8 ***pppuVar1;
  ulonglong uVar2;
  ulonglong uVar3;
  longlong lVar4;
  ulonglong uVar5;
  undefined8 *puVar6;
  undefined8 ***_Memory;
  ulonglong uVar7;
  undefined8 ****ppppuVar8;
  undefined1 auStackY_f8 [32];
  longlong local_b8 [7];
  undefined8 *local_80;
  undefined8 **local_78;
  undefined8 **ppuStack_70;
  ulonglong local_68;
  ulonglong uStack_60;
  undefined4 local_58;
  undefined8 ***local_50;
  undefined8 uStack_48;
  ulonglong local_40;
  ulonglong local_38;
  ulonglong local_30;
  
  local_30 = DAT_180095900 ^ (ulonglong)auStackY_f8;
  lVar4 = FUN_1800382c0();
  if (lVar4 != 0) {
    local_50 = (undefined8 ****)0x0;
    uStack_48 = 0;
    local_40 = 0;
    local_38 = 0;
    FUN_180001670(&local_50,(void *)*param_1,param_1[1]);
    uVar3 = local_38;
    uVar2 = local_40;
    pppuVar1 = local_50;
    local_78 = (undefined8 **)0x0;
    ppuStack_70 = (undefined8 ***)0x0;
    local_68 = 0;
    uStack_60 = 0;
    ppppuVar8 = &local_50;
    if (0xf < local_38) {
      ppppuVar8 = (undefined8 ****)local_50;
    }
    if (0x7fffffffffffffff < local_40) {
                    /* WARNING: Subroutine does not return */
      FUN_180019860();
    }
    if (local_40 < 0x10) {
      local_68 = local_40;
      uStack_60 = 0xf;
      local_78 = *ppppuVar8;
      ppuStack_70 = ppppuVar8[1];
    }
    else {
      uVar5 = local_40 | 0xf;
      uVar7 = 0x7fffffffffffffff;
      if ((uVar5 < 0x8000000000000000) && (uVar7 = uVar5, uVar5 < 0x16)) {
        uVar7 = 0x16;
      }
      local_78 = (undefined8 **)FUN_1800013a0(uVar7 + 1);
      local_68 = uVar2;
      uStack_60 = uVar7;
      memcpy(local_78,ppppuVar8,uVar2 + 1);
    }
    local_80 = (undefined8 *)0x0;
    local_58 = param_2;
    puVar6 = (undefined8 *)FUN_180050418(0x30);
    *puVar6 = std::
              _Func_impl_no_alloc<`void___cdecl_FocusRecovery::_anon_A6DA2F39::SendMenuMessage(std::basic_string_view<char,std::char_traits<char>_>,RE::UI_MESSAGE_TYPE)'::`2'::<lambda_1>,void>
              ::vftable;
    FUN_18000fca0(puVar6 + 1,&local_78);
    *(undefined4 *)(puVar6 + 5) = local_58;
    local_80 = puVar6;
    FUN_180037e90(lVar4,local_b8);
    if (0xf < uStack_60) {
      _Memory = (undefined8 ***)local_78;
      if ((0xfff < uStack_60 + 1) &&
         (_Memory = (undefined8 ***)local_78[-1],
         0x1f < (ulonglong)((longlong)local_78 + (-8 - (longlong)_Memory)))) {
                    /* WARNING: Subroutine does not return */
        _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
      }
      free(_Memory);
    }
    if (0xf < uVar3) {
      ppppuVar8 = (undefined8 ****)pppuVar1;
      if ((0xfff < uVar3 + 1) &&
         (ppppuVar8 = (undefined8 ****)pppuVar1[-1],
         0x1f < (ulonglong)((longlong)pppuVar1 + (-8 - (longlong)ppppuVar8)))) {
                    /* WARNING: Subroutine does not return */
        _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
      }
      free(ppppuVar8);
    }
  }
  return;
}

