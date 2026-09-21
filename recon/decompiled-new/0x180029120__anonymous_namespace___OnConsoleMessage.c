// PrismExport addr=0x180029120 name=`anonymous-namespace'::OnConsoleMessage ghidra=_anonymous_namespace_::OnConsoleMessage status=clean lines=55

void _anonymous_namespace___OnConsoleMessage(undefined8 param_1,char param_2,undefined *param_3)

{
  longlong *plVar1;
  int iVar2;
  undefined *local_res18;
  undefined *local_res20;
  char *local_58;
  undefined8 local_50;
  char *local_48;
  undefined4 uStack_40;
  undefined4 uStack_3c;
  char *local_38;
  char *local_28;
  undefined8 uStack_20;
  char *local_18;
  
  local_res18 = &DAT_180056bd5;
  if (param_3 != (undefined *)0x0) {
    local_res18 = param_3;
  }
  local_res20 = &DAT_18005df68;
  if (param_2 == '\x02') {
    plVar1 = (longlong *)FUN_180047e20();
    iVar2 = 4;
    uStack_40 = 0x26a;
  }
  else if (param_2 == '\x01') {
    plVar1 = (longlong *)FUN_180047e20();
    iVar2 = 3;
    uStack_40 = 0x26c;
  }
  else {
    plVar1 = (longlong *)FUN_180047e20();
    iVar2 = 2;
    uStack_40 = 0x26e;
  }
  local_50 = 7;
  local_48 = "src\\main.cpp";
  uStack_3c = uStack_20._4_4_;
  uStack_20 = CONCAT44(uStack_20._4_4_,uStack_40);
  local_38 = 
  "void __cdecl `anonymous-namespace\'::OnConsoleMessage(unsigned __int64,enum PRISMA_UI_API::ConsoleMessageLevel,const char *)"
  ;
  local_58 = "[{}] {}";
  local_28 = "src\\main.cpp";
  local_18 = 
  "void __cdecl `anonymous-namespace\'::OnConsoleMessage(unsigned __int64,enum PRISMA_UI_API::ConsoleMessageLevel,const char *)"
  ;
  FUN_1800229b0(plVar1,(longlong *)&local_28,iVar2,(longlong *)&local_58,&local_res20,&local_res18);
  return;
}

