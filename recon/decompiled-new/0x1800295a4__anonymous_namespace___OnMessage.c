// PrismExport addr=0x1800295a4 name=`anonymous-namespace'::OnMessage ghidra=FUN_180029200 status=clean lines=137

void FUN_180029200(longlong param_1)

{
  HMODULE pHVar1;
  FARPROC pFVar2;
  longlong *plVar3;
  longlong *plVar4;
  longlong lVar5;
  undefined **ppuVar6;
  longlong lVar7;
  char **ppcVar8;
  char *local_58;
  undefined8 local_50;
  char *local_48;
  undefined4 uStack_40;
  undefined4 uStack_3c;
  char *local_38;
  char *local_28;
  undefined8 uStack_20;
  char *local_18;
  
  if (*(int *)(param_1 + 8) != 8) {
    return;
  }
  pHVar1 = GetModuleHandleA("PrismaUI.dll");
  if ((pHVar1 == (HMODULE)0x0) ||
     (pFVar2 = GetProcAddress(pHVar1,"RequestPluginAPI"), pFVar2 == (FARPROC)0x0)) {
    DAT_18009c1b8 = (longlong *)0x0;
LAB_180029273:
    DAT_18009c1b0 = DAT_18009c1b8;
    pHVar1 = GetModuleHandleA("PrismaUI.dll");
    if ((pHVar1 == (HMODULE)0x0) ||
       (pFVar2 = GetProcAddress(pHVar1,"RequestPluginAPI"), pFVar2 == (FARPROC)0x0)) {
      DAT_18009c1b0 = (longlong *)0x0;
    }
    else {
      DAT_18009c1b0 = (longlong *)(*pFVar2)(0);
      if (DAT_18009c1b0 != (longlong *)0x0) goto LAB_1800292b9;
    }
    plVar3 = (longlong *)FUN_180047e20();
    uStack_3c = uStack_20._4_4_;
    local_48 = "src\\main.cpp";
    local_38 = 
    "void __cdecl `anonymous-namespace\'::OnMessage(struct SKSE::MessagingInterface::Message *)";
    local_58 = "PrismaUI API not found";
    uStack_40 = 0x35c;
    uStack_20 = CONCAT44(uStack_20._4_4_,0x35c);
    local_50 = 0x16;
    local_18 = 
    "void __cdecl `anonymous-namespace\'::OnMessage(struct SKSE::MessagingInterface::Message *)";
    local_28 = "src\\main.cpp";
    FUN_18000daf0(plVar3,(longlong *)&local_28,5,(longlong *)&local_58);
  }
  else {
    DAT_18009c1b0 = (longlong *)(*pFVar2)(CONCAT71((int7)((ulonglong)pHVar1 >> 8),1));
    DAT_18009c1b8 = DAT_18009c1b0;
    if (DAT_18009c1b0 == (longlong *)0x0) goto LAB_180029273;
LAB_1800292b9:
    DAT_18009c1c0 =
         (**(code **)*DAT_18009c1b0)
                   (DAT_18009c1b0,"SexLabPPrism/controller-0.6.1.html",
                    _anonymous_namespace___CreateViews__<lambda_1>__operator);
    (**(code **)(*DAT_18009c1b0 + 0x70))(DAT_18009c1b0,DAT_18009c1c0,0x50);
    if (DAT_18009c1b8 != (longlong *)0x0) {
      (**(code **)(*DAT_18009c1b8 + 0xa8))
                (DAT_18009c1b8,DAT_18009c1c0,_anonymous_namespace___OnConsoleMessage);
    }
    (**(code **)(*DAT_18009c1b0 + 0x18))
              (DAT_18009c1b0,DAT_18009c1c0,"slppReady",
               _anonymous_namespace___CreateViews__<lambda_2>__operator);
    (**(code **)(*DAT_18009c1b0 + 0x18))
              (DAT_18009c1b0,DAT_18009c1c0,"slppAction",_anonymous_namespace___SendAction);
    (**(code **)(*DAT_18009c1b0 + 0x18))
              (DAT_18009c1b0,DAT_18009c1c0,"slppSearchRequest",
               _anonymous_namespace___RequestSearchInput);
    (**(code **)(*DAT_18009c1b0 + 0x18))
              (DAT_18009c1b0,DAT_18009c1c0,"slppCollapsed",_anonymous_namespace___SetCollapsed);
    (**(code **)(*DAT_18009c1b0 + 0x18))
              (DAT_18009c1b0,DAT_18009c1c0,"slppLog",
               _anonymous_namespace___CreateViews__<lambda_4>__operator);
    (**(code **)(*DAT_18009c1b0 + 0x18))
              (DAT_18009c1b0,DAT_18009c1c0,"slppCatalogRetry",
               _anonymous_namespace___CreateViews__<lambda_5>__operator);
    (**(code **)(*DAT_18009c1b0 + 0x40))(DAT_18009c1b0,DAT_18009c1c0);
    plVar3 = (longlong *)FUN_180047e20();
    uStack_3c = uStack_20._4_4_;
    ppcVar8 = &local_58;
    local_48 = "src\\main.cpp";
    local_38 = "void __cdecl `anonymous-namespace\'::CreateViews(void)";
    ppuVar6 = &local_28;
    local_58 = "Single controller/HUD view created: {}";
    lVar7 = 2;
    uStack_40 = 0x34c;
    uStack_20 = CONCAT44(uStack_20._4_4_,0x34c);
    local_50 = 0x26;
    local_28 = "src\\main.cpp";
    local_18 = "void __cdecl `anonymous-namespace\'::CreateViews(void)";
    FUN_180022130(plVar3,(longlong *)ppuVar6,2,(longlong *)ppcVar8,&DAT_18009c1c0);
    plVar4 = (longlong *)FUN_1800390b0(plVar3,ppuVar6,lVar7,ppcVar8);
    if (plVar4 != (longlong *)0x0) {
      if ((*(int *)(*(longlong *)((longlong)ThreadLocalStoragePointer + (ulonglong)_tls_index * 8) +
                   0x34) < DAT_18009c240) && (FUN_180050874(&DAT_18009c240), DAT_18009c240 == -1)) {
        atexit(FUN_180054f70);
        _Init_thread_footer(&DAT_18009c240);
      }
      ppuVar6 = &PTR_vftable_180095110;
      FUN_180025bc0(plVar4,0x180095110,lVar7,ppcVar8);
      plVar3 = plVar4;
    }
    lVar5 = FUN_180037840(plVar3,ppuVar6,lVar7,ppcVar8);
    if (lVar5 != 0) {
      if ((*(int *)(*(longlong *)((longlong)ThreadLocalStoragePointer + (ulonglong)_tls_index * 8) +
                   0x34) < DAT_18009c244) && (FUN_180050874(&DAT_18009c244), DAT_18009c244 == -1)) {
        atexit(FUN_180054f80);
        _Init_thread_footer(&DAT_18009c244);
      }
      FUN_180025bc0((longlong *)(lVar5 + 8),0x180095118,lVar7,ppcVar8);
    }
    plVar3 = (longlong *)FUN_180047e20();
    uStack_3c = uStack_20._4_4_;
    local_48 = "src\\main.cpp";
    local_38 = 
    "void __cdecl `anonymous-namespace\'::OnMessage(struct SKSE::MessagingInterface::Message *)";
    local_58 = "SexLab P+ Prism ready";
    uStack_40 = 0x366;
    uStack_20 = CONCAT44(uStack_20._4_4_,0x366);
    local_50 = 0x15;
    local_18 = 
    "void __cdecl `anonymous-namespace\'::OnMessage(struct SKSE::MessagingInterface::Message *)";
    local_28 = "src\\main.cpp";
    FUN_18000daf0(plVar3,(longlong *)&local_28,2,(longlong *)&local_58);
  }
  return;
}

