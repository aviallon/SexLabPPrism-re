// PrismExport addr=0x1800261e0 name=void __cdecl `anonymous-namespace'::ApplyPresentation(void) ghidra=_anonymous_namespace_::ApplyPresentation status=clean lines=210

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void _anonymous_namespace___ApplyPresentation
               (undefined8 param_1,undefined8 param_2,longlong param_3,undefined8 param_4)

{
  bool bVar1;
  char cVar2;
  char cVar3;
  char cVar4;
  longlong *plVar5;
  byte bVar6;
  void *_Memory;
  longlong lVar7;
  uint uVar8;
  ulonglong uVar9;
  undefined1 auStackY_e8 [32];
  char *local_b8;
  ulonglong *puStack_b0;
  char *local_a8;
  char *local_98;
  undefined8 uStack_90;
  char *local_88;
  char *local_78;
  ulonglong *puStack_70;
  char *local_68;
  char *local_58;
  void *local_50;
  undefined8 uStack_48;
  undefined8 local_40;
  ulonglong uStack_38;
  ulonglong local_30 [2];
  char *local_20;
  char *local_18;
  ulonglong local_10;
  
  local_10 = DAT_180095900 ^ (ulonglong)auStackY_e8;
  local_58 = (char *)((ulonglong)local_58 & 0xffffffff00000000);
  uVar8 = (uint)DAT_18009c1c9;
  _anonymous_namespace___ApplyVanillaHUDVisibility((ulonglong)uVar8,param_2,param_3,param_4);
  _anonymous_namespace___ApplyConsoleVisibility((ulonglong)uVar8,param_2,param_3,param_4);
  cVar2 = DAT_18009c1cb;
  cVar4 = DAT_180095120;
  if (DAT_18009c1b0 != (longlong *)0x0) {
    if ((DAT_18009c1c9 == 0) || (DAT_18009c1ca != '\0')) {
      bVar1 = false;
    }
    else {
      bVar1 = true;
    }
    if (DAT_18009c1c0 != 0) {
      cVar3 = (**(code **)(*DAT_18009c1b0 + 0x60))();
      if (cVar3 != '\0') {
        if (bVar1) {
          cVar3 = (**(code **)(*DAT_18009c1b0 + 0x48))(DAT_18009c1b0,DAT_18009c1c0);
          if (cVar3 != '\0') {
            (**(code **)(*DAT_18009c1b0 + 0x38))(DAT_18009c1b0,DAT_18009c1c0);
            plVar5 = (longlong *)FUN_180047e20();
            local_b8 = "src\\main.cpp";
            puStack_b0 = (ulonglong *)CONCAT44(puStack_b0._4_4_,0xf4);
            local_a8 = "void __cdecl `anonymous-namespace\'::ApplyPresentation(void)";
            local_78 = "Presentation: controller shown";
            puStack_70 = (ulonglong *)0x1e;
            local_98 = "src\\main.cpp";
            uStack_90 = puStack_b0;
            local_88 = "void __cdecl `anonymous-namespace\'::ApplyPresentation(void)";
            FUN_18000daf0(plVar5,(longlong *)&local_98,2,(longlong *)&local_78);
          }
          local_20 = "false";
          if ((cVar4 != '\0') || (local_18 = "true", cVar2 != '\0')) {
            local_18 = "false";
          }
          if (cVar4 != '\0') {
            local_20 = "true";
          }
          local_98 = "window.slppSetCollapsed({});window.slppSetInteractive({});";
          uStack_90 = 0x3a;
          local_30[0] = 0xb000000000000000;
          local_30[1] = 0xb000000000000008;
          local_78 = (char *)0x2;
          puStack_70 = local_30;
          uStack_48 = 0;
          local_40 = _DAT_18005c8e0;
          uStack_38 = _UNK_18005c8e8;
          local_50 = (void *)0x0;
          local_58 = (char *)CONCAT44(local_58._4_4_,2);
          lVar7 = 0;
          uVar9 = 0;
          do {
            bVar6 = (byte)(local_30[uVar9] >> 0x3c);
            if (bVar6 == 0xc) {
              lVar7 = lVar7 + *(longlong *)
                               ((longlong)local_30 + (local_30[uVar9] & 0xfffffffffffffff) + 0x18);
            }
            else if (bVar6 == 0xb) {
              lVar7 = lVar7 + 0x20;
            }
            else {
              lVar7 = lVar7 + 8;
            }
            uVar9 = uVar9 + 1;
          } while (uVar9 < 2);
          FUN_18001e160(&local_50,lVar7 + 0x3a);
          local_b8 = local_78;
          puStack_b0 = puStack_70;
          FUN_18000f650(&local_78,&local_50,(longlong *)&local_98,&local_b8);
          _anonymous_namespace___InvokeOn(DAT_18009c1c0,&DAT_18009c1c8,&local_50);
          if (0xf < uStack_38) {
            _Memory = local_50;
            if ((0xfff < uStack_38 + 1) &&
               (_Memory = *(void **)((longlong)local_50 + -8),
               0x1f < (ulonglong)((longlong)local_50 + (-8 - (longlong)_Memory)))) {
                    /* WARNING: Subroutine does not return */
              _invoke_watson((wchar_t *)0x0,(wchar_t *)0x0,(wchar_t *)0x0,0,0);
            }
            free(_Memory);
          }
          local_40 = _DAT_18005c8e0;
          uStack_38 = _UNK_18005c8e8;
          local_50 = (void *)((ulonglong)local_50 & 0xffffffffffffff00);
          if ((cVar4 == '\0') && (cVar2 == '\0')) {
            cVar4 = (**(code **)(*DAT_18009c1b0 + 0x20))(DAT_18009c1b0,DAT_18009c1c0);
            if (cVar4 == '\0') {
              cVar4 = (**(code **)(*DAT_18009c1b0 + 0x28))(DAT_18009c1b0,DAT_18009c1c0,0,0);
              if (cVar4 == '\0') {
                plVar5 = (longlong *)FUN_180047e20();
                local_b8 = "src\\main.cpp";
                puStack_b0 = (ulonglong *)CONCAT44(puStack_b0._4_4_,0x104);
                local_a8 = "void __cdecl `anonymous-namespace\'::ApplyPresentation(void)";
                local_98 = "Presentation: Focus() failed";
                uStack_90 = 0x1c;
                local_78 = "src\\main.cpp";
                puStack_70 = puStack_b0;
                local_68 = "void __cdecl `anonymous-namespace\'::ApplyPresentation(void)";
                FUN_18000daf0(plVar5,(longlong *)&local_78,3,(longlong *)&local_98);
              }
              else {
                plVar5 = (longlong *)FUN_180047e20();
                local_b8 = "src\\main.cpp";
                puStack_b0 = (ulonglong *)CONCAT44(puStack_b0._4_4_,0x102);
                local_a8 = "void __cdecl `anonymous-namespace\'::ApplyPresentation(void)";
                local_98 = "Presentation: focus acquired (cursor on UI)";
                uStack_90 = 0x2b;
                local_78 = "src\\main.cpp";
                puStack_70 = puStack_b0;
                local_68 = "void __cdecl `anonymous-namespace\'::ApplyPresentation(void)";
                FUN_18000daf0(plVar5,(longlong *)&local_78,2,(longlong *)&local_98);
              }
            }
          }
          else {
            cVar4 = (**(code **)(*DAT_18009c1b0 + 0x20))(DAT_18009c1b0,DAT_18009c1c0);
            if (cVar4 != '\0') {
              (**(code **)(*DAT_18009c1b0 + 0x30))(DAT_18009c1b0,DAT_18009c1c0);
              local_58 = "camera mode";
              if (cVar2 != '\0') {
                local_58 = "modal search owns input";
              }
              plVar5 = (longlong *)FUN_180047e20();
              local_b8 = "src\\main.cpp";
              puStack_b0 = (ulonglong *)CONCAT44(puStack_b0._4_4_,0xfc);
              local_a8 = "void __cdecl `anonymous-namespace\'::ApplyPresentation(void)";
              local_98 = "Presentation: Prisma focus released ({})";
              uStack_90 = 0x28;
              local_78 = "src\\main.cpp";
              puStack_70 = puStack_b0;
              local_68 = "void __cdecl `anonymous-namespace\'::ApplyPresentation(void)";
              FUN_180022e60(plVar5,(longlong *)&local_78,2,(longlong *)&local_98,&local_58);
            }
          }
        }
        else {
          cVar4 = (**(code **)(*DAT_18009c1b0 + 0x20))();
          if (cVar4 != '\0') {
            (**(code **)(*DAT_18009c1b0 + 0x30))(DAT_18009c1b0,DAT_18009c1c0);
            plVar5 = (longlong *)FUN_180047e20();
            local_b8 = "src\\main.cpp";
            puStack_b0 = (ulonglong *)CONCAT44(puStack_b0._4_4_,0xeb);
            local_a8 = "void __cdecl `anonymous-namespace\'::ApplyPresentation(void)";
            local_78 = "Presentation: released focus (scene inactive)";
            puStack_70 = (ulonglong *)0x2d;
            local_98 = "src\\main.cpp";
            uStack_90 = puStack_b0;
            local_88 = "void __cdecl `anonymous-namespace\'::ApplyPresentation(void)";
            FUN_18000daf0(plVar5,(longlong *)&local_98,2,(longlong *)&local_78);
          }
          cVar4 = (**(code **)(*DAT_18009c1b0 + 0x48))(DAT_18009c1b0,DAT_18009c1c0);
          if (cVar4 == '\0') {
            (**(code **)(*DAT_18009c1b0 + 0x40))(DAT_18009c1b0,DAT_18009c1c0);
            plVar5 = (longlong *)FUN_180047e20();
            local_b8 = "src\\main.cpp";
            puStack_b0 = (ulonglong *)CONCAT44(puStack_b0._4_4_,0xef);
            local_a8 = "void __cdecl `anonymous-namespace\'::ApplyPresentation(void)";
            local_78 = "Presentation: controller hidden";
            puStack_70 = (ulonglong *)0x1f;
            local_98 = "src\\main.cpp";
            uStack_90 = puStack_b0;
            local_88 = "void __cdecl `anonymous-namespace\'::ApplyPresentation(void)";
            FUN_18000daf0(plVar5,(longlong *)&local_98,2,(longlong *)&local_78);
          }
        }
      }
    }
  }
  return;
}

