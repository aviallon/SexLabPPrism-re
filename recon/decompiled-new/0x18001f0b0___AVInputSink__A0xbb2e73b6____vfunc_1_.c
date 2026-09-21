// PrismExport addr=0x18001f0b0 name=.?AVInputSink@?A0xbb2e73b6@@::vfunc[1] ghidra=_anonymous_namespace_::HandleFocusHotkey status=clean lines=95

undefined8 _anonymous_namespace___HandleFocusHotkey(undefined8 param_1,longlong *param_2)

{
  char cVar1;
  longlong lVar2;
  longlong *plVar3;
  longlong lVar4;
  char *local_res10 [3];
  char *local_88;
  undefined8 local_80;
  char *local_78;
  undefined8 uStack_70;
  char *local_68;
  undefined **local_58 [7];
  undefined ***local_20;
  
  if (param_2 != (longlong *)0x0) {
    for (lVar4 = *param_2; lVar4 != 0; lVar4 = *(longlong *)(lVar4 + 0x10)) {
      lVar2 = FUN_180039110(lVar4);
      cVar1 = DAT_180095120;
      if ((((lVar2 != 0) && (*(int *)(lVar2 + 8) == 0)) && (0.0 < *(float *)(lVar2 + 0x28))) &&
         (*(float *)(lVar2 + 0x2c) == 0.0)) {
        if ((*(int *)(lVar2 + 0x20) == 0x29) && (DAT_18009c1c9 != '\0')) {
          plVar3 = (longlong *)FUN_180047e20();
          uStack_70 = CONCAT44(uStack_70._4_4_,0x2eb);
          local_80 = 0x37;
          local_88 = "Developer Console hotkey suppressed during active scene";
          local_78 = "src\\main.cpp";
          local_68 = 
          "enum RE::BSEventNotifyControl __cdecl `anonymous-namespace\'::InputSink::ProcessEvent(class RE::InputEvent *const *,class RE::BSTEventSource<class RE::InputEvent *> *)"
          ;
          FUN_18000daf0(plVar3,(longlong *)&local_78,2,(longlong *)&local_88);
          return 1;
        }
        if (DAT_18009c1cb == '\0') {
          if (*(int *)(lVar2 + 0x20) == 0x3e) {
            if (DAT_18009c1c9 == '\0') {
              return 0;
            }
            LOCK();
            UNLOCK();
            local_res10[0] = "UI mode (Prisma focused)";
            if (DAT_180095120 == '\0') {
              local_res10[0] = "camera mode (Prisma released)";
            }
            DAT_180095120 = DAT_180095120 == '\0';
            plVar3 = (longlong *)FUN_180047e20();
            uStack_70 = CONCAT44(uStack_70._4_4_,0x2d1);
            local_80 = 6;
            local_88 = "F4: {}";
            local_78 = "src\\main.cpp";
            local_68 = "void __cdecl `anonymous-namespace\'::HandleFocusHotkey(void)";
            FUN_180022e60(plVar3,(longlong *)&local_78,2,(longlong *)&local_88,local_res10);
            lVar4 = FUN_1800382c0();
            local_58[0] = std::
                          _Func_impl_no_alloc<`void___cdecl__anon_BB2E73B6::QueuePresentation(void)'::`2'::<lambda_1>,void>
                          ::vftable;
            local_20 = local_58;
            FUN_180037e90(lVar4,(longlong *)local_58);
            if (cVar1 == '\0') {
              FUN_1800130e0(DAT_18009c1b0,DAT_18009c1c0,0x18009c1c9,0x180095120);
              return 0;
            }
            FocusRecovery__Cancel();
            return 0;
          }
          if ((*(int *)(lVar2 + 0x20) == 0x3d) && (DAT_18009c1c9 != '\0')) {
            LOCK();
            UNLOCK();
            local_res10[0] = "restored";
            if (DAT_18009c1ca == '\0') {
              local_res10[0] = "fully hidden";
            }
            DAT_18009c1ca = DAT_18009c1ca == '\0';
            plVar3 = (longlong *)FUN_180047e20();
            uStack_70 = CONCAT44(uStack_70._4_4_,0x2f8);
            local_80 = 0x10;
            local_88 = "F3: interface {}";
            local_78 = "src\\main.cpp";
            local_68 = 
            "enum RE::BSEventNotifyControl __cdecl `anonymous-namespace\'::InputSink::ProcessEvent(class RE::InputEvent *const *,class RE::BSTEventSource<class RE::InputEvent *> *)"
            ;
            FUN_180022e60(plVar3,(longlong *)&local_78,2,(longlong *)&local_88,local_res10);
            FUN_18002bea0();
            return 0;
          }
        }
      }
    }
  }
  return 0;
}

