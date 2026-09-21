// PrismExport addr=0x1800255c0 name=void __cdecl `anonymous-namespace'::RequestSearchInput::<lambda_1>::operator ()(void) const ghidra=_anonymous_namespace_::RequestSearchInput::<lambda_1>::opera status=clean lines=107

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void _anonymous_namespace___RequestSearchInput__<lambda_1>__opera
               (longlong *param_1,undefined8 param_2,longlong param_3,undefined8 param_4)

{
  char cVar1;
  longlong lVar2;
  ulonglong uVar3;
  undefined *puVar4;
  longlong *plVar5;
  char **ppcVar6;
  undefined8 uVar7;
  code *pcVar8;
  longlong lVar9;
  undefined1 auStack_d8 [32];
  char *local_b8;
  undefined8 local_b0;
  char *local_a8;
  undefined8 uStack_a0;
  char *local_98;
  undefined **local_88;
  char *local_80;
  undefined8 uStack_78;
  char *local_70;
  undefined8 uStack_68;
  undefined ***local_50;
  char *local_48;
  undefined8 uStack_40;
  char *local_38;
  undefined8 uStack_30;
  ulonglong local_28;
  
  local_28 = DAT_180095900 ^ (ulonglong)auStack_d8;
  plVar5 = param_1;
  if ((((DAT_18009c1c9 != '\0') && (DAT_180095120 == '\0')) &&
      (plVar5 = DAT_18009c1b0, DAT_18009c1b0 != (longlong *)0x0)) &&
     (param_2 = DAT_18009c1c0, cVar1 = (**(code **)(*DAT_18009c1b0 + 0x60))(), cVar1 != '\0')) {
    plVar5 = DAT_18009c1b0;
    uVar7 = DAT_18009c1c0;
    cVar1 = (**(code **)(*DAT_18009c1b0 + 0x20))();
    if (cVar1 != '\0') {
      plVar5 = DAT_18009c1b0;
      uVar7 = DAT_18009c1c0;
      (**(code **)(*DAT_18009c1b0 + 0x30))();
    }
    lVar2 = FUN_180037ae0(plVar5,uVar7,param_3,param_4);
    if (lVar2 != 0) {
      pcVar8 = (code *)0x0;
      local_b8 = (char *)0x0;
      local_a8 = (char *)0x108eb;
      uStack_a0 = 0x10e29;
      local_98 = (char *)0x108eb;
      ppcVar6 = &local_a8;
      uVar3 = REL__IDDatabase__id2offset((ulonglong *)ppcVar6,uVar7,param_3,param_4);
      if (uVar3 != 0) {
        puVar4 = FUN_18001bc40(ppcVar6,uVar7,param_3,param_4);
        lVar9 = *(longlong *)(puVar4 + 0x110);
        uVar3 = REL__IDDatabase__id2offset((ulonglong *)&local_a8,uVar7,param_3,param_4);
        pcVar8 = (code *)(lVar9 + uVar3);
      }
      (*pcVar8)(&local_b8,"PrismaUI_FocusMenu");
      uVar7 = 0;
      lVar9 = 4;
      ppcVar6 = &local_b8;
      FUN_180037a40(lVar2,ppcVar6,4,0);
      FUN_180010460(&local_b8,ppcVar6,lVar9,uVar7);
    }
    plVar5 = (longlong *)FUN_180047e20();
    local_a8 = "src\\main.cpp";
    uStack_a0 = CONCAT44(uStack_40._4_4_,0x29b);
    local_98 = 
    "void __cdecl `anonymous-namespace\'::RequestSearchInput::<lambda_1>::operator ()(void) const";
    local_b8 = "Modal focus transfer: Prisma unfocused and FocusMenu hide requested";
    local_b0 = 0x43;
    local_48 = "src\\main.cpp";
    uStack_40 = uStack_a0;
    local_38 = 
    "void __cdecl `anonymous-namespace\'::RequestSearchInput::<lambda_1>::operator ()(void) const";
    FUN_18000daf0(plVar5,(longlong *)&local_48,2,(longlong *)&local_b8);
    lVar2 = FUN_1800382c0();
    FUN_18000fca0(&local_48,param_1);
    local_88 = std::
               _Func_impl_no_alloc<`public:___cdecl_`void___cdecl__anon_BB2E73B6::RequestSearchInput(char_const*___ptr64)'::`2'::<lambda_1>::operator()(void)const___ptr64'::`2'::<lambda_1>,void>
               ::vftable;
    local_80 = local_48;
    uStack_78 = uStack_40;
    local_70 = local_38;
    uStack_68 = uStack_30;
    local_38 = (char *)_DAT_18005c8e0;
    uStack_30 = _UNK_18005c8e8;
    local_48 = (char *)((ulonglong)local_48 & 0xffffffffffffff00);
    local_50 = &local_88;
    FUN_180037df0(lVar2,(longlong *)&local_88);
    FUN_180019420((longlong *)&local_48);
    return;
  }
  LOCK();
  DAT_18009c1cb = 0;
  UNLOCK();
  _anonymous_namespace___ApplyPresentation(plVar5,param_2,param_3,param_4);
  return;
}

