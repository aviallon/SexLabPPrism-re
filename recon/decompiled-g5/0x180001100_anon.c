// PrismExport addr=0x180001100 name=anon ghidra=FUN_180001100 status=clean lines=22

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_180001100(void)

{
  undefined8 uVar1;
  char *pcVar2;
  
  pcVar2 = (char *)FUN_1800013a0(0x20);
  uVar1 = s___active__false__18005c930._8_8_;
  DAT_18009c1a0 = _DAT_18005fd50;
  DAT_18009c1a8 = _UNK_18005fd58;
  DAT_18009c190 = pcVar2;
  *(undefined8 *)pcVar2 = s___active__false__18005c930._0_8_;
  *(undefined8 *)(pcVar2 + 8) = uVar1;
  pcVar2[0x10] = '\0';
  atexit(FUN_180054ef0);
  return;
}

