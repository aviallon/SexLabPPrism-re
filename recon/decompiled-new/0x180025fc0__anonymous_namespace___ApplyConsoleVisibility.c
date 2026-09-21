// PrismExport addr=0x180025fc0 name=`anonymous-namespace'::ApplyConsoleVisibility ghidra=_anonymous_namespace_::ApplyConsoleVisibility status=clean lines=97

void _anonymous_namespace___ApplyConsoleVisibility
               (undefined8 param_1,undefined8 param_2,longlong param_3,undefined8 param_4)

{
  char cVar1;
  longlong lVar2;
  longlong **pplVar3;
  longlong *plVar4;
  longlong *plVar5;
  bool bVar6;
  longlong *local_res10;
  longlong *local_res18;
  longlong *local_res20;
  char *local_78;
  undefined8 local_70;
  char *local_68;
  undefined8 local_60;
  char *local_58;
  undefined4 uStack_50;
  undefined4 uStack_4c;
  char *local_48;
  char *local_38;
  undefined8 uStack_30;
  char *local_28;
  
  local_res10 = (longlong *)((ulonglong)local_res10 & 0xffffffff00000000);
  lVar2 = FUN_180037840(param_1,param_2,param_3,param_4);
  bVar6 = lVar2 == 0;
  if (bVar6) {
    pplVar3 = &local_res18;
    plVar5 = (longlong *)0x0;
  }
  else {
    local_78 = "Console";
    local_70 = 7;
    pplVar3 = (longlong **)FUN_1800377b0(lVar2,(longlong *)&local_res10,&local_78,param_4);
    plVar5 = *pplVar3;
  }
  local_res20 = plVar5;
  *pplVar3 = (longlong *)0x0;
  if (bVar6) {
    if (local_res18 != (longlong *)0x0) {
      (**(code **)(*local_res18 + 0x228))();
    }
  }
  if ((!bVar6) && (local_res10 != (longlong *)0x0)) {
    (**(code **)(*local_res10 + 0x228))();
    local_res10 = (longlong *)0x0;
  }
  if (plVar5 != (longlong *)0x0) {
    if ((char)param_1 == '\0') {
      if (DAT_18009c1ce != '\0') {
        (**(code **)(*plVar5 + 0x40))(plVar5,DAT_180095122);
        DAT_18009c1ce = '\0';
        plVar4 = (longlong *)FUN_180047e20();
        local_58 = "src\\main.cpp";
        uStack_50 = 0xd7;
        uStack_4c = uStack_30._4_4_;
        local_48 = "void __cdecl `anonymous-namespace\'::ApplyConsoleVisibility(bool)";
        local_68 = "Developer Console movie visibility restored: {}";
        local_60 = 0x2f;
        uStack_30 = CONCAT44(uStack_30._4_4_,0xd7);
        local_38 = "src\\main.cpp";
        local_28 = "void __cdecl `anonymous-namespace\'::ApplyConsoleVisibility(bool)";
        FUN_1800227a0(plVar4,(longlong *)&local_38,2,(longlong *)&local_68,&DAT_180095122);
      }
    }
    else {
      if (DAT_18009c1ce == '\0') {
        DAT_180095122 = (**(code **)(*plVar5 + 0x48))(plVar5);
        DAT_18009c1ce = '\x01';
      }
      cVar1 = (**(code **)(*plVar5 + 0x48))(plVar5);
      if (cVar1 != '\0') {
        (**(code **)(*plVar5 + 0x40))(plVar5,0);
        plVar4 = (longlong *)FUN_180047e20();
        local_58 = "src\\main.cpp";
        uStack_50 = 0xd2;
        uStack_4c = uStack_30._4_4_;
        local_48 = "void __cdecl `anonymous-namespace\'::ApplyConsoleVisibility(bool)";
        local_68 = "Developer Console movie hidden during scene";
        local_60 = 0x2b;
        uStack_30 = CONCAT44(uStack_30._4_4_,0xd2);
        local_38 = "src\\main.cpp";
        local_28 = "void __cdecl `anonymous-namespace\'::ApplyConsoleVisibility(bool)";
        FUN_18000daf0(plVar4,(longlong *)&local_38,2,(longlong *)&local_68);
      }
    }
  }
  if (plVar5 != (longlong *)0x0) {
    (**(code **)(*plVar5 + 0x228))(plVar5);
  }
  return;
}

