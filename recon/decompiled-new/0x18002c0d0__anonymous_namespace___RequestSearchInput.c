// PrismExport addr=0x18002c0d0 name=`anonymous-namespace'::RequestSearchInput ghidra=_anonymous_namespace_::RequestSearchInput status=clean lines=86

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void _anonymous_namespace___RequestSearchInput(char *param_1)

{
  longlong *plVar1;
  size_t sVar2;
  longlong lVar3;
  char *_Str;
  bool bVar4;
  undefined1 auStack_d8 [48];
  char *local_a8;
  undefined8 local_a0;
  undefined **local_98;
  char *local_90;
  undefined8 uStack_88;
  undefined8 local_80;
  undefined8 uStack_78;
  undefined ***local_60;
  char *local_58;
  undefined8 uStack_50;
  char *local_48;
  undefined8 uStack_40;
  char *local_38;
  undefined8 uStack_30;
  char *local_28;
  undefined8 uStack_20;
  ulonglong local_18;
  
  local_18 = DAT_180095900 ^ (ulonglong)auStack_d8;
  if ((((DAT_18009c1c9 != '\0') && (DAT_180095120 == '\0')) && (DAT_18009c1ca == '\0')) &&
     ((DAT_18009c1b0 != 0 && (DAT_18009c1c0 != 0)))) {
    LOCK();
    bVar4 = DAT_18009c1cb == '\0';
    if (bVar4) {
      DAT_18009c1cb = '\x01';
    }
    UNLOCK();
    if (bVar4) {
      _Str = "";
      if (param_1 != (char *)0x0) {
        _Str = param_1;
      }
      local_58 = (char *)0x0;
      uStack_50 = 0;
      local_48 = (char *)0x0;
      uStack_40 = 0;
      sVar2 = strlen(_Str);
      FUN_180001670(&local_58,_Str,sVar2);
      lVar3 = FUN_1800382c0();
      uStack_78 = uStack_40;
      local_80 = local_48;
      local_90 = local_58;
      uStack_30 = uStack_50;
      local_48 = (char *)_DAT_18005c8e0;
      uStack_40 = _UNK_18005c8e8;
      local_58 = (char *)((ulonglong)local_58 & 0xffffffffffffff00);
      local_98 = std::
                 _Func_impl_no_alloc<`void___cdecl__anon_BB2E73B6::RequestSearchInput(char_const*___ptr64)'::`2'::<lambda_1>,void>
                 ::vftable;
      uStack_88 = uStack_50;
      local_28 = (char *)_DAT_18005c8e0;
      uStack_20 = _UNK_18005c8e8;
      local_38 = (char *)((ulonglong)local_90 & 0xffffffffffffff00);
      local_60 = &local_98;
      FUN_180037e90(lVar3,(longlong *)&local_98);
    }
    else {
      plVar1 = (longlong *)FUN_180047e20();
      local_38 = "src\\main.cpp";
      uStack_30 = CONCAT44(uStack_50._4_4_,0x287);
      local_28 = "void __cdecl `anonymous-namespace\'::RequestSearchInput(const char *)";
      local_a8 = "Modal search request ignored: menu already open";
      local_a0 = 0x2f;
      local_58 = "src\\main.cpp";
      uStack_50 = uStack_30;
      local_48 = "void __cdecl `anonymous-namespace\'::RequestSearchInput(const char *)";
      FUN_18000daf0(plVar1,(longlong *)&local_58,2,(longlong *)&local_a8);
    }
  }
  return;
}

