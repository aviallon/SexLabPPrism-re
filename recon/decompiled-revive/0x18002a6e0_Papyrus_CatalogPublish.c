// PrismExport addr=0x18002a6e0 name=Papyrus_CatalogPublish ghidra=FUN_18002a6e0 status=clean lines=50

void FUN_18002a6e0(void)

{
  int iVar1;
  longlong *plVar2;
  longlong lVar3;
  longlong lVar4;
  longlong local_res10 [3];
  char *local_78;
  undefined8 local_70;
  char *local_68;
  undefined8 uStack_60;
  char *local_58;
  undefined **local_48;
  longlong local_40;
  undefined ***local_10;
  
  iVar1 = _Mtx_lock(&DAT_1800951f0);
  if (iVar1 != 0) {
                    /* WARNING: Subroutine does not return */
    std::_Throw_Cpp_error(5);
  }
  if (DAT_18009523c != 0x7fffffff) {
    lVar4 = DAT_18009c1e0 - DAT_18009c1d8 >> 7;
    local_res10[0] = lVar4;
    _Mtx_unlock(&DAT_1800951f0);
    plVar2 = (longlong *)FUN_180047e20();
    uStack_60 = CONCAT44(uStack_60._4_4_,0x241);
    local_70 = 0x35;
    local_78 = "Catalog session cache reused: {} scenes already in UI";
    local_68 = "src\\main.cpp";
    local_58 = 
    "void __cdecl `anonymous-namespace\'::Papyrus_CatalogPublish(struct RE::StaticFunctionTag *)";
    FUN_180022130(plVar2,(longlong *)&local_68,2,(longlong *)&local_78,local_res10);
    lVar3 = FUN_1800382c0();
    local_48 = std::
               _Func_impl_no_alloc<`void___cdecl__anon_BB2E73B6::Papyrus_CatalogPublish(RE::StaticFunctionTag*___ptr64)'::`2'::<lambda_1>,void>
               ::vftable;
    local_10 = &local_48;
    local_40 = lVar4;
    FUN_180037e90(lVar3,(longlong *)&local_48);
    return;
  }
  DAT_18009523c = 0x7ffffffe;
                    /* WARNING: Subroutine does not return */
  std::_Throw_Cpp_error(6);
}

