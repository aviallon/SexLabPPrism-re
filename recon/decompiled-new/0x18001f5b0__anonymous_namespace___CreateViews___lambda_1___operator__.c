// PrismExport addr=0x18001f5b0 name=`anonymous-namespace'::CreateViews::<lambda_1>::operator() ghidra=_anonymous_namespace_::CreateViews::<lambda_1>::operator status=clean lines=42

void _anonymous_namespace___CreateViews__<lambda_1>__operator(void)

{
  longlong *plVar1;
  longlong lVar2;
  char *local_78;
  undefined8 local_70;
  char *local_68;
  undefined8 uStack_60;
  char *local_58;
  undefined **local_48 [7];
  undefined ***local_10;
  
  LOCK();
  DAT_18009c1c8 = 1;
  UNLOCK();
  plVar1 = (longlong *)FUN_180047e20();
  uStack_60 = CONCAT44(uStack_60._4_4_,0x329);
  local_70 = 0x19;
  local_78 = "Controller view DOM ready";
  local_68 = "src\\main.cpp";
  local_58 = 
  "auto __cdecl `anonymous-namespace\'::CreateViews::<lambda_1>::operator ()(unsigned __int64) const"
  ;
  FUN_18000daf0(plVar1,(longlong *)&local_68,2,(longlong *)&local_78);
  lVar2 = FUN_1800382c0();
  local_48[0] = std::
                _Func_impl_no_alloc<`void___cdecl__anon_BB2E73B6::PushState(void)'::`2'::<lambda_1>,void>
                ::vftable;
  local_10 = local_48;
  FUN_180037e90(lVar2,(longlong *)local_48);
  lVar2 = FUN_1800382c0();
  local_48[0] = std::
                _Func_impl_no_alloc<`void___cdecl__anon_BB2E73B6::PushCompatible(void)'::`2'::<lambda_1>,void>
                ::vftable;
  local_10 = local_48;
  FUN_180037e90(lVar2,(longlong *)local_48);
  return;
}

